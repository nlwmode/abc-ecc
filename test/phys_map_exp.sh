#!/usr/bin/env bash
# Places and times the netlist that "&phys_map" produced, and compares the
# physical data of that netlist against the pre-mapping network the choice
# experiments start from.
#
#   post  &phys_map; &phys_place; &phys_sta
#         The mapped netlist is real cells: after the mapping there are no
#         choice classes left, so this is the plain placement and timing of
#         what was actually built.  This is the reference for every other
#         experiment.
#   pre   &phys_bind; &phys_place; &phys_sta
#         The unmapped AIG bound to virtual supergate cells -- the network
#         the choice graph is a choice over.  Placed and timed the same
#         way, so the two rows of a design differ only in the network.
#
# Both runs are one "abc" invocation, both use the same placement and STA
# settings, and both report the wirelength, area, congestion and the longest
# path of the network that was placed -- there is no representative
# subgraph to select here.
#
#   test/phys_map_exp.sh [options] [design.aig ...]
#
#   -o dir    workspace directory [default = phys_map_ws]
#   -r        remove the workspace before the run
#   -b dir    directory with the benchmarks, either directly or one
#             subdirectory per family [default = ../benchmarks/random_control]
#   -t file   technology LEF
#   -c file   cell LEF
#   -l file   Liberty file
#   -m how    mapping mode: phys (&phys_map), root (&phys_map -s) or
#             nf (&phys_map -l, plain &nf) [default = phys]
#   -g how    which runs to do: "post", "pre" or "both" [default = both]
#   -i num    conjugate-gradient iterations [default = 300]
#   -s num    nonlinear placement iterations [default = 400]
#   -u num    target core utilization of every placement [default = 0.80]
#   -q        do not print the per-design progress
#
# MAP_ARGS and STA_ARGS are passed through to &phys_map and &phys_sta.
#
# The workspace holds runs/<design>.<mode>/{run.log,summary.csv} and the
# comparison in summary.csv, which is also printed at the end.

set -u
ROOT=$(cd "$(dirname "$0")/.." && pwd)
ABC=${ABC:-$ROOT/abc}
PDK=${PDK:-$ROOT/../icsprout55-pdk}
TECH=${TECH:-$PDK/prtech/techLEF/N551P6M.lef}
CELLS=${CELLS:-$PDK/IP/STD_cell/ics55_LLSC_H7C_V1p10C100/ics55_LLSC_H7CR/lef/ics55_LLSC_H7CR.lef}
LIB=${LIB:-$ROOT/../work/lib/ics55_LLSC_H7CR_typ_tt_1p2_25_nldm.lib}
BENCH=${BENCH:-$ROOT/../benchmarks/random_control}
WS=phys_map_ws
CG=300
NEST=400
UTIL=0.80
MODE=phys
RUNS=both
RESET=0
QUIET=0

while getopts "o:rb:t:c:l:m:g:i:s:u:qh" opt; do
    case $opt in
        o) WS=$OPTARG ;;
        r) RESET=1 ;;
        b) BENCH=$OPTARG ;;
        t) TECH=$OPTARG ;;
        c) CELLS=$OPTARG ;;
        l) LIB=$OPTARG ;;
        m) MODE=$OPTARG ;;
        g) RUNS=$OPTARG ;;
        i) CG=$OPTARG ;;
        s) NEST=$OPTARG ;;
        u) UTIL=$OPTARG ;;
        q) QUIET=1 ;;
        *) sed -n '2,/^$/p' "$0" | sed 's/^# \{0,1\}//'; exit 1 ;;
    esac
done
shift $((OPTIND - 1))

[ -x "$ABC" ]   || { echo "Cannot find the ABC binary: $ABC" >&2; exit 1; }
[ -f "$TECH" ]  || { echo "Cannot find the technology LEF: $TECH" >&2; exit 1; }
[ -f "$CELLS" ] || { echo "Cannot find the cell LEF: $CELLS" >&2; exit 1; }
[ -f "$LIB" ]   || { echo "Cannot find the Liberty file: $LIB" >&2; exit 1; }
case $MODE in
    phys) MAPCMD="&phys_map" ;;
    root) MAPCMD="&phys_map -s" ;;
    nf)   MAPCMD="&phys_map -l" ;;
    *) echo "Unknown mapping mode: $MODE (use phys, root or nf)" >&2; exit 1 ;;
esac
MAPCMD="$MAPCMD ${MAP_ARGS:-}"
STACMD="&phys_sta -v ${STA_ARGS:-}"
PLACECMD="&phys_place -v -i $CG -s $NEST -u $UTIL"
case $RUNS in
    post) MODES="post" ;;
    pre)  MODES="pre" ;;
    both) MODES="post pre" ;;
    *) echo "Unknown run set: $RUNS (use post, pre or both)" >&2; exit 1 ;;
