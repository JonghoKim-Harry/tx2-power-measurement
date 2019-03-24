#!/usr/bin/gnuplot -c

#
# This script draws GPU power graph for CIFAR-10
#


#
set term png
set output "cifar10-top5freq-comparison.png"

#
set title "TX2 GPU Power"
set key outside above  # Set key (legend) places at the outside of the plot

# x: Time
set xlabel "Time"
set xtics nomirror (2500000000.0, 5000000000.0)
set format x "%.1tx10^{%T} ns"

# y: GPU Power
set ylabel "GPU Power"
set ytics nomirror
set autoscale y
set format y "%.0f mW"

plot "/home/nvidia/caffe/cifar10_gpu_1300500000Hz.txt" using 2:4 title "Power_{GPU} 1.3 GHz", \
     "/home/nvidia/caffe/cifar10_gpu_1211250000Hz.txt" using 2:4 title "Power_{GPU} 1.2 GHz", \
     "/home/nvidia/caffe/cifar10_gpu_1122000000Hz.txt" using 2:4 title "Power_{GPU} 1.1 GHz", \
     "/home/nvidia/caffe/cifar10_gpu_1032750000Hz.txt" using 2:4 title "Power_{GPU} 1.0 GHz", \
     "/home/nvidia/caffe/cifar10_gpu_943500000Hz.txt" using 2:4 title "Power_{GPU} 943 MHz", \
     "/home/nvidia/caffe/cifar10_gpu_default_governor.txt" using 2:4 title "Power_{GPU} Default Governor"

