#!/usr/bin/env bash
# Measures what a choice-aware placement is worth to physical-aware
# technology mapping.  The claim under test is that a choice graph makes the
# physical data of the pre-mapping network wrong, that the wrong data is what
# the mapper ranks its cuts by, and that selecting the alternatives before the
# placement -- the choice placer -- repairs it.
#
# A standard physical-aware mapping flow has three steps:
#
#   step 1  read the library, read the AIG, structure hash, make the choice
#           graph.  It is the same for every experiment below, so it is not
#           varied: the graph is always "&st; &dch" (or "&choice").
#   step 2  &phys_bind; &phys_place; &phys_sta
#           the placement and the timing of the un-mapped network, which is
#           what the mapper is about to read.
#   step 3  &phys_map; &phys_place; &phys_sta
#           the placement and the timing of the netlist that came out, which
#           is what the flow finally delivers.
#
# The four experiments differ only in step 2:
#
#   E1  select    &phys_place -c (the choice placer)
#       The flow the claim is about.  Only the selected alternative of every
#       class is placed, so the density, the wires and the timing the mapper
#       reads describe the netlist it will pick from.  Step 3 sizes its own
#       core from the mapped netlist it places, which is what E2 inherits.
#
#   E2  pinned    &phys_place (no -c), on the cores E1 chose (-w/-y)
#       Test 1 of the choice-blind flow.  Both floorplans are inherited from
#       E1: step 2 places on the core the choice placer placed on, and step 3
#       places the mapped netlist on the core the choice placer sized for it.
#       The two flows may then not differ by the size of the core, only by
#       the netlist the placement inside it was made from.
#
#   E3  default   &phys_place -A (no -c)
#       Test 2 of the choice-blind flow.  The core is derived from the area
#       of every alternative it places, which is what a flow with no notion
#       of a selection builds by itself: the floorplan of the whole graph.
#
#   E4  repr      &dch -R first, then &phys_place (no -c)
#       The choices are deleted before anything else happens: only the
#       representative cone of every class survives, so step 1 has nothing
#       left to distort.  It is the control for the value of the
#       alternatives themselves.
#
#   E5  norounds  the choice placer with "-r 1", i.e. no selection round
#       (diagnostic, not part of the four-experiment suite).  Step 2 places
#       only the representative alternative of every class on E1's step-2
#       core, so it differs from E1 in the selection alone: E1 evaluates
#       every alternative and switches the classes that lower the
#       wirelength, E5 switches nothing.  If the two deliver the same
#       netlist, the selection rounds -- the part of the choice placer that
#       costs the extra runtime -- do not reach the mapper.
#
# E1 against E2 answers the first conclusion with the floorplan held fixed:
# the choice placer gives the mapper physical data that matches the netlist,
# so the netlist of step 3 is better than the one a choice-blind flow
# produces, and the difference is the placement and not the core.  E1 against
# E3 asks the same question when the blind flow is also allowed to size its
# own core -- the difference then contains both the floorplan and the
# placement, which is what a real choice-blind flow would suffer.  E1 against
# E4 answers the second conclusion: with correct physical data, the
# alternatives are worth more than the single representative cone --
# choice-based physical mapping needs the choices to be there in the first
# place.
#
# Every row is one design and one "abc" invocation.  The rows of one design
# differ in the experiment only: the library, the choice generator, the
# mapper and the placement settings are the same.
#
#   test/phys_choiceplacer.sh [options] [design.aig ...]
#
#   -o dir    workspace directory [default = phys_choiceplacer_ws]
#   -r        remove the workspace before the run
#   -b dir    directory with the benchmarks, either directly or one
#             subdirectory per family [default = ../benchmarks]
#   -t file   technology LEF
#   -c file   cell LEF
#   -l file   Liberty file
#   -g how    choice generator: "dch" (&st; &dch), "choice" (&choice) or
#             "both" [default = dch]
#   -e list   experiments to run, as the digits of E1..E5, e.g. "-e 12345",
#             or "all" [default = all]; E2 and E5 need E1 in the same
#             workspace
#   -n num    selection rounds of the choice placer [default = 3]
#   -p num    choice tolerance of the choice placer [default = 0.01]
#   -i num    conjugate-gradient iterations [default = 300]
#   -s num    nonlinear placement iterations [default = 400]
#   -u num    target core utilization of every placement [default = 0.80]
#   -q        do not print the per-design progress
#
# MAP_ARGS and STA_ARGS are passed through to "&phys_map" and "&phys_sta".
#
# The workspace holds runs/<design>.<experiment>.<generator>/{run.log,
# summary.csv} and the combined table in all.csv.
#
# The columns of all.csv:
#   design, gen, exp, status,
#   s2_cells, s2_area_um2, s2_placed_area_um2, s2_core_w_um, s2_core_h_um,
#   s2_util, s2_wirelength_um, s2_max_cong, s2_overflow_bins, s2_tmax_ps,
#   s3_cells, s3_area_um2, s3_core_w_um, s3_core_h_um, s3_util,
#   s3_wirelength_um, s3_max_cong, s3_overflow_bins, s3_tmax_ps,
#   milliseconds, target_util, choice_rounds, choice_tol, aig_nodes,
#   bound_cells
#
# s2_* is the un-mapped network the mapper read, s3_* is the netlist it
# built.  The area and the utilization always describe the representative
# subgraph, so the four experiments report comparable numbers; s2_placed_area
# is what the placement actually moved, which is where E2 and E4 differ from
# E1.

