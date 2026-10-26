#!/usr/bin/env bash
# Runs the physical-synthesis flow on a set of benchmarks and writes a
# debug workspace.  Every design gets its own directory holding the whole
# console log of the run, the placement dumps of every iteration (a
# picture of the placement, the cell and net tables, the bin density and
# the row occupancy), and one line per iteration in summary.csv.
#
# Run from anywhere; the paths default to the layout of the physmap tree.
#
#   test/phys_flow.sh [options] [design.aig ...]
#
#   -o dir    workspace directory [default = phys_ws]
#   -r        remove the workspace before the run
#   -b dir    directory with the benchmarks [default = ../benchmarks/random_control]
#   -t file   technology LEF [default = $PDK/prtech/techLEF/N551P6M.lef]
#   -c file   cell LEF       [default = $PDK/IP/STD_cell/.../ics55_LLSC_H7CR.lef]
#   -l file   Liberty file   [default = ../work/lib/ics55_LLSC_H7CR_typ_tt_1p2_25_nldm.lib]
#   -i num    mapping-and-placement iterations [default = 3]
#   -q        do not print the per-iteration progress lines
#
#   PDK, TECH, CELLS, LIB, BENCH and ABC can also be set in the environment.
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

while getopts "o:rb:t:c:l:i:qh" opt; do
    case $opt in
        o) WS=$OPTARG ;;
        r) RESET=1 ;;
        b) BENCH=$OPTARG ;;
        t) TECH=$OPTARG ;;
        c) CELLS=$OPTARG ;;
        l) LIB=$OPTARG ;;
        i) ITER=$OPTARG ;;
        q) QUIET=1 ;;
        *) sed -n '2,22p' "$0" | sed 's/^# \{0,1\}//'; exit 1 ;;
    esac
done
shift $((OPTIND - 1))

[ -x "$ABC" ]   || { echo "Cannot find the ABC binary: $ABC" >&2; exit 1; }
[ -f "$TECH" ]  || { echo "Cannot find the technology LEF: $TECH" >&2; exit 1; }
[ -f "$CELLS" ] || { echo "Cannot find the cell LEF: $CELLS" >&2; exit 1; }
[ -f "$LIB" ]   || { echo "Cannot find the Liberty file: $LIB" >&2; exit 1; }

if [ $# -gt 0 ]; then
    designs=("$@")
else
    designs=("$BENCH"/*.aig)
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
printf 'design,iter,cells,util,rows_used,rows_total,max_row_occ,wirelength_um,tmax_ps\n' > "$WS/summary.csv"

failed=""
for d in "${designs[@]}"; do
    name=$(basename "$d" .aig)
    log="$WS/$name/run.log"
    mkdir -p "$WS/$name"
    cmd="read_lef -t $TECH; read_lef $CELLS; read_liberty $LIB"
    [ -n "$SIG_RES" ] && cmd="$cmd; set_signal_res $SIG_RES"
    [ -n "$SIG_CAP" ] && cmd="$cmd; set_signal_cap $SIG_CAP"
    cmd="$cmd; &phys_start; &read $d; &nf"
    for i in $(seq 1 "$ITER"); do
        [ "$i" -gt 1 ] && cmd="$cmd; &phys_map"
        cmd="$cmd; &phys_bind; &phys_place -v; &phys_sta; &phys_dump -o $WS -t it$i"
    done
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
    # one row per iteration: appended after the placement and the STA of it
    awk -v name="$name" '
        /^Placement: [0-9]+ cells placed/ {
            c = $2; u = $9; r = $10; o = $16; w = $18
            gsub(",", "", u); gsub(",", "", o)
            split(r, a, "/")
            iter++
            pc = c; pu = u; pr = a[1]; pt = a[2]; po = o; pw = w; pending = 1
        }
        /^STA: longest path/ && pending {
            printf "%s,%d,%s,%s,%s,%s,%s,%s,%s\n", name, iter, pc, pu, pr, pt, po, pw, $4
            pending = 0
        }
    ' "$log" >> "$WS/summary.csv"
    if [ "$QUIET" = 0 ]; then
        printf '%-12s %-7s %3ds  %s\n' "$name" "$status" "$elapsed" \
            "$(awk -F, -v n="$name" '$1 == n { line = $0 } END { print line }' "$WS/summary.csv" | cut -d, -f2-)"
    fi
done

echo
echo "Workspace: $WS"
awk -F, 'NR == 1 { printf "%-12s %4s %6s %6s %7s %8s %13s %9s\n", "design", "iter", "cells", "util", "rows", "maxocc", "wirelength", "tmax_ps"
                     next }
         { printf "%-12s %4s %6s %6s %7s %8s %13s %9s\n", $1, "it" $2, $3, $4, $5 "/" $6, $7, $8, $9 }' \
    "$WS/summary.csv"
echo
echo "Open $WS/index.html for the per-design logs and pictures."
if [ -n "$failed" ]; then
    echo "FAILED:$failed" >&2
    exit 1
fi
exit 0
