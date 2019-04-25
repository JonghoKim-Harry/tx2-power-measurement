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
set title "TX2 GPU Power and Frequency (run: " . ARG3 . ")"
set key outside above  # Set key (legend) places at the outside of the plot

# x: Time
set xlabel "Time"
set xtics (1000000000)
set format x "%.1tx10^{%T} ns"
set autoscale x

# y: GPU Power
set ylabel "GPU Power"
set ytics nomirror
set autoscale y
set format y "%.0f mW"

# y2: GPU Frequency
# Available frequencies in Hz: 140250000 229500000 318750000 408000000 497250000 586500000 675750000 765000000 854250000 943500000 1032750000 1122000000 1211250000 1300500000
# Min GPU frequency is 140250000 Hz
# Max GPU frequency is 1122000000 Hz
set y2label "GPU Frequency"
set y2range [0:1122000000]
set y2tics nomirror
set y2tics (0, 140250000, 229500000, 318750000, 408000000, 497250000, 586500000, 675750000, 765000000, 854250000, 943500000, 1032750000, 1122000000, 1211250000, 1300500000)
set y2tics add ("Freq_{GPU MIN} (140.3 MHz)" 140250000, "Freq_{GPU MAX} (1.1 GHz)" 1122000000)
set format y2 "%.1s %cHz"

plot ARG1 using 2:3 title "Power_{GPU}" axes x1y1, ARG1 using 2:4 title "Freq_{GPU}" axes x1y2

exit
