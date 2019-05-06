#!/bin/bash

#
#  $1: Interval, integer in millisecond
#  $2: tegrastats logfile name
#
#  $3~Last Argument: A shell command
#
INTERVAL=$1
TEGRALOG_FILENAME=$2
SHELL_COMMAND="${@:3}"

#
# You should run this script with sudoer privilege
#
if [ "$EUID" -ne 0 ]; then
    echo "$0: Please run as root privilege"
    exit
fi

#
echo "$0: Measurement Interval will be $INTERVAL ms, as you gave"

# START tegrastats as a background process
sudo ./tegrastats --interval $INTERVAL --logfile $TEGRALOG_FILENAME &

# Sleep to cooldown CPUs
#echo "$0: Sleeping 2 seconds in order to cooldown CPUs"
#sleep 2s

$SHELL_COMMAND

# Sleep to cooldown GPUs
echo "$0: Sleeping 6 seconds in order to cooldown GPU"
sleep 6s

# STOP tegrastats. See logfile
sudo ./tegrastats --stop
