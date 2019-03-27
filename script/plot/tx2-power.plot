#!/usr/bin/gnuplot -c

#
# This script draws single GPU frequency-power graph.
# Regardless of governor or frequency
#
# ARG1: Input date file name
# ARG2: Output plot file name
# ARG3: Benchmark name (for tagging)
#

print ARG0 . " Script Usage"
print "\n"
print "   " . "gnuplot -c " . ARG0 . " <input data filename> <output plot filename> <benchmark name>"
print "\n"
print "    * " . "ARG1: Input data file name"
print "    * " . "ARG2: Output plot file name"
print "    * " . "ARG3: Benchmark name (for tagging)"

#
set term png
set output ARG2

#
set title "TX2 Power Analysis (run: " . ARG3 . ")"
set key outside above  # Set key (legend) places at the outside of the plot

# x: Time
set xlabel "Time"
#set xtics (2500000000, 5000000000)
#set format x "%.1tx10^{%T} ns"
set format x "%.1t s"
set autoscale x

# y: Power
set ylabel "TX2 Power by Component"
set ytics nomirror
set autoscale y
set format y "%.0f mW"

plot ARG1 using 1:2 title "Power_{GPU}", ARG1 using 1:4 title "Power_{ALL}", ARG1 using 1:5 title "Power_{MEM}", ARG1 using 1:6 title "Power_{ALL-CPU}"

exit
