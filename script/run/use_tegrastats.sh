#!/bin/bash

#
#  $1: tegrastats logfile name
#  $2~Last Argument: A shell command 
#
TEGRALOG_FILENAME=$1
SHELL_COMMAND="${@:2}"


#
# You should run this script with sudoer privilege
#
if [ "$EUID" -ne 0 ]; then
    echo "Please run as root privilege"
    exit
fi

# START tegrastats as a background process
sudo ./tegrastats --interval 2 --logfile $TEGRALOG_FILENAME &

# Sleep to cooldown CPUs
echo "Sleeping 2 seconds in order to cooldown CPUs"
sleep 2s

$SHELL_COMMAND

# Sleep to cooldown GPUs
echo "Sleeping 6 seconds in order to cooldown GPU"
sleep 6s

# STOP tegrastats. See logfile
sudo ./tegrastats --stop
