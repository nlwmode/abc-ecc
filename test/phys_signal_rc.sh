#!/usr/bin/env bash
# Checks that the unit wire RC can be taken from a routing layer of the
# technology LEF and that "set_signal_res -l" and "set_signal_cap -l"
# give exactly the same timing as setting the numbers by hand.
#
# The geometry, the binding and the placement are identical in the three
# runs; only the way the unit RC reaches "&phys_sta" differs:
#
#   layer-opt  &phys_sta -l LAYER
#   set-layer  set_signal_res -l LAYER; set_signal_cap -l LAYER; &phys_sta
#   set-value  set_signal_res R; set_signal_cap C; &phys_sta
#
# where R and C are the values printed by the two "-l" commands.  The
# whole STA report of the three runs has to be byte-identical; the script
# also fails when a "-l" command cannot derive a value.
#
#   test/phys_signal_rc.sh [options] [design.aig ...]
#
#   -o dir    workspace directory [default = phys_rc_ws]
#   -r        remove the workspace before the run
#   -b dir    directory with the benchmarks, either directly or one
#             subdirectory per family [default = ../benchmarks/random_control]
#   -t file   technology LEF [default = $PDK/prtech/techLEF/N551P6M_ecos.lef]
#   -c file   cell LEF       [default = $PDK/IP/STD_cell/.../ics55_LLSC_H7CR.lef]
#   -l file   Liberty file   [default = ../work/lib/ics55_LLSC_H7CR_typ_tt_1p2_25_nldm.lib]
#   -L layer  routing layer whose RC is used [default = MET2]
#   -q        do not print the per-design progress lines
#
#   PDK, TECH, CELLS, LIB, BENCH and ABC can also be set in the environment.

set -u
ROOT=$(cd "$(dirname "$0")/.." && pwd)
ABC=${ABC:-$ROOT/abc}
PDK=${PDK:-$ROOT/../icsprout55-pdk}
TECH=${TECH:-$PDK/prtech/techLEF/N551P6M_ecos.lef}
CELLS=${CELLS:-$PDK/IP/STD_cell/ics55_LLSC_H7C_V1p10C100/ics55_LLSC_H7CR/lef/ics55_LLSC_H7CR.lef}
LIB=${LIB:-$ROOT/../work/lib/ics55_LLSC_H7CR_typ_tt_1p2_25_nldm.lib}
BENCH=${BENCH:-$ROOT/../benchmarks/random_control}
LAYER=MET2
WS=$ROOT/phys_rc_ws
RESET=0
QUIET=0

while getopts "o:rb:t:c:l:L:qh" opt; do
    case $opt in
        o) WS=$OPTARG ;;
        r) RESET=1 ;;
        b) BENCH=$OPTARG ;;
        t) TECH=$OPTARG ;;
        c) CELLS=$OPTARG ;;
        l) LIB=$OPTARG ;;
        L) LAYER=$OPTARG ;;
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
    designs=("$BENCH"/*.aig)
    [ -e "${designs[0]}" ] || designs=("$BENCH"/*/*.aig)
fi
if [ ${#designs[@]} -eq 0 ] || [ ! -e "${designs[0]}" ]; then
    echo "No benchmarks found in $BENCH" >&2
    exit 1
fi

if [ "$RESET" = 1 ]; then
    rm -rf "$WS"
fi
mkdir -p "$WS"

# the RC that the layer supplies, as printed by the -l commands
RC=$("$ABC" -c "read_lef -t $TECH; set_signal_res -l $LAYER; set_signal_cap -l $LAYER" 2>&1)
echo "$RC" | grep -q "Signal resistance: .* (layer " && \
echo "$RC" | grep -q "Signal capacitance: .* (layer " || {
    echo "$RC" >&2
    echo "Cannot derive the unit RC of layer \"$LAYER\" from $TECH" >&2
    exit 1
}
RES=$(echo "$RC" | sed -n 's/^Signal resistance: \(.*\) ohm\/um .*/\1/p')
CAP=$(echo "$RC" | sed -n 's/^Signal capacitance: \(.*\) pF\/um .*/\1/p')
[ -n "$RES" ] && [ -n "$CAP" ] || { echo "$RC" >&2; exit 1; }

failed=""
for d in "${designs[@]}"; do
    name=$(basename "$d" .aig)
    dir="$WS/$name"
    mkdir -p "$dir"
    prefix="read_lef -t $TECH; read_lef $CELLS; read_liberty $LIB"
    prefix="$prefix; &phys_start; &read $d; &phys_bind; &phys_place"
    if [ "$QUIET" != 1 ]; then
        echo "Checking $name with $LAYER (R=$RES ohm/um, C=$CAP pF/um)"
    fi
    "$ABC" -c "$prefix; &phys_sta -v -l $LAYER" > "$dir/layer-opt.log" 2>&1
    "$ABC" -c "$prefix; set_signal_res -l $LAYER; set_signal_cap -l $LAYER; &phys_sta -v" \
        > "$dir/set-layer.log" 2>&1
    "$ABC" -c "$prefix; set_signal_res $RES; set_signal_cap $CAP; &phys_sta -v" \
        > "$dir/set-value.log" 2>&1
    # compare the timing report only: the command-line banner differs
    sed -n '/^STA:/,$p' "$dir/layer-opt.log" > "$dir/layer-opt.sta"
    sed -n '/^STA:/,$p' "$dir/set-layer.log" > "$dir/set-layer.sta"
    sed -n '/^STA:/,$p' "$dir/set-value.log" > "$dir/set-value.sta"
    status="ok"
    for f in set-layer set-value; do
        if ! cmp -s "$dir/layer-opt.sta" "$dir/$f.sta"; then
            status="MISMATCH ($f)"
            diff -u "$dir/layer-opt.sta" "$dir/$f.sta" > "$dir/$f.diff"
        fi
    done
    if [ ! -s "$dir/layer-opt.sta" ]; then
        status="NO REPORT"
    fi
    printf '%-12s %s\n' "$name" "$status"
    [ "$status" = ok ] || failed="$failed $name"
done

if [ -n "$failed" ]; then
    echo "Mismatch:$failed (see $WS)" >&2
    exit 1
fi
echo "The layer-derived and the explicit unit RC give identical timing."
