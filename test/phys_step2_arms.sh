#!/usr/bin/env bash
# Step-2 experiment that isolates what the choice-aware placement does to
# the placement itself, without &phys_map:
#
#   sparse     &phys_place -c -r 1        only the selected alternative of
#                                         every class is placed globally
#   sel        &phys_place -c -r N        the same, then N-1 selection rounds
#   dense      &phys_place                every alternative is a cell (the
#                                         choice-blind baseline)
#   dense_sel  &phys_place -c -D -r N     the selection decides and scores the
#                                         netlist, but every alternative stays
#                                         a cell of the global placement
#   reduce     &phys_place -R             the alternatives the placer does not
#                                         select are deleted from the network
#                                         and the reduced netlist is placed
#                                         choice-blind (the control that the
#                                         sparse placement describes exactly
#                                         the netlist "sparse" places)
#
# The three controls of the design are part of the run set:
#
#   budget      dense_b and dense_sel_b repeat the two dense arms with the
#               nonlinear iteration budget scaled by -B, because a dense arm
#               places 1.6x more cells with the same -s
#   legalize    every arm is also run with -l (suffix _l), so an overlap-free
#               placement can be compared as well
#   timing      every run times the design twice: "&phys_sta -C" (only the
#               selected alternative, the netlist that is reported) and the
#               choice-blind "&phys_sta" (every alternative timed as an
#               ordinary cell); the pair is the口径 control of the TMax
#
# Every arm is pinned to the core of the sparse arm of the same design, so a
# difference between two arms is a difference of the placement model or of
# the netlist, not of the floorplan.
#
#   test/phys_step2_arms.sh [options] [design.aig ...]
#
#   -o dir    workspace directory [default = phys_step2_ws]
#   -r        remove the workspace before the run
#   -b dir    directory with the benchmarks, either directly or one
#             subdirectory per family [default = ../benchmarks]
#   -n num    selection rounds of sel/dense_sel [default = 3]
#   -e num    choice tolerance [default = 0.01]
#   -i num    conjugate-gradient iterations [default = 300]
#   -s num    nonlinear placement iterations [default = 400]
#   -B num    budget multiplier of the dense_b arms [default = 1.6]
#   -u num    target core utilization of the sparse arm [default = 0.80]
#   -x        skip the legalized arms
#   -q        do not print the per-run progress
#
# The workspace holds runs/<design>.<arm>/run.log and arms.csv, which is
# also printed at the end.

set -u
ROOT=$(cd "$(dirname "$0")/.." && pwd)
ABC=${ABC:-$ROOT/abc}
PDK=${PDK:-$ROOT/../icsprout55-pdk}
TECH=${TECH:-$PDK/prtech/techLEF/N551P6M.lef}
CELLS=${CELLS:-$PDK/IP/STD_cell/ics55_LLSC_H7C_V1p10C100/ics55_LLSC_H7CR/lef/ics55_LLSC_H7CR.lef}
LIB=${LIB:-$ROOT/../work/lib/ics55_LLSC_H7CR_typ_tt_1p2_25_nldm.lib}
BENCH=${BENCH:-$ROOT/../benchmarks}
WS=phys_step2_ws
ROUNDS=3
TOL=0.01
CG=300
NEST=400
BUDGET=1.6
UTIL=0.80
RESET=0
QUIET=0
SKIP_LEGAL=0

