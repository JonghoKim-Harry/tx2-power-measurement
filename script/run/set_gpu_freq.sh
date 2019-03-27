#!/bin/bash

#
# This script set GPU frequency to be fixed to given frequency value.
# 
#     $1: Frequency
#
#   * Note that given frequency is not among available frequency,
#     it will be denied with error message
#
#   * Note that if you run this script without sudo privilege,
#     it will be denied with error message
#

FIXED_GPU_FREQ=$1
AVAILABLE_GPU_FREQ_LIST=$(cat /sys/devices/17000000.gp10b/devfreq/17000000.gp10b/available_frequencies)

FOUND=0

#
# You should run this script with sudoer privilege
#
if [ "$EUID" -ne 0 ]; then
    echo "Please run as root privilege"
    exit
fi

for AVAILABLE_FREQ in $AVAILABLE_GPU_FREQ_LIST
do
    if [ "$FIXED_GPU_FREQ" == "$AVAILABLE_FREQ" ]
    then
        FOUND=1
        break
    fi
done;

if [ "$FOUND" == "0" ]; then
    echo "ERROR: You gave unavailable frequency"
else
    echo "You gave available frequency"
    sudo echo $FIXED_GPU_FREQ > /sys/devices/17000000.gp10b/devfreq/17000000.gp10b/min_freq
    sudo echo $FIXED_GPU_FREQ > /sys/devices/17000000.gp10b/devfreq/17000000.gp10b/max_freq
    echo "GPU frequency is set to $FIXED_GPU_FREQ Hz"
fi