set -u
ROOT=$(cd "$(dirname "$0")/.." && pwd)
ABC=${ABC:-$ROOT/abc}
PDK=${PDK:-$ROOT/../icsprout55-pdk}
TECH=${TECH:-$PDK/prtech/techLEF/N551P6M.lef}
CELLS=${CELLS:-$PDK/IP/STD_cell/ics55_LLSC_H7C_V1p10C100/ics55_LLSC_H7CR/lef/ics55_LLSC_H7CR.lef}
LIB=${LIB:-$ROOT/../work/lib/ics55_LLSC_H7CR_typ_tt_1p2_25_nldm.lib}
BENCH=${BENCH:-$ROOT/../benchmarks}
WS=phys_choiceplacer_ws
GEN=dch
EXPS=all
ROUNDS=3
TOL=0.01
CG=300
NEST=400
UTIL=0.80
RESET=0
QUIET=0

while getopts "o:rb:t:c:l:g:e:n:p:i:s:u:qh" opt; do
    case $opt in
        o) WS=$OPTARG ;;
        r) RESET=1 ;;
        b) BENCH=$OPTARG ;;
        t) TECH=$OPTARG ;;
        c) CELLS=$OPTARG ;;
        l) LIB=$OPTARG ;;
        g) GEN=$OPTARG ;;
        e) EXPS=$OPTARG ;;
        n) ROUNDS=$OPTARG ;;
        p) TOL=$OPTARG ;;
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
case $EXPS in
    all) EXPS="1 2 3 4" ;;
    *)   EXPS=$(echo "$EXPS" | sed 's/./& /g') ;;
esac
for e in $EXPS; do
    case $e in
        1|2|3|4|5) ;;
        *) echo "Unknown experiment: E$e (use the digits 1..5 or all)" >&2; exit 1 ;;
    esac
done

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

# The two cores of E1, which E3 inherits: the step-2 core the choice placer
# placed on, and the step-3 core it sized for its own mapped netlist.  They
# are read from the run E1 already wrote, so a workspace is self-contained
# and E3 can be re-run on its own.
core_of() { awk -F, 'NF > 17 { print $8, $9, $17, $18 }' "$WS/runs/$1.1.$2/summary.csv" 2>/dev/null; }

