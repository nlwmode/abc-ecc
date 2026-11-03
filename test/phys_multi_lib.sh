#!/usr/bin/env bash
# Checks the corner handling of "read_liberty" and the corner options of
# the session commands:
#
#   read_liberty -k tt lib1.lib; read_liberty -k tt lib2.lib
#   read_liberty -k ss lib3.lib; read_liberty -k ss lib4.lib
#   &phys_start -k tt; ...; &phys_sta -k ss
#
# Two Liberty files read into the same corner are merged, which requires
# distinct cell names (the H7CR and H7CH families have distinct names);
# two corners keep the same cell names with different timing.  The design
# is placed once with the first corner and timed with every corner, both
# before the mapping (supergates) and after it (mapped cells).
#
# The script also checks the corners named after the PVT of the files:
# "read_liberty" without "-k" stores "tt_1.2_25", "ss_1.08_-40" and so on,
# and "&phys_start" without "-k" opens the middle corner of the PVT order
# (process fast to slow, then voltage, then temperature), which is tt when
# the ff, tt and ss files are read in any order.
#
# The script fails when a corner cannot be read, when a corner cannot be
# timed, when reading a file twice into the same corner is accepted, when
# reading Liberty data during an open session is accepted, when the
# PVT-named default corner is wrong, when mapping into a corner that lacks
# the mapped cells is accepted, or when switching the timing library to a
# corner that lacks bound cells is accepted.
#
#   test/phys_multi_lib.sh [options] [design.aig ...]
#
#   -o dir     workspace directory [default = phys_multi_ws]
#   -r         remove the workspace before the run
#   -t file    technology LEF [default = $PDK/prtech/techLEF/N551P6M_ecos.lef]
#   -c file    cell LEF (repeatable; default = H7CR and H7CH)
#   -k c:file  add a Liberty file to corner c (repeatable; default = the
#              tt and ss corners of H7CR and H7CH, two files per corner)
#   -d file    design (repeatable; default = benchmarks/random_control/ctrl.aig)
#   -q         do not print the per-design progress lines
#
#   PDK, TECH, CELLS, CORNERS, BENCH and ABC can also be set in the
#   environment; CORNERS is a space-separated list of "corner:file".

set -u
ROOT=$(cd "$(dirname "$0")/.." && pwd)
ABC=${ABC:-$ROOT/abc}
PDK=${PDK:-$ROOT/../icsprout55-pdk}
P=$PDK/IP/STD_cell/ics55_LLSC_H7C_V1p10C100
TECH=${TECH:-$PDK/prtech/techLEF/N551P6M_ecos.lef}
WS=$ROOT/phys_multi_ws
RESET=0
QUIET=0
CELLS=()
CORNERS=()
DESIGNS=()

while getopts "o:rt:c:k:d:qh" opt; do
    case $opt in
        o) WS=$OPTARG ;;
        r) RESET=1 ;;
        t) TECH=$OPTARG ;;
        c) CELLS+=("$OPTARG") ;;
        k) CORNERS+=("$OPTARG") ;;
        d) DESIGNS+=("$OPTARG") ;;
        q) QUIET=1 ;;
        *) sed -n '2,/^$/p' "$0" | sed 's/^# \{0,1\}//'; exit 1 ;;
    esac
done
shift $((OPTIND - 1))
DESIGNS+=("$@")

