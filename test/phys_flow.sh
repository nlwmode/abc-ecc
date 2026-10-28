#!/usr/bin/env bash
# Runs the physical-synthesis flow on a set of benchmarks and writes a
# debug workspace.  Every design gets its own directory holding the whole
# console log of the run, the placement dumps of every iteration (a
# picture of the placement with translucent standard cells, a RUDY
# routability map, the cell and net tables, the bin density, the per-bin
# congestion, and the row occupancy), and one line per iteration in
# summary.csv.
#
# The flow starts from the unmapped AIG of the benchmark:
#   &phys_start; &phys_bind; &phys_place; &phys_sta; &phys_map; &phys_end
# &phys_bind binds every AIG node to a virtual cell, &phys_place and
# &phys_sta give it coordinates and a timing estimate, and &phys_map
# maps the network with the physical cut ranking.  The mapping is then
# placed and timed again, and the next &phys_map runs on those
# coordinates, which is the physical feedback loop of the flow.  The
# pre-mapping placement is dumped as it0 and the mapping rounds as
# it1..itN.
#
# Run from anywhere; the paths default to the layout of the physmap tree.
#
#   test/phys_flow.sh [options] [design.aig ...]
#
#   -o dir    workspace directory [default = phys_ws]
#   -r        remove the workspace before the run
#   -b dir    directory with the benchmarks, either directly or one
#             subdirectory per family [default = ../benchmarks/random_control]
#   -t file   technology LEF [default = $PDK/prtech/techLEF/N551P6M.lef]
#   -c file   cell LEF       [default = $PDK/IP/STD_cell/.../ics55_LLSC_H7CR.lef]
#   -l file   Liberty file   [default = ../work/lib/ics55_LLSC_H7CR_typ_tt_1p2_25_nldm.lib]
#   -i num    mapping-and-placement iterations [default = 3]
#   -m mode   mapping mode: phys (physical cut ranking with every drive),
#             root (physical cut ranking with the root gates only), or nf
#             (plain &nf, the baseline of the comparison) [default = phys]
#   -w        write the mapped netlist of the last iteration to
#             $WS/<design>/netlist.v and check that it is functionally
#             equivalent to the original AIG with ABC's "cec"; the design
#             fails when the netlist is not equivalent.  The check reads
#             the written Verilog back through the Liberty library, so a
#             netlist that cannot be read as mapped gates fails as well.
#             The result of every design is also recorded in $WS/cec.csv.
#   -q        do not print the per-iteration progress lines
#
#   PDK, TECH, CELLS, LIB, BENCH and ABC can also be set in the environment,
#   and MAP_ARGS adds options to every mapping command, for example
#   MAP_ARGS="-R 20" or MAP_ARGS="-d".  The cut ranking of &phys_map is
#   lexicographic (critical-path timing, area, congestion, routability)
#   and has no cost weights, so the only quality knob of the mapping is
#   "-R", which relaxes the delay bound and gives the area recovery room
#   to undo the fastest drives.
#   SIG_RES (ohm/um) and SIG_CAP (pF/um) set the unit wire RC used by the
#   timing analysis; when they are empty, &phys_sta takes the resistance
#   from the LEF routing layer and a default capacitance.

set -u
ROOT=$(cd "$(dirname "$0")/.." && pwd)
ABC=${ABC:-$ROOT/abc}
PDK=${PDK:-$ROOT/../icsprout55-pdk}
TECH=${TECH:-$PDK/prtech/techLEF/N551P6M.lef}
CELLS=${CELLS:-$PDK/IP/STD_cell/ics55_LLSC_H7C_V1p10C100/ics55_LLSC_H7CR/lef/ics55_LLSC_H7CR.lef}
LIB=${LIB:-$ROOT/../work/lib/ics55_LLSC_H7CR_typ_tt_1p2_25_nldm.lib}
BENCH=${BENCH:-$ROOT/../benchmarks/random_control}
SIG_RES=${SIG_RES:-}
SIG_CAP=${SIG_CAP:-}
WS=phys_ws
ITER=3
RESET=0
QUIET=0
MODE=phys
NETLIST=0