esac

if [ $# -gt 0 ]; then
    designs=("$@")
else
    designs=("$BENCH"/*.aig)
    [ -e "${designs[0]}" ] || designs=("$BENCH"/*/*.aig)
fi
[ -e "${designs[0]}" ] || { echo "No benchmarks found in $BENCH" >&2; exit 1; }

if [ "$RESET" = 1 ]; then
    rm -rf "$WS"
fi
mkdir -p "$WS/runs"

failed=""
for d in "${designs[@]}"; do
    name=$(basename "$d" .aig)
    for mode in $MODES; do
        log="$WS/runs/$name.$mode/run.log"
        mkdir -p "$WS/runs/$name.$mode"
        cmd="read_lef -t $TECH; read_lef $CELLS; read_liberty $LIB; &phys_start; &read $d"
        if [ "$mode" = post ]; then
            # the pre-mapping placement and timing are the input of the
            # physical mapper, not part of what this row measures
            cmd="$cmd; &phys_bind; $PLACECMD; $STACMD; $MAPCMD; $PLACECMD; $STACMD"
        else
            # the same network the choice graph is a choice over
            cmd="$cmd; &phys_bind; $PLACECMD; $STACMD"
        fi
        cmd="$cmd; &phys_end"
        start=$(date +%s%N)
        "$ABC" -c "$cmd" > "$log" 2>&1
        rc=$?
        elapsed=$(( ($(date +%s%N) - start) / 1000000 ))
        status=ok
        if [ "$rc" -ne 0 ] || grep -q '^Error:' "$log"; then
            status=FAILED
            failed="$failed $name.$mode"
        fi
        # the placement and the timing of the network this row is about:
        # the last placement summary and the last "longest path" of a log
        # that holds two placements in the post mode.  Both runs print the
        # same lines, so the last occurrence is the one to read.
        awk -v name="$name" -v mode="$mode" -v m="$MODE" -v secs="$elapsed" \
            -v st="$status" -v UTIL="$UTIL" '
            /^Bound [0-9]+ cell instances/ { bound = $2 }
            /^Bound [0-9]+ AIG nodes/ { aig = $2 }
            /^Placement: congestion/ {
                mc = $6; ov = $7; gsub(",", "", mc); gsub(",", "", ov); next
            }
            /^Placement: [0-9]+ cells placed/ {
                # the legalization segment is one word ("legalization
                # skipped") or two ("N overlaps remain"), so the values are
                # read from the comma-separated segments, where their
                # positions are fixed
                split($0, seg, ", ")
                split(seg[1], f, " "); cells = f[2]
                split(seg[3], f, " "); u = f[2]
                split(seg[6], f, " "); w = f[2]
                split(seg[7], f, " "); a = f[3]
                gsub(",", "", u); gsub(",", "", a)
            }
            /^STA: longest path/ { tmax = $4 }
            # the cell count of the timed network, printed by "&phys_sta -v"
            /^STA: [0-9]+ cells/ { sta = $2 }
            END {
                printf "%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s\n",
                    name, m, mode, st, cells, sta, bound, u, w, a, mc, ov, tmax, secs
            }
        ' "$log" >> "$WS/runs/$name.$mode/summary.csv"
        [ "$QUIET" = 1 ] || printf '%-12s %-5s %-4s %6sms\n' "$name" "$mode" "$status" "$elapsed"
    done
done

printf 'design,map_mode,netlist,status,cells_placed,sta_cells,mapped_cells,util,wirelength_um,area_um2,max_cong,overflow_bins,tmax_ps,milliseconds\n' \
    > "$WS/summary.csv"
for f in "$WS"/runs/*/summary.csv; do
    [ -f "$f" ] && cat "$f" >> "$WS/summary.csv"
done

echo
echo "Workspace: $WS"
awk -F, 'NR == 1 { printf "%-12s %-5s %-6s %6s %6s %8s %11s %6s %9s %8s\n",
                       "design", "map", "netlist", "cells", "sta",
                       "wire_um", "area_um2", "cong", "tmax_ps", "msec"
                     next }
         { printf "%-12s %-5s %-6s %6s %6s %8s %11s %6s %9s %8s\n",
                  $1, $2, $3, $5, $6, $9, $10, $11, $13, $14 }' "$WS/summary.csv"
echo
echo "Per-design logs: $WS/runs/<design>.<netlist>/run.log"
if [ -n "$failed" ]; then
    echo "FAILED:$failed" >&2
    exit 1
fi
exit 0
