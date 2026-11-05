#!/usr/bin/env bash
# Compares the two placements of the same choice graph:
#
#   base  &phys_place        every alternative of every choice class is a
#                            real cell of the analytical placement: it
#                            claims area in the density map and it pulls
#                            its nets, so the placer sees the choice graph
#                            as it is stored.  This is the choice-blind
#                            baseline, and it is the default.
#   ours  &phys_place -c     block coordinate descent: only the selected
#                            alternative of every class enters the global
#                            placement, the alternatives are placed
#                            locally at the output anchor of their class
#                            and weighed by the exact wirelength of their
#                            incident nets, and the selection that wins is
#                            re-placed globally.  -r is the number of
#                            selection rounds, -t the relative gain that
#                            makes a switch worth it.
#
# The same choice graph is used for both runs of a design.  It cannot be
# written to a file -- "&write" stores the strashed network and drops the
# sibling lists -- so every run generates it from the unmapped AIG with the
# same command, which is deterministic: "&st; &dch" or "&choice".
#
# The area, the utilization, the wirelength and the timing always describe
# the representative subgraph -- the selected alternative of every class --
# in both modes.  That is what makes the two runs comparable: the baseline
# only places more cells than the netlist it reports, it does not gain area
# or wires from them.
#
#   test/phys_choice_exp.sh [options] [design.aig ...]
#
#   -o dir    workspace directory [default = phys_choice_ws]
#   -r        remove the workspace before the run
#   -b dir    directory with the benchmarks, either directly or one
#             subdirectory per family [default = ../benchmarks/random_control]
#   -t file   technology LEF
#   -c file   cell LEF
#   -l file   Liberty file
#   -g how    choice generator: "dch" (&st; &dch, sibling lists in pSibls)
#             or "choice" (&choice, equivalence classes in pReprs) or
#             "both" [default = both]
#   -n num    number of selection rounds of the ours run [default = 3]
#   -e num    choice tolerance of the ours run [default = 0.01]
#   -i num    conjugate-gradient iterations [default = 300, the placer's
#             own default; a small number leaves the placement spread-only
#             and buries the difference between the two runs]
#   -s num    nonlinear placement iterations [default = 400]
#   -u num    target core utilization of both runs [default = 0.80; the
#             representative subgraph of a choice graph is much smaller
#             than the graph the baseline sees, so the baseline's 0.60
#             under-fills the core]
#   -q        do not print the per-design progress
#
# The workspace holds runs/<design>.<mode>.<generator>/{run.log,summary.csv}
# and the comparison in summary.csv, which is also printed at the end.

set -u
ROOT=$(cd "$(dirname "$0")/.." && pwd)
ABC=${ABC:-$ROOT/abc}
PDK=${PDK:-$ROOT/../icsprout55-pdk}
TECH=${TECH:-$PDK/prtech/techLEF/N551P6M.lef}
CELLS=${CELLS:-$PDK/IP/STD_cell/ics55_LLSC_H7C_V1p10C100/ics55_LLSC_H7CR/lef/ics55_LLSC_H7CR.lef}
LIB=${LIB:-$ROOT/../work/lib/ics55_LLSC_H7CR_typ_tt_1p2_25_nldm.lib}
BENCH=${BENCH:-$ROOT/../benchmarks/random_control}
WS=phys_choice_ws
ROUNDS=3
TOL=0.01
CG=300
NEST=400
UTIL=0.80
GEN=both
RESET=0
QUIET=0

