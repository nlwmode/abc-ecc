#!/usr/bin/env bash
# Controlled comparison of the two interconnect models of &phys_sta: the
# rectilinear Steiner tree (the default) and the minimum spanning tree
# (&phys_sta -m).  Both models run on the same placement, so the only
# difference is the wire model: the placement, the cell binding and the
# gate timing are identical.
#
# Two stages are measured:
#
#   pre   the virtual netlist of &phys_bind, placed once, before mapping
#   post  the mapped netlist, after ITER rounds of
#         &phys_map; &phys_place; &phys_sta, placed and timed once more
#
# In both stages the Steiner run comes first and the spanning-tree run
# second, on the identical placement.  Every run prints the total wire
# length, the number of Steiner points, the total net capacitance, the
# slowest net and the longest path, which this script collects into
# $WS/steiner.csv, one row per design, stage and model:
#
#   design,stage,model,wire_um,cap_pf,steiner_pts,steiner_nets,
#   tmax_ps,wns_ps,worst_net_ps,worst_net_sinks,steiner_run,steiner_rej
#
# and prints a per-design table plus the geometric-mean ratios of the
# Steiner model over the spanning-tree model.
#
#   test/phys_steiner_exp.sh [options] [design.aig ...]
#
#   -o dir    workspace directory [default = phys_steiner_ws]
#   -b dir    directory with the benchmarks, either directly or one
#             subdirectory per family [default = ../benchmarks]
#   -i num    mapping rounds of the post stage [default = 3]
#   -M mode   mapping mode of the post stage: root (&phys_map -s, the
#             default) or phys (&phys_map with every drive)
#   -s stage  pre, post or both [default = both]
#   -q        do not print the per-design progress lines
#
#   PDK, TECH, CELLS, LIB, BENCH and ABC can also be set in the
#   environment.

set -u
ROOT=$(cd "$(dirname "$0")/.." && pwd)
ABC=${ABC:-$ROOT/abc}
PDK=${PDK:-$ROOT/../icsprout55-pdk}
TECH=${TECH:-$PDK/prtech/techLEF/N551P6M.lef}
CELLS=${CELLS:-$PDK/IP/STD_cell/ics55_LLSC_H7C_V1p10C100/ics55_LLSC_H7CR/lef/ics55_LLSC_H7CR.lef}
LIB=${LIB:-$ROOT/../work/lib/ics55_LLSC_H7CR_typ_tt_1p2_25_nldm.lib}
BENCH=${BENCH:-$ROOT/../benchmarks}

WS=phys_steiner_ws
ITER=3
MODE=root
STAGE=both
QUIET=0

usage() { sed -n '2,/^$/p' "$0" | sed 's/^# \{0,1\}//'; exit 1; }

while getopts "o:b:i:M:s:qh" opt; do
    case $opt in
        o) WS=$OPTARG ;;
        b) BENCH=$OPTARG ;;
        i) ITER=$OPTARG ;;
        M) MODE=$OPTARG ;;
        s) STAGE=$OPTARG ;;
        q) QUIET=1 ;;
        *) usage ;;
    esac
done
shift $((OPTIND-1))

[ -f "$TECH" ]  || { echo "Cannot find the technology LEF: $TECH" >&2; exit 1; }
[ -f "$CELLS" ] || { echo "Cannot find the cell LEF: $CELLS" >&2; exit 1; }
[ -f "$LIB" ]   || { echo "Cannot find the Liberty file: $LIB" >&2; exit 1; }
[ -x "$ABC" ]   || { echo "Cannot find abc: $ABC" >&2; exit 1; }
case $MODE in
    root) MAPCMD="&phys_map -s" ;;
    phys) MAPCMD="&phys_map" ;;
    *) echo "Unknown mapping mode: $MODE (use root or phys)" >&2; exit 1 ;;
esac
case $STAGE in
    pre)  stages=(pre) ;;
    post) stages=(post) ;;
    both) stages=(pre post) ;;
    *) echo "Unknown stage: $STAGE (use pre, post or both)" >&2; exit 1 ;;
esac