while getopts "o:rb:t:c:l:i:m:qwh" opt; do
    case $opt in
        o) WS=$OPTARG ;;
        r) RESET=1 ;;
        b) BENCH=$OPTARG ;;
        t) TECH=$OPTARG ;;
        c) CELLS=$OPTARG ;;
        l) LIB=$OPTARG ;;
        i) ITER=$OPTARG ;;
        m) MODE=$OPTARG ;;
        w) NETLIST=1 ;;
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

if [ $# -gt 0 ]; then
    designs=("$@")
else
    # $BENCH may hold the AIGs directly or one benchmark directory per
    # family (benchmarks/arithmetic, benchmarks/random_control), so fall
    # back to the second level when the first glob does not match.
    designs=("$BENCH"/*.aig)
    [ -e "${designs[0]}" ] || designs=("$BENCH"/*/*.aig)
fi
if [ ${#designs[@]} -eq 0 ]; then
    echo "No benchmarks found in $BENCH" >&2
    exit 1
fi
[ -f "${designs[0]}" ] || { echo "No benchmarks found in $BENCH" >&2; exit 1; }

if [ "$RESET" = 1 ]; then
    rm -rf "$WS"
fi
mkdir -p "$WS"
printf 'design,iter,cells,util,rows_used,rows_total,max_row_occ,wirelength_um,area_um2,max_cong,overflow_bins,tmax_ps\n' > "$WS/summary.csv"
if [ "$NETLIST" = 1 ]; then
    printf 'design,status\n' > "$WS/cec.csv"
fi

failed=""
for d in "${designs[@]}"; do
    name=$(basename "$d" .aig)
    log="$WS/$name/run.log"
    mkdir -p "$WS/$name"
    cmd="read_lef -t $TECH; read_lef $CELLS; read_liberty $LIB"
    [ -n "$SIG_RES" ] && cmd="$cmd; set_signal_res $SIG_RES"
    [ -n "$SIG_CAP" ] && cmd="$cmd; set_signal_cap $SIG_CAP"
    # the design is an unmapped AIG: bind its nodes to virtual cells, place
    # and time them, and only then map with the physical cut ranking.  Every
    # iteration maps again on the placement of the mapped cells, which is
    # the physical feedback of the flow.
    cmd="$cmd; &phys_start; &read $d; &phys_bind; &phys_place -v; &phys_sta"
    cmd="$cmd; &phys_dump -o $WS -t it0"
    for i in $(seq 1 "$ITER"); do
        cmd="$cmd; $MAPCMD; &phys_place -v; &phys_sta; &phys_dump -o $WS -t it$i"
    done
    if [ "$NETLIST" = 1 ]; then
        # &put turns the mapped Gia into an ABC network and write_verilog
        # exports it with the library cells and their pin names
        cmd="$cmd; &put; write_verilog $WS/$name/netlist.v"
    fi
    cmd="$cmd; &phys_end"
    start=$(date +%s)
    "$ABC" -c "$cmd" > "$log" 2>&1
    rc=$?
    elapsed=$(( $(date +%s) - start ))
    if [ "$rc" -ne 0 ] || grep -q '^Error:' "$log"; then
        failed="$failed $name"
        status="FAILED"
    else
        status="ok"
    fi
    # read the written netlist back through the Liberty library and prove it
    # functionally equivalent to the original AIG with ABC's combinational
    # equivalence checker; a netlist that cannot be read as mapped gates is
    # caught by the blackbox and primary input checks below
    cec_result="skipped"
    cec_time=0
    if [ "$NETLIST" = 1 ]; then
        if [ "$status" = ok ] && [ -s "$WS/$name/netlist.v" ]; then
            cec_start=$(date +%s)
            "$ABC" -c "read_liberty $LIB; &phys_start; read_verilog -m $WS/$name/netlist.v; cec $d" \
                > "$WS/$name/cec.log" 2>&1
            cec_time=$(( $(date +%s) - cec_start ))
            if grep -q 'interpreted as blackboxes' "$WS/$name/cec.log"; then
                cec_result="blackbox"
            elif grep -q 'NOT EQUIVALENT' "$WS/$name/cec.log"; then
                cec_result="not-equivalent"
            elif grep -q 'Miter computation has failed' "$WS/$name/cec.log"; then
                cec_result="mismatch"
            elif grep -q '^Networks are equivalent' "$WS/$name/cec.log"; then
                cec_result="equivalent"
            else
                cec_result="error"
            fi
        elif [ "$status" = ok ]; then
            cec_result="missing"
        fi
        if [ "$cec_result" != equivalent ]; then
            failed="$failed $name"
            status="FAILED"
        fi
        printf '%s,%s\n' "$name" "$cec_result" >> "$WS/cec.csv"
    fi
    # one row per iteration: appended after the placement and the STA of it
    awk -v name="$name" '
        /^Placement: [0-9]+ nodes .* core [0-9.]+ x [0-9.]+ um/ {
            mc = "-"; ov = "-"
        }
        /^Placement: congestion/ {
            # "max 1.00, 1 overflow bins" attaches the comma to the value
            mc = $6; ov = $7; gsub(",", "", mc); gsub(",", "", ov); next
        }
        /^Placement: [0-9]+ cells placed/ {
            # "..., wirelength 17560.1 um, cell area 2698.9 um^2."
            c = $2; u = $9; r = $10; o = $16; w = $18; a = $22
            gsub(",", "", u); gsub(",", "", o)
            split(r, a2, "/")
            iter++
            pc = c; pu = u; pr = a2[1]; pt = a2[2]; po = o; pw = w
            pa = a; pending = 1
        }
        /^STA: longest path/ && pending {
            printf "%s,%d,%s,%s,%s,%s,%s,%s,%.1f,%s,%s,%s\n", name, iter - 1, pc, pu, pr, pt, po, pw, pa, mc, ov, $4
            pending = 0
        }
    ' "$log" >> "$WS/summary.csv"
    if [ "$QUIET" = 0 ]; then
        printf '%-12s %-7s %3ds  %s\n' "$name" "$status" "$elapsed" \
            "$(awk -F, -v n="$name" '$1 == n { line = $0 } END { print line }' "$WS/summary.csv" | cut -d, -f2-)"
        if [ "$NETLIST" = 1 ]; then
            printf '%-12s %-7s %3ds  netlist %s\n' "$name" "cec" "$cec_time" "$cec_result"
        fi
    fi
done

echo
echo "Workspace: $WS"
awk -F, 'NR == 1 { printf "%-12s %4s %6s %6s %7s %8s %11s %10s %6s %5s %9s\n", "design", "iter", "cells", "util", "rows", "maxocc", "wirelength", "area", "cong", "ovfl", "tmax_ps"
                     next }
         { printf "%-12s %4s %6s %6s %7s %8s %11s %10s %6s %5s %9s\n", $1, "it" $2, $3, $4, $5 "/" $6, $7, $8, $9, $10, $11, $12 }' \
    "$WS/summary.csv"
echo
echo "Open $WS/index.html for the per-design logs and pictures."
if [ "$NETLIST" = 1 ]; then
    awk -F, 'NR == 1 { next }
             { n[$2]++ }
             END {
                 printf "Netlists:"
                 for (s in n) printf " %s %d", s, n[s]
                 printf "\n"
             }' "$WS/cec.csv"
    echo "The netlists are in $WS/<design>/netlist.v, the equivalence logs in cec.log."
fi
if [ -n "$failed" ]; then
    echo "FAILED:$failed" >&2
    exit 1
fi
exit 0
