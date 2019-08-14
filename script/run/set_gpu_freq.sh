#!/bin/bash

#
# This script set GPU frequency to be fixed to given frequency value.
# 
#     $1: Min Frequency
#     $2: (Max Frequency)
#
#   * If maximum frequency is not given, it will be equal to minimum frequency
#
#   * Note that given frequency is not among available frequency,
#     it will be denied with error message
#
#   * Note that if you run this script without sudo privilege,
#     it will be denied with error message
#

MIN_GPU_FREQ=$1

MAX_GPU_FREQ=
if [ "$#" -gt 1 ]; then
    MAX_GPU_FREQ=$2
else
    MAX_GPU_FREQ=$MIN_GPU_FREQ
fi

AVAILABLE_GPU_FREQ_LIST=$(cat /sys/devices/17000000.gp10b/devfreq/17000000.gp10b/available_frequencies)

MIN_FREQ_FOUND=0
MAX_FREQ_FOUND=0

#
# You should run this script with sudoer privilege
#
if [ "$EUID" -ne 0 ]; then
    echo "$0: Please run as root privilege"
    exit
fi

for AVAILABLE_FREQ in $AVAILABLE_GPU_FREQ_LIST
do
    if [ "$MIN_GPU_FREQ" == "$AVAILABLE_FREQ" ]
    then
        MIN_FREQ_FOUND=1
    fi
    if [ "$MAX_GPU_FREQ" == "$AVAILABLE_FREQ" ]
    then
        MAX_FREQ_FOUND=1
    fi
done;

if [ "$MIN_FREQ_FOUND" == "0" ]; then
    echo "$0: ERROR: You gave unavailable minimum frequency"
elif [ "$MAX_FREQ_FOUND" == "0" ]; then
    echo "$0: ERROR: You gave unavailable maximum frequency"
else
    sudo echo $MIN_GPU_FREQ > /sys/devices/17000000.gp10b/devfreq/17000000.gp10b/min_freq
    echo "$0: Minimum GPU frequency is set to $MIN_GPU_FREQ Hz"
     
    sudo echo $MAX_GPU_FREQ > /sys/devices/17000000.gp10b/devfreq/17000000.gp10b/max_freq
    echo "$0: Maximum GPU frequency is set to $MAX_GPU_FREQ Hz"
fi