if [ $# -gt 0 ]; then
    designs=("$@")
else
    designs=("$BENCH"/*.aig)
    [ -e "${designs[0]}" ] || designs=("$BENCH"/*/*.aig)
fi
[ -f "${designs[0]}" ] || { echo "No benchmarks found in $BENCH" >&2; exit 1; }

mkdir -p "$WS"
CSV=$WS/steiner.csv
printf 'design,stage,model,wire_um,cap_pf,steiner_pts,steiner_nets,tmax_ps,wns_ps,worst_net_ps,worst_net_sinks,steiner_run,steiner_rej\n' > "$CSV"

# reads one console log and prints the rows of the two verbose STA runs
parse_log() {
    gawk -v des="$1" -v stg="$2" '
    /^STA: wire / {
        model = ($0 ~ /Steiner tree/) ? "steiner" : "mst";
        have = 1; run = ""; rej = "";
        match($0, /wire ([0-9.]+) um/, m);            wire = m[1];
        match($0, /([0-9]+) Steiner points/, m);      pts  = m[1];
        match($0, /on ([0-9]+) nets/, m);             nets = m[1];
        match($0, /\), ([0-9.]+) pF/, m);             cap  = m[1];
    }
    /^STA: Steiner search ran on / {
        match($0, /ran on ([0-9]+) nets/, m);         run  = m[1];
        match($0, /, ([0-9]+) had no shorter/, m);    rej  = m[1];
    }
    /^STA: slowest net: / {
        match($0, /, ([0-9]+) sinks/, m);             sinks = m[1];
        match($0, /sinks, ([0-9.]+) ps/, m);          worst = m[1];
    }
    /^STA: longest path / {
        # the non-verbose &phys_sta before the two measured runs also
        # prints a longest path; only a block opened by a wire line counts
        if (!have) next;
        match($0, /longest path ([0-9.]+) ps/, m);    tmax = m[1];
        match($0, /WNS ([0-9.]+|-?[0-9.]+) ps/, m);   wns  = m[1];
        printf "%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s\n",
               des, stg, model, wire, cap, pts, nets, tmax, wns, worst, sinks, run, rej;
        have = 0;
    }' "$3"
}

failed=""
for d in "${designs[@]}"; do
    name=$(basename "$d" .aig)
    mkdir -p "$WS/$name"
    for stage in "${stages[@]}"; do
        log="$WS/$name/${stage}.log"
        cmd="read_lef -t $TECH; read_lef $CELLS; read_liberty $LIB"
        cmd="$cmd; &phys_start; &read $d; &phys_bind; &phys_place; &phys_sta"
        if [ "$stage" = "post" ]; then
            for i in $(seq 1 "$ITER"); do
                cmd="$cmd; $MAPCMD; &phys_place; &phys_sta"
            done
        fi
        # the two models on the same placement: Steiner first, then MST
        cmd="$cmd; &phys_sta -v; &phys_sta -v -m; &phys_end"
        start=$(date +%s)
        "$ABC" -c "$cmd" > "$log" 2>&1
        rc=$?
        secs=$(( $(date +%s) - start ))
        if [ $rc -ne 0 ] || ! grep -q "^STA: longest path" "$log"; then
            echo "[$name] $stage: FAILED (rc=$rc, see $log)" >&2
            failed="$failed $name($stage)"
            continue
        fi
        parse_log "$name" "$stage" "$log" >> "$CSV"
        [ $QUIET -eq 1 ] || printf '[%s] %-5s %3ds\n' "$name" "$stage" "$secs"
    done
done

echo
echo "=== Steiner vs spanning tree (ratios are Steiner / MST, geometric mean)"
gawk -F, 'NR > 1 {
    k = $2 SUBSEP $1; n[k] = 1;
    w[$2 SUBSEP $1 SUBSEP $3] = $4;
    t[$2 SUBSEP $1 SUBSEP $3] = $8;
    if ($3 == "steiner" && $6 > 0) nets[$2]++;
    if ($3 == "steiner") { run[$2] += $12; rej[$2] += $13; }
}
END {
    printf "%-6s %5s %10s %10s %8s %10s %10s %8s %9s %8s %8s\n",
           "stage", "des", "wire_ste", "wire_mst", "wire_r", "tmax_ste", "tmax_mst", "tmax_r",
           "ste_nets", "attempted", "rejected";
    for (k in n) {
        split(k, a, SUBSEP); s = a[1]; d = a[2]; cnt[s]++;
        lw[s] += log(w[s SUBSEP d SUBSEP "steiner"]);
        lm[s] += log(w[s SUBSEP d SUBSEP "mst"]);
        lt[s] += log(t[s SUBSEP d SUBSEP "steiner"]);
        lu[s] += log(t[s SUBSEP d SUBSEP "mst"]);
    }
    for (s in cnt)
        printf "%-6s %5d %10.2f %10.2f %8.4f %10.2f %10.2f %8.4f %9d %8d %8d\n",
               s, cnt[s], exp(lw[s]/cnt[s]), exp(lm[s]/cnt[s]), exp((lw[s]-lm[s])/cnt[s]),
               exp(lt[s]/cnt[s]), exp(lu[s]/cnt[s]), exp((lt[s]-lu[s])/cnt[s]),
               nets[s], run[s], rej[s];
}' "$CSV"

echo
echo "wrote $CSV"
if [ -n "$failed" ]; then
    echo "FAILED:$failed" >&2
    exit 1
fi