while getopts "o:rb:n:e:i:s:B:u:xqh" opt; do
    case $opt in
        o) WS=$OPTARG ;;
        r) RESET=1 ;;
        b) BENCH=$OPTARG ;;
        n) ROUNDS=$OPTARG ;;
        e) TOL=$OPTARG ;;
        i) CG=$OPTARG ;;
        s) NEST=$OPTARG ;;
        B) BUDGET=$OPTARG ;;
        u) UTIL=$OPTARG ;;
        x) SKIP_LEGAL=1 ;;
        q) QUIET=1 ;;
        *) sed -n '2,/^$/p' "$0" | sed 's/^# \{0,1\}//'; exit 1 ;;
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
    designs=("$BENCH"/*/*.aig)
    [ -e "${designs[0]}" ] || designs=("$BENCH"/*.aig)
fi
[ -e "${designs[0]}" ] || { echo "No benchmarks found in $BENCH" >&2; exit 1; }

if [ "$RESET" = 1 ]; then
    rm -rf "$WS"
fi
mkdir -p "$WS/runs"
echo "design,arm,status,cells,area_um2,core_w,core_h,util,occ,max_cong,overflow," \
     "wire_um,tmax_C_ps,tmax_blind_ps,placed_area_um2,reduced_objs,msec,nest_iter" \
     > "$WS/arms.csv"

# one run: $1 design, $2 name, $3 arm, $4 place options, $5 core (pinned by
# the caller), $6 nest iterations
run_arm() {
    local d=$1 name=$2 arm=$3 popts=$4 core=$5 nest=$6
    local dir log start elapsed rc status cmd
    dir="$WS/runs/$name.$arm"
    log="$dir/run.log"
    mkdir -p "$dir"
    cmd="read_lef -t $TECH; read_lef $CELLS; read_liberty $LIB"
    cmd="$cmd; &phys_start; &read $d; &st; &dch; &phys_bind"
    cmd="$cmd; &phys_place -v $popts $core -i $CG -s $nest -u $UTIL"
    # the reported timing first (the selected alternative), then the
    # choice-blind view of the same placement
    cmd="$cmd; &phys_sta -v -C; &phys_sta -v; &phys_end"
    start=$(date +%s%N)
    "$ABC" -c "$cmd" > "$log" 2>&1
    rc=$?
    elapsed=$(( ($(date +%s%N) - start) / 1000000 ))
    status=ok
    if [ "$rc" -ne 0 ] || grep -q '^Error:' "$log"; then
        status=FAILED
        failed="$failed $name.$arm"
    fi
    awk -v name="$name" -v arm="$arm" -v secs="$elapsed" -v st="$status" '
        /^Placement: reduced the choice graph/ { red = $(NF-1) }
        /^Placement: congestion/ {
            mc = $6; ov = $7; gsub(",", "", mc); gsub(",", "", ov); next
        }
        /^Placement: [0-9]+ cells placed/ {
            split($0, seg, ", ")
            split(seg[1], f, " "); cells = f[2]
            split(seg[3], f, " "); cw = f[2]; ch = f[4]; gsub(/um/, "", ch)
            split(seg[4], f, " "); u = f[2]
            split(seg[6], f, " "); occ = f[4]
            split(seg[7], f, " "); w = f[2]
            split(seg[8], f, " "); a = f[3]
            split(seg[9], f, " "); pa = f[3]
            gsub(",", "", u); gsub(",", "", a); gsub(",", "", pa)
            next
        }
        /^STA: longest path/ { t[++nt] = $4 }
        END {
            printf "%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s\n",
                name, arm, st, cells, a, cw, ch, u, occ, mc, ov, w,
                t[1], t[2], pa, red, secs, "'"$nest"'"
        }
    ' "$log" >> "$WS/arms.csv"
    [ "$QUIET" = 1 ] || printf '%-12s %-12s %-4s %6sms\n' "$name" "$arm" "$status" "$elapsed"
}

failed=""
for d in "${designs[@]}"; do
    name=$(basename "$d" .aig)
    # the core of the sparse arm is the floorplan of every arm of this design
    run_arm "$d" "$name" sparse "-c -r 1" "" "$NEST"
    core_w=$(awk -F, -v n="$name" '$1 == n && $2 == "sparse" { print $6 }' "$WS/arms.csv")
    core_h=$(awk -F, -v n="$name" '$1 == n && $2 == "sparse" { print $7 }' "$WS/arms.csv")
    if [ -z "$core_w" ] || [ -z "$core_h" ]; then
        echo "Cannot read the core of $name from the sparse run." >&2
        failed="$failed $name.sparse"
        continue
    fi
    core="-w $core_w -y $core_h"
    nest_b=$(awk -v n="$NEST" -v b="$BUDGET" 'BEGIN { printf "%d", n * b + 0.5 }')
    run_arm "$d" "$name" sel        "-c -r $ROUNDS -t $TOL" "$core" "$NEST"
    run_arm "$d" "$name" dense      ""                       "$core" "$NEST"
    run_arm "$d" "$name" dense_sel  "-c -D -r $ROUNDS -t $TOL" "$core" "$NEST"
    run_arm "$d" "$name" reduce     "-R"                     "$core" "$NEST"
    run_arm "$d" "$name" dense_b    ""                       "$core" "$nest_b"
    run_arm "$d" "$name" dense_sel_b "-c -D -r $ROUNDS -t $TOL" "$core" "$nest_b"
    if [ "$SKIP_LEGAL" = 0 ]; then
        run_arm "$d" "$name" sparse_l    "-c -r 1 -l"          "$core" "$NEST"
        run_arm "$d" "$name" sel_l       "-c -r $ROUNDS -t $TOL -l" "$core" "$NEST"
        run_arm "$d" "$name" dense_l     "-l"                  "$core" "$NEST"
        run_arm "$d" "$name" dense_sel_l "-c -D -r $ROUNDS -t $TOL -l" "$core" "$NEST"
        run_arm "$d" "$name" reduce_l    "-R -l"               "$core" "$NEST"
    fi
done

echo
echo "Workspace: $WS"
awk -F, 'BEGIN { printf "%-12s %-12s %6s %9s %9s %10s %6s %8s %6s %9s %10s %7s\n",
                  "design", "arm", "cells", "area_um2", "pl_area", "wire_um", "occ",
                  "cong", "ovfl", "tmax_C", "tmax_blind", "msec" }
         $1 == "design" { next }
         { printf "%-12s %-12s %6s %9s %9s %10s %6s %8s %6s %9s %10s %7s\n",
                  $1, $2, $4, $5, $15, $12, $9, $10, $11, $13, $14, $17 }' "$WS/arms.csv"
echo
echo "CSV: $WS/arms.csv"
echo "Per-run logs: $WS/runs/<design>.<arm>/run.log"
if [ -n "$failed" ]; then
    echo "FAILED:$failed" >&2
    exit 1
fi
exit 0