run_one() {
    # run_one <design> <name> <generator> <experiment> <command>
    local d=$1 name=$2 gen=$3 exp=$4 cmd=$5
    local log="$WS/runs/$name.$exp.$gen/run.log"
    mkdir -p "$WS/runs/$name.$exp.$gen"
    local start elapsed rc status
    start=$(date +%s%N)
    "$ABC" -c "read_lef -t $TECH; read_lef $CELLS; read_liberty $LIB; &phys_start; &read $d; $cmd; &phys_end" \
        > "$log" 2>&1
    rc=$?
    elapsed=$(( ($(date +%s%N) - start) / 1000000 ))
    status=ok
    if [ "$rc" -ne 0 ] || grep -q '^Error:' "$log"; then
        status=FAILED
        failed="$failed $name.$exp.$gen"
    fi
    # The log holds the two steps in order: the mapper's "Bound N cell
    # instances" line closes step 2 and opens step 3.  Every placement and
    # timing print is attributed to the step it happens in, so the two
    # halves of the flow can be compared on their own.
    awk -v name="$name" -v gen="$gen" -v ex="$exp" -v st="$status" \
        -v ms="$elapsed" -v UTIL="$UTIL" -v R="$ROUNDS" -v T="$TOL" '
        function readplace(   seg, f) {
            split($0, seg, ", ")
            split(seg[1], f, " ");  pcells = f[2]
            split(seg[3], f, " ");  pcorew = f[2]; pcoreh = f[4]
            split(seg[4], f, " ");  putil = f[2]
            split(seg[7], f, " ");  pwire = f[2]
            split(seg[8], f, " ");  parea = f[3]
            split(seg[9], f, " ");  pplaced = f[3]
            gsub(",", "", pcells); gsub(",", "", putil); gsub(",", "", pwire)
            gsub(",", "", parea);  gsub(",", "", pplaced)
        }
        /^Bound [0-9]+ cell instances/ { step = 3; bound = $2; next }
        /^Bound [0-9]+ AIG nodes/ { aig = $2; next }
        /^Placement: congestion/ {
            mc = $6; ov = $7; gsub(",", "", mc); gsub(",", "", ov); next
        }
        /^Placement: [0-9]+ cells placed/ {
            readplace()
            if ( step == 3 ) {
                c3 = pcells; a3 = parea; w3 = pwire; u3 = putil
                cw3 = pcorew; ch3 = pcoreh; cong3 = mc; ovf3 = ov
            } else {
                c2 = pcells; a2 = parea; pa2 = pplaced; w2 = pwire; u2 = putil
                cw2 = pcorew; ch2 = pcoreh; cong2 = mc; ovf2 = ov
            }
            next
        }
        /^STA: longest path/ {
            if ( step == 3 ) t3 = $4; else t2 = $4
            next
        }
        END {
            printf "%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s\n",
                name, gen, ex, st,
                c2, a2, pa2, cw2, ch2, u2, w2, cong2, ovf2, t2,
                c3, a3, cw3, ch3, u3, w3, cong3, ovf3, t3,
                ms, UTIL, R, T, aig, bound
        }
    ' "$log" > "$WS/runs/$name.$exp.$gen/summary.csv"
    [ "$QUIET" = 1 ] || printf '%-12s %-2s %-6s %-4s %6sms\n' "$name" "$exp" "$gen" "$status" "$elapsed"
}

