#!/usr/bin/env bash
# Paired ratios of the step-2 placement arms (see phys_step2_arms.sh).
#
# Every ratio of a design is relative to that design's "sparse" arm, and
# every arm of a design is placed in the same core, so a ratio is the effect
# of the placement model or of the netlist, not of the floorplan.
#
#   test/phys_step2_report.sh [arms.csv]
#
# Prints
#   * per-design ratios of the wirelength of the placement, of the reported
#     TMax (&phys_sta -C, the selected alternative only), and of the
#     choice-blind TMax of the same placement relative to its reported TMax
#   * the geometric mean of every ratio over the designs that have it
#   * the number of designs in which the wirelength beats the sparse arm
#   * the legalization control: the same arm with and without -l
#
# Rows of the per-design block
#   sel        the selection rounds on the sparse netlist
#   dense      the choice-blind placement of every alternative
#   dsel       dense_sel: the selection scores the dense placement
#   reduce     the sparse netlist materialized and placed choice-blind
#   d_b        dense with a budget scaled by -B (budget control)
#   dsel_b     dense_sel with the same scaled budget

set -u
CSV=${1:-phys_step2_ws/arms.csv}
[ -f "$CSV" ] || { echo "Cannot find $CSV" >&2; exit 1; }

awk -F, '
function col(name, arm, idx,    i) {
    for (i = 1; i <= nrow; i++)
        if (r_name[i] == name && r_arm[i] == arm) return r_val[i, idx]
    return ""
}
function ratio(d, arm, idx,    a, b) {
    a = col(d, arm, idx); b = col(d, "sparse", idx)
    if (a == "" || b == "" || b == 0) return ""
    return a / b
}
function gm(idxfield, k,    d, n, lg, v) {
    lg = 0; n = 0
    for (d = 1; d <= nd; d++) {
        v = ratio(order[d], arms[k], idxfield)
        if (v == "") continue
        lg += log(v); n++
    }
    return (n > 0) ? exp(lg / n) : 0
}
function wins(k,    d, n, w, a, b) {
    n = 0; w = 0
    for (d = 1; d <= nd; d++) {
        a = col(order[d], arms[k], "wire"); b = col(order[d], "sparse", "wire")
        if (a == "" || b == "") continue
        n++; if (a < b) w++
    }
    return w "/" n
}
$1 == "design" { next }
$4 !~ /^[0-9]+$/ { next }
{
    nrow++
    r_name[nrow] = $1; r_arm[nrow] = $2
    r_val[nrow, "cells"] = $4; r_val[nrow, "area"] = $5
    r_val[nrow, "wire"]  = $12; r_val[nrow, "occ"] = $9
    r_val[nrow, "cong"]  = $10; r_val[nrow, "ovfl"] = $11
    r_val[nrow, "tmax"]  = $13; r_val[nrow, "blind"] = $14
    r_val[nrow, "pa"]    = $15
    if (!(r_name[nrow] in seen)) { seen[r_name[nrow]] = 1; order[++nd] = r_name[nrow] }
}
END {
    split("sel dense dense_sel reduce dense_b dense_sel_b", arms, " ")
    hdr = sprintf("%-12s %9s %7s %7s %7s %7s %7s %7s", "design", "sparse",
                  "sel", "dense", "dsel", "reduce", "d_b", "dsel_b")
    printf "%s\n", hdr
    for (d = 1; d <= nd; d++) {
        name = order[d]
        printf "%-12s %9.1f", name, col(name, "sparse", "wire")
        for (k = 1; k <= 6; k++) {
            v = ratio(name, arms[k], "wire")
            if (v == "") printf " %7s", "-"; else printf " %7.3f", v
        }
        printf "\n"
    }
    printf "\nTMax of the reported netlist (&phys_sta -C)\n%s\n", hdr
    for (d = 1; d <= nd; d++) {
        name = order[d]
        printf "%-12s %9.1f", name, col(name, "sparse", "tmax")
        for (k = 1; k <= 6; k++) {
            v = ratio(name, arms[k], "tmax")
            if (v == "") printf " %7s", "-"; else printf " %7.3f", v
        }
        printf "\n"
    }
    printf "\nchoice-blind TMax / reported TMax of the same placement\n"
    printf "%-12s %9s", "design", ""
    for (k = 1; k <= 6; k++) printf " %7s", arms[k]
    printf "\n"
    for (d = 1; d <= nd; d++) {
        name = order[d]
        printf "%-12s %9s", name, ""
        for (k = 1; k <= 6; k++) {
            b = col(name, arms[k], "blind"); t = col(name, arms[k], "tmax")
            if (t == "" || t == 0) printf " %7s", "-"
            else                   printf " %7.3f", b / t
        }
        printf "\n"
    }
    printf "\ngeomean over the designs that have both arms\n"
    printf "%-22s %7s %7s %7s %7s %7s %7s\n", "", "sel", "dense", "dsel", "reduce", "d_b", "dsel_b"
    printf "%-22s", "  wire / sparse"
    for (k = 1; k <= 6; k++) printf " %7.3f", gm("wire", k)
    printf "\n%-22s", "  TMax / sparse"
    for (k = 1; k <= 6; k++) printf " %7.3f", gm("tmax", k)
    printf "\n%-22s", "  blind / reported"
    for (k = 1; k <= 6; k++) {
        lg = 0; n = 0
        for (d = 1; d <= nd; d++) {
            b = col(order[d], arms[k], "blind"); t = col(order[d], arms[k], "tmax")
            if (b == "" || t == "" || t == 0) continue
            lg += log(b / t); n++
        }
        printf " %7.3f", (n > 0) ? exp(lg / n) : 0
    }
    printf "\n%-22s", "  wire wins / n"
    for (k = 1; k <= 6; k++) printf " %7s", wins(k)
    printf "\nA4 control: reduce / sparse, the same cell set placed choice-blind\n"
    printf "%-12s %7s %7s %7s %7s %7s\n", "design", "cells", "area", "wire", "TMax", "cong"
    for (d = 1; d <= nd; d++) {
        name = order[d]
        a = col(name, "sparse", "cells"); b = col(name, "reduce", "cells")
        printf "%-12s", name
        if (b == "" || a == "" || a == 0) { printf " %7s %7s %7s %7s %7s\n", "-", "-", "-", "-", "-"; continue }
        printf " %7.3f %7.3f %7.3f %7.3f",
            b / a,
            col(name, "reduce", "area") / col(name, "sparse", "area"),
            col(name, "reduce", "wire") / col(name, "sparse", "wire"),
            col(name, "reduce", "tmax") / col(name, "sparse", "tmax")
        printf " %7.3f\n", col(name, "reduce", "cong") / (col(name, "sparse", "cong") + 1e-9)
    }
    printf "\n\ndense_sel / dense (the selection on a fixed placement model)\n"
    printf "%-22s %7.3f\n", "  wire", gm2("wire", 3, 2)
    printf "%-22s %7.3f\n", "  TMax -C", gm2("tmax", 3, 2)
    printf "\n\nlegalization: wire of <arm>_l / wire of <arm>\n"
    split("sparse sel dense dense_sel reduce", ls, " ")
    printf "%-12s %7s %7s %7s %7s %7s\n", "design", ls[1], ls[2], ls[3], ls[4], ls[5]
    for (d = 1; d <= nd; d++) {
        name = order[d]
        printf "%-12s", name
        for (k = 1; k <= 5; k++) {
            a = col(name, ls[k] "_l", "wire"); b = col(name, ls[k], "wire")
            if (a == "" || b == "" || b == 0) printf " %7s", "-"
            else                               printf " %7.3f", a / b
        }
        printf "\n"
    }
    printf "\ngeomean of <arm>_l / <arm>\n%-12s", ""
    for (k = 1; k <= 5; k++) printf " %7s", ls[k]
    printf "\n%-12s", "  wire"
    for (k = 1; k <= 5; k++) {
        lg = 0; n = 0
        for (d = 1; d <= nd; d++) {
            a = col(order[d], ls[k] "_l", "wire"); b = col(order[d], ls[k], "wire")
            if (a == "" || b == "" || b == 0) continue
            lg += log(a / b); n++
        }
        printf " %7.3f", (n > 0) ? exp(lg / n) : 0
    }
    printf "\n"
}
function gm2(idxfield, kA, kB,    d, n, lg, a, b) {
    lg = 0; n = 0
    for (d = 1; d <= nd; d++) {
        a = col(order[d], arms[kA], idxfield); b = col(order[d], arms[kB], idxfield)
        if (a == "" || b == "" || b == 0) continue
        lg += log(a / b); n++
    }
    return (n > 0) ? exp(lg / n) : 0
}
' "$CSV"