if [ ${#CELLS[@]} -eq 0 ]; then
    CELLS=("$P/ics55_LLSC_H7CR/lef/ics55_LLSC_H7CR.lef"
           "$P/ics55_LLSC_H7CH/lef/ics55_LLSC_H7CH.lef")
fi
if [ ${#CORNERS[@]} -eq 0 ]; then
    CORNERS=("tt:$P/ics55_LLSC_H7CR/liberty/ics55_LLSC_H7CR_typ_tt_1p2_25_nldm.lib"
             "tt:$P/ics55_LLSC_H7CH/liberty/ics55_LLSC_H7CH_typ_tt_1p2_25_nldm.lib"
             "ss:$P/ics55_LLSC_H7CR/liberty/ics55_LLSC_H7CR_ss_cworst_1p08_m40_nldm.lib"
             "ss:$P/ics55_LLSC_H7CH/liberty/ics55_LLSC_H7CH_ss_cworst_1p08_m40_nldm.lib")
fi
if [ ${#DESIGNS[@]} -eq 0 ]; then
    DESIGNS=("$ROOT/../benchmarks/random_control/ctrl.aig")
fi

[ -x "$ABC" ]  || { echo "Cannot find the ABC binary: $ABC" >&2; exit 1; }
[ -f "$TECH" ] || { echo "Cannot find the technology LEF: $TECH" >&2; exit 1; }
for f in "${CELLS[@]}"; do
    [ -f "$f" ] || { echo "Cannot find the cell LEF: $f" >&2; exit 1; }
done

names=()        # corner names in the read order
first_file=()   # first Liberty file of each corner
cmd="read_lef -t $TECH"
for f in "${CELLS[@]}"; do
    cmd="$cmd; read_lef $f"
done
for spec in "${CORNERS[@]}"; do
    case $spec in
        *:*) ;;
        *) echo "Bad corner specification \"$spec\" (use corner:file)" >&2; exit 1 ;;
    esac
    c=${spec%%:*}
    f=${spec#*:}
    [ -f "$f" ] || { echo "Cannot find the Liberty file: $f" >&2; exit 1; }
    found=0
    for n in "${names[@]}"; do [ "$n" = "$c" ] && found=1; done
    [ "$found" = 1 ] || names+=("$c")
    [ ${#first_file[@]} -ge ${#names[@]} ] || first_file+=("$f")
    cmd="$cmd; read_liberty -k $c $f"
done
[ ${#names[@]} -ge 2 ] || { echo "At least two corners are needed." >&2; exit 1; }
start=${names[0]}

if [ "$RESET" = 1 ]; then
    rm -rf "$WS"
fi
mkdir -p "$WS"
failed=""

# the corners are reported by print_phys
"$ABC" -c "$cmd; print_phys" > "$WS/corners.log" 2>&1
for c in "${names[@]}"; do
    grep -q "^Liberty\[$c\]:" "$WS/corners.log" || {
        echo "Corner \"$c\" is not reported by print_phys" >&2
        failed="$failed print_phys"
    }
done
grep -q '^Error:' "$WS/corners.log" && failed="$failed read"

# reading a file again into the same corner is refused
"$ABC" -c "$cmd; read_liberty -k $start ${first_file[0]}" > "$WS/duplicate.log" 2>&1
grep -q 'already in corner' "$WS/duplicate.log" || {
    echo "Reading a file twice into the same corner was accepted" >&2
    failed="$failed duplicate"
}

# reading Liberty data during an open session is refused
"$ABC" -c "$cmd; &phys_start -k $start; read_liberty -k $start ${first_file[0]}" > "$WS/session.log" 2>&1
grep -q 'session is open' "$WS/session.log" || {
    echo "Reading Liberty data during an open session was accepted" >&2
    failed="$failed session"
}

# corners named by PVT, and the middle corner of the PVT order as default;
# the files are read as ff, ss, tt, so the answer does not depend on the
# read order
pvt_cmd="read_lef -t $TECH"
for f in "${CELLS[@]}"; do
    pvt_cmd="$pvt_cmd; read_lef $f"
done
pvt_files=()
for spec in \
    "$P/ics55_LLSC_H7CR/liberty/ics55_LLSC_H7CR_ff_cbest_1p32_125_nldm.lib" \
    "$P/ics55_LLSC_H7CH/liberty/ics55_LLSC_H7CH_ff_cbest_1p32_125_nldm.lib" \
    "$P/ics55_LLSC_H7CR/liberty/ics55_LLSC_H7CR_ss_cworst_1p08_m40_nldm.lib" \
    "$P/ics55_LLSC_H7CH/liberty/ics55_LLSC_H7CH_ss_cworst_1p08_m40_nldm.lib" \
    "$P/ics55_LLSC_H7CR/liberty/ics55_LLSC_H7CR_typ_tt_1p2_25_nldm.lib" \
    "$P/ics55_LLSC_H7CH/liberty/ics55_LLSC_H7CH_typ_tt_1p2_25_nldm.lib"
do
    pvt_files+=("$spec")
    pvt_cmd="$pvt_cmd; read_liberty $spec"
done
pvt_ok=1
for f in "${pvt_files[@]}"; do [ -f "$f" ] || pvt_ok=0; done
if [ "$pvt_ok" = 1 ]; then
    "$ABC" -c "$pvt_cmd; print_phys" > "$WS/pvt.log" 2>&1
    for c in ff_1.32_125 ss_1.08_-40 tt_1.2_25; do
        grep -q "^Liberty\[$c\]:" "$WS/pvt.log" || {
            echo "The PVT-named corner \"$c\" was not stored" >&2
            failed="$failed pvt"
        }
    done
    grep -Eq 'corner "tt_1.2_25" holds [0-9]+ cells \([0-9]+ added\)' "$WS/pvt.log" || {
        echo "The two families of a PVT corner were not merged" >&2
        failed="$failed pvt"
    }
    "$ABC" -c "$pvt_cmd; &phys_start" > "$WS/pvt_start.log" 2>&1
    grep -q 'Started physical-synthesis session: corner "tt_1.2_25"' "$WS/pvt_start.log" || {
        echo "The default corner is not the middle corner of the PVT order" >&2
        failed="$failed pvt-default"
    }
    # The mapping library is the one of the "&phys_start" corner.  Mapping
    # after "&phys_sta -k" into a corner that holds another cell family
    # must be refused, because the mapped cells would be left unbound and
    # the timing would silently ignore them.
    mismatch_cmd="read_lef -t $TECH"
    for f in "${CELLS[@]}"; do
        mismatch_cmd="$mismatch_cmd; read_lef $f"
    done
    mismatch_cmd="$mismatch_cmd; read_liberty ${pvt_files[4]}"
    mismatch_cmd="$mismatch_cmd; read_liberty ${pvt_files[5]}"
    mismatch_cmd="$mismatch_cmd; read_liberty ${pvt_files[3]}"
    "$ABC" -c "$mismatch_cmd; &phys_start -k tt_1.2_25; &read ${DESIGNS[0]}; &phys_bind; &phys_place; &phys_sta; &phys_sta -k ss_1.08_-40; &phys_map" \
        > "$WS/mismatch.log" 2>&1
    grep -q 'missing from Liberty corner' "$WS/mismatch.log" || {
        echo "Mapping into a corner without the mapped cells was not refused" >&2
        failed="$failed mismatch"
    }
    # switching the timing library after the mapping is checked cell by
    # cell, so the same corner must be refused without changing the session
    "$ABC" -c "$mismatch_cmd; &phys_start -k tt_1.2_25; &read ${DESIGNS[0]}; &phys_bind; &phys_place; &phys_sta; &phys_map; &phys_place; &phys_sta; &phys_sta -k ss_1.08_-40" \
        > "$WS/switch.log" 2>&1
    grep -q 'the timing library is not switched' "$WS/switch.log" || {
        echo "Switching to a corner without the bound cells was not refused" >&2
        failed="$failed switch"
    }
else
    [ "$QUIET" = 1 ] || echo "PVT test skipped: some ff/ss/tt Liberty files are missing"
fi

for d in "${DESIGNS[@]}"; do
    name=$(basename "$d" .aig)
    log="$WS/$name.log"
    # place once with the first corner, then time every corner before and
    # after the mapping; the STA lines come in the corner order
    run="$cmd; &phys_start -k $start; &read $d; &phys_bind; &phys_place; &phys_sta"
    run="$run; &phys_map; &phys_place"
    for c in "${names[@]}"; do
        if [ "$c" = "$start" ]; then
            run="$run; &phys_sta"
        else
            run="$run; &phys_sta -k $c"
        fi
    done
    run="$run; &phys_end"
    "$ABC" -c "$run" > "$log" 2>&1
    if grep -q '^Error:' "$log"; then
        failed="$failed $name"
        [ "$QUIET" = 1 ] || echo "$name: error (see $log)"
        continue
    fi
    # the first STA line is the unmapped network of the first corner, the
    # next ones are the mapped network of every corner in the read order
    tmax=($(sed -n 's/^STA: longest path \([0-9.]*\) ps.*/\1/p' "$log"))
    [ ${#tmax[@]} -eq $((1 + ${#names[@]})) ] || {
        echo "$name: expected $((1 + ${#names[@]})) timing reports, got ${#tmax[@]}" >&2
        failed="$failed $name"
        continue
    }
    if ! grep -q '0 not found in Liberty' "$log"; then
        echo "$name: the binding did not find every cell in the corner" >&2
        failed="$failed $name"
        continue
    fi
    if [ "$QUIET" != 1 ]; then
        printf '%-12s unmapped %s ps;' "$name" "${tmax[0]}"
        i=0
        for c in "${names[@]}"; do
            printf ' %s %s ps;' "$c" "${tmax[$((1 + i))]}"
            i=$((i + 1))
        done
        echo
    fi
done

if [ -n "$failed" ]; then
    echo "FAILED:$failed (see $WS)" >&2
    exit 1
fi
echo "Every corner is stored and timed; duplicate, in-session and mismatched reads are refused."
