#!/usr/bin/env bash
# Compares the quality of a physical-synthesis run against a baseline run
# written by test/phys_flow.sh and checks the acceptance thresholds of the
# physical flow (the M4 gate):
#
#   area          <= AREA_RATIO  x the baseline
#   TMax          <= TMAX_RATIO  x the baseline
#   wirelength    <= WIRE_RATIO  x the baseline
#   overflow bins <= the baseline (absolute count)
#   runtime       <= TIME_RATIO  x the baseline (only when the run logs
#                                 are given, because summary.csv has no time)
#
# Both workspaces must hold the summary.csv of phys_flow.sh and must have
# been run on the same benchmarks with the same number of iterations.
#
#   test/phys_compare.sh [options] baseline_ws candidate_ws [base.log cand.log]
#
#   -i num    compare iteration num [default = 3]
#   -A num    area ratio limit [default = 1.05]
#   -T num    TMax ratio limit [default = 1.05]
#   -W num    wirelength ratio limit [default = 1.00]
#   -R num    runtime ratio limit [default = 3.00]
#   -o file   write the per-design table as CSV to file
#   -v        print every design, also those that pass [default = no]
#
# The exit status is 0 when every design passes and 1 otherwise.

set -u

ITER=3
AREA_RATIO=1.05
TMAX_RATIO=1.05
WIRE_RATIO=1.00
TIME_RATIO=3.00
CSV=""
VERBOSE=0

while getopts "i:A:T:W:R:o:vh" opt; do
    case $opt in
        i) ITER=$OPTARG ;;
        A) AREA_RATIO=$OPTARG ;;
        T) TMAX_RATIO=$OPTARG ;;
        W) WIRE_RATIO=$OPTARG ;;
        R) TIME_RATIO=$OPTARG ;;
        o) CSV=$OPTARG ;;
        v) VERBOSE=1 ;;
        *) sed -n '2,/^$/p' "$0" | sed 's/^# \{0,1\}//'; exit 1 ;;
    esac
done
shift $((OPTIND - 1))