failed=""
for d in "${designs[@]}"; do
    name=$(basename "$d" .aig)
    for gen in $GENS; do
        case $gen in
            dch)    mkg="&st; &dch" ;;
            choice) mkg="&choice" ;;
        esac
        # E1 first: E2 inherits the two cores it chose
        for pass in 1 2; do
            for e in $EXPS; do
                if [ $pass = 1 ] && [ "$e" != 1 ]; then continue; fi
                if [ $pass = 2 ] && [ "$e" = 1 ]; then continue; fi
                place="&phys_place -v -i $CG -s $NEST -u $UTIL"
                p3="$place"
                # The timing of step 2 is what the mapper ranks its cuts by,
                # so it has to describe the same netlist as the placement
                # that produced it.  E1 selected an alternative of every
                # class, so its step 2 is timed in choice mode ("-C"): only
                # the selected alternative drives and loads a net.  The
                # other three flows have no selection -- they place every
                # alternative and time every alternative -- so their step 2
                # is timed choice-blind, which is the distortion being
                # measured.  Step 3 is always timed choice-blind: after
                # "&phys_map" there are no classes left.
                sta2="&phys_sta -v ${STA_ARGS:-}"
                sta3="&phys_sta -v ${STA_ARGS:-}"
                case $e in
                    1)  pre="$mkg";   p2="$place -c -r $ROUNDS -t $TOL"
                        sta2="$sta2 -C" ;;
                    2)  core=$(core_of "$name" "$gen")
                        if [ -z "$core" ]; then
                            [ "$QUIET" = 1 ] || printf '%-12s %-2s %-6s skipped (run E1 first)\n' "$name" "$e" "$gen"
                            continue
                        fi
                        # both steps run on the cores E1 used, so the flow
                        # differs from E1 by the selection only
                        set -- $core
                        pre="$mkg"; p2="$place -w $1 -y $2"; p3="$place -w $3 -y $4" ;;
                    3)  pre="$mkg";   p2="$place -A" ;;
                    4)  pre="$mkg; &dch -R"; p2="$place" ;;
                    5)  core=$(core_of "$name" "$gen")
                        if [ -z "$core" ]; then
                            [ "$QUIET" = 1 ] || printf '%-12s %-2s %-6s skipped (run E1 first)\n' "$name" "$e" "$gen"
                            continue
                        fi
                        # the same cores as E1 and no selection round, so
                        # the only difference is which alternative is placed
                        set -- $core
                        pre="$mkg"; p2="$place -c -r 1 -w $1 -y $2"
                        p3="$place -w $3 -y $4"
                        sta2="$sta2 -C" ;;
                esac
                # step 1, step 2, step 3
                cmd="$pre; &phys_bind; $p2; $sta2;"
                cmd="$cmd &phys_map ${MAP_ARGS:-}; $p3; $sta3;"
                run_one "$d" "$name" "$gen" "$e" "$cmd"
            done
        done
    done
done

printf 'design,gen,exp,status,s2_cells,s2_area_um2,s2_placed_area_um2,s2_core_w_um,s2_core_h_um,s2_util,s2_wirelength_um,s2_max_cong,s2_overflow_bins,s2_tmax_ps,s3_cells,s3_area_um2,s3_core_w_um,s3_core_h_um,s3_util,s3_wirelength_um,s3_max_cong,s3_overflow_bins,s3_tmax_ps,milliseconds,target_util,choice_rounds,choice_tol,aig_nodes,bound_cells\n' \
    > "$WS/all.csv"
for f in "$WS"/runs/*/summary.csv; do
    [ -f "$f" ] && cat "$f" >> "$WS/all.csv"
done

echo
echo "Workspace: $WS"
awk -F, 'NR == 1 { printf "%-12s %-2s %-6s %7s %9s %7s %9s %7s %9s %8s\n",
                       "design", "E", "gen", "s2cell", "s2_area",
                       "s2wire", "s3_area", "s3wire", "s3_tmax", "msec"
                     next }
         { printf "%-12s %-2s %-6s %7s %9s %7s %9s %7s %9s %8s\n",
                  $1, $3, $2, $5, $6, $11, $16, $20, $23, $24 }' "$WS/all.csv"
echo
echo "Per-design logs: $WS/runs/<design>.<experiment>.<generator>/run.log"
if [ -n "$failed" ]; then
    echo "FAILED:$failed" >&2
    exit 1
fi
exit 0