while getopts "o:rb:t:c:l:g:n:e:i:s:u:qh" opt; do
    case $opt in
        o) WS=$OPTARG ;;
        r) RESET=1 ;;
        b) BENCH=$OPTARG ;;
        t) TECH=$OPTARG ;;
        c) CELLS=$OPTARG ;;
        l) LIB=$OPTARG ;;
        g) GEN=$OPTARG ;;
        n) ROUNDS=$OPTARG ;;
        e) TOL=$OPTARG ;;
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
case $GEN in
    dch)    GENS="dch" ;;
    choice) GENS="choice" ;;
    both)   GENS="dch choice" ;;
    *) echo "Unknown choice generator: $GEN (use dch, choice or both)" >&2; exit 1 ;;
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
    for gen in $GENS; do
        case $gen in
            dch)    mkg="&st; &dch" ;;
            choice) mkg="&choice" ;;
        esac
        for mode in base ours; do
            if [ "$mode" = base ]; then
                place="&phys_place -v"
            else
                place="&phys_place -v -c -r $ROUNDS -t $TOL"
            fi
            log="$WS/runs/$name.$mode.$gen/run.log"
            mkdir -p "$WS/runs/$name.$mode.$gen"
            cmd="read_lef -t $TECH; read_lef $CELLS; read_liberty $LIB"
            cmd="$cmd; &phys_start; &read $d; $mkg; &phys_bind"
            cmd="$cmd; $place -i $CG -s $NEST -u $UTIL"
            # both runs report the representative subgraph: the timing of
            # the selected alternative of every class
            cmd="$cmd; &phys_sta -v -C; &phys_end"
            start=$(date +%s%N)
            "$ABC" -c "$cmd" > "$log" 2>&1
            rc=$?
            # milliseconds: most designs finish in one or two seconds, and a
            # whole-second clock cannot tell the two placements apart
            elapsed=$(( ($(date +%s%N) - start) / 1000000 ))
            status=ok
            if [ "$rc" -ne 0 ] || grep -q '^Error:' "$log"; then
                status=FAILED
                failed="$failed $name.$mode.$gen"
            fi
            # one row per run: the representative-subgraph cost of the
            # final selection, the congestion map and the timing
            awk -v name="$name" -v mode="$mode" -v gen="$gen" -v secs="$elapsed" -v st="$status" -v UTIL="$UTIL" '
                /^Choices: [0-9]+ classes/ { cls = $2; alts = $4 }
                /^Placement: selection round [0-9]+ changed/ {
                    changed += $6; rounds++
                }
                /^Placement: congestion/ {
                    mc = $6; ov = $7; gsub(",", "", mc); gsub(",", "", ov); next
                }
                /^Placement: [0-9]+ cells placed/ {
                    # read the values from the comma-separated segments:
                    # the legalization segment is one word or two
                    split($0, seg, ", ")
                    split(seg[1], f, " "); cells = f[2]
                    split(seg[3], f, " "); u = f[2]
                    split(seg[6], f, " "); w = f[2]
                    split(seg[7], f, " "); a = f[3]
                    gsub(",", "", u); gsub(",", "", a)
                }
                /^Placement: [0-9]+ of the [0-9]+ placed cells are/ {
                    cand = $2
                }
                /^STA: longest path/ { tmax = $4 }
                /^STA: [0-9]+ cells/ { sta = $2 }
                END {
                    printf "%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s\n",
                        name, gen, mode, st, cells, cand, sta, cls, u, w, a, mc, ov, tmax, secs, UTIL
                }
            ' "$log" >> "$WS/runs/$name.$mode.$gen/summary.csv"
            [ "$QUIET" = 1 ] || printf '%-12s %-5s %-6s %-4s %6sms\n' "$name" "$mode" "$gen" "$status" "$elapsed"
        done
    done
done

# the two runs of a design, side by side
printf 'design,gen,mode,status,cells_placed,candidates,sta_cells,classes,util,wirelength_um,area_um2,max_cong,overflow_bins,tmax_ps,milliseconds,target_util\n' \
    > "$WS/summary.csv"
for f in "$WS"/runs/*/summary.csv; do
    [ -f "$f" ] && cat "$f" >> "$WS/summary.csv"
done

echo
echo "Workspace: $WS"
awk -F, 'NR == 1 { printf "%-12s %-7s %-5s %6s %6s %6s %8s %11s %10s %6s %9s %8s\n",
                       "design", "gen", "mode", "cells", "cand", "sta",
                       "wire_um", "area_um2", "cong", "ovfl", "tmax_ps", "msec"
                     next }
         { printf "%-12s %-7s %-5s %6s %6s %6s %8s %11s %10s %6s %9s %8s\n",
                  $1, $2, $3, $5, $6, $7, $10, $11, $12, $13, $14, $15 }' "$WS/summary.csv"
echo
echo "Per-design logs: $WS/runs/<design>.<mode>.<generator>/run.log"
if [ -n "$failed" ]; then
    echo "FAILED:$failed" >&2
    exit 1
fi
exit 0