if [ $# -lt 2 ] || [ $# -gt 4 ]; then
    sed -n '2,/^$/p' "$0" | sed 's/^# \{0,1\}//'
    exit 1
fi
BASE_WS=$1
CAND_WS=$2
BASE_LOG=${3:-}
CAND_LOG=${4:-}
for f in "$BASE_WS/summary.csv" "$CAND_WS/summary.csv"; do
    [ -f "$f" ] || { echo "Cannot find $f" >&2; exit 1; }
done
if [ -n "$BASE_LOG" ]; then
    [ -f "$BASE_LOG" ] || { echo "Cannot find $BASE_LOG" >&2; exit 1; }
fi
if [ -n "$CAND_LOG" ]; then
    [ -f "$CAND_LOG" ] || { echo "Cannot find $CAND_LOG" >&2; exit 1; }
fi

# Join the two workspaces on the design name at the requested iteration.
TMP=$(mktemp)
trap 'rm -f "$TMP"' EXIT

# join current rows and compare; the CSV field order is
# design,iter,cells,util,rows_used,rows_total,max_row_occ,wirelength,area,cong,ovfl,tmax
awk -F, -v it="$ITER" '
    FNR == NR {
        if ($2 == it) base[$1] = $0
        next
    }
    $2 == it {
        if (!($1 in base))
            next
        split(base[$1], b, ",")
        printf "%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s\n", $1,
            b[8], $8, b[9], $9, b[11], $11, b[12], $12, b[3], $3, b[10], $10
    }
' "$BASE_WS/summary.csv" "$CAND_WS/summary.csv" > "$TMP"

# Per-design runtime ratios from the progress lines of the two logs.
TIMEFILE=$(mktemp)
trap 'rm -f "$TMP" "$TIMEFILE"' EXIT
if [ -n "$BASE_LOG" ] && [ -n "$CAND_LOG" ]; then
    awk '
        FNR == NR {
            if ($2 == "ok" || $2 == "FAILED") {
                t = $3; sub(/s$/, "", t); bt[$1] = t
            }
            next
        }
        ($2 == "ok" || $2 == "FAILED") {
            t = $3; sub(/s$/, "", t)
            if (bt[$1] + 0 > 0)
                printf "%s,%.3f\n", $1, t / bt[$1]
        }
    ' "$BASE_LOG" "$CAND_LOG" > "$TIMEFILE"
fi

awk -F, -v ar="$AREA_RATIO" -v tr="$TMAX_RATIO" \
    -v wr="$WIRE_RATIO" -v rr="$TIME_RATIO" \
    -v verbose="$VERBOSE" -v csv="$CSV" -v tf="$TIMEFILE" '
function ratio(a, b,    x) {
    if (b + 0 == 0) return (a + 0 == 0) ? 1.0 : 999.0
    x = (a + 0) / (b + 0)
    return x
}
BEGIN {
    while ((getline line < tf) > 0) {
        split(line, t, ",")
        tr_by[t[1]] = t[2]
    }
    close(tf)
    if (csv != "") {
        printf "design,area_base,area_cand,area_ratio,wire_base,wire_cand,wire_ratio,ovfl_base,ovfl_cand,tmax_base,tmax_cand,tmax_ratio,cells_base,cells_cand,cong_base,cong_cand,time_ratio,status\n" > csv
    }
}
{
    # fields of TMP:
    # 1 design, 2 wire_base, 3 wire_cand, 4 area_base, 5 area_cand,
    # 6 ovfl_base, 7 ovfl_cand, 8 tmax_base, 9 tmax_cand,
    # 10 cells_base, 11 cells_cand, 12 cong_base, 13 cong_cand
    name = $1
    o0 = ($6 == "-") ? 0 : $6 + 0
    o1 = ($7 == "-") ? 0 : $7 + 0
    ra = ratio($5, $4)
    rw = ratio($3, $2)
    rt = ratio($9, $8)
    me = (name in tr_by) ? tr_by[name] + 0 : -1
    bad = 0
    why = ""
    if (ra > ar) { bad = 1; why = why " area" }
    if (rt > tr) { bad = 1; why = why " tmax" }
    if (rw > wr) { bad = 1; why = why " wire" }
    if (o1 > o0) { bad = 1; why = why " overflow" }
    if (me >= 0 && me > rr) { bad = 1; why = why " runtime" }
    status = bad ? "FAIL" : "pass"
    if (bad) nfail++
    n++
    if (bad || verbose)
        printf "%-12s area %7.1f/%-7.1f %5.3f  wire %9.1f/%-9.1f %5.3f  ovfl %2d/%-2d  tmax %9.1f/%-9.1f %5.3f  cells %6d/%-6d  cong %4.2f/%-4.2f  time %s  %s%s\n",
            name, $5, $4, ra, $3, $2, rw, o1, o0, $9, $8, rt, $11, $10, $13, $12,
            (me < 0 ? " -- " : sprintf("%5.2f", me)), status, why
    if (csv != "")
        printf "%s,%.1f,%.1f,%.3f,%.1f,%.1f,%.3f,%d,%d,%.1f,%.1f,%.3f,%d,%d,%.2f,%.2f,%.3f,%s\n",
            name, $4, $5, ra, $2, $3, rw, o0, o1, $8, $9, rt, $10, $11, $12, $13,
            (me < 0 ? -1 : me), (bad ? "FAIL" : "pass") > csv
}
END {
    printf "\n%d design(s), %d failed\n", n, nfail
    printf "limits: area <= %.3fx, tmax <= %.3fx, wire <= %.3fx, overflow not worse, runtime <= %.2fx when logged\n",
        ar, tr, wr, rr
    exit (nfail > 0)
}' "$TMP"
