#!/bin/bash

#
# This script iterate all possible frequencies as fixed frequency,
# then run a default governor
#
# $1: Benchmark Name
# $2~Last Argument: Shell Command
#

help() {

    echo -e "Usage for: $(basename $0)"
    echo -e "\$1: Benchmark Name"
    echo -e "\$2 ~ Last Argument: Caffe Command"
    echo -e "Note that you should run this script with root privilege"
}

CUSTOM_USER="nvidia:nvidia"

BENCHMARK_NAME=$1
SHELL_COMMAND="${@:2}"

MIN="140250000"
MAX="1122000000"
DEFAULT_GPU_GOVERNOR="nvhost_podgov"
AVAILABLE_GPU_FREQ_LIST=$(cat /sys/devices/17000000.gp10b/devfreq/17000000.gp10b/available_frequencies)

POWER_MEASUREMENT_HOME=$(realpath $(dirname $0)/../..)
POWER_MEASUREMENT_TOOL="$POWER_MEASUREMENT_HOME/power_measurement"
RESULT_DIR="$POWER_MEASUREMENT_HOME/exp_result/$BENCHMARK_NAME"

if [ ! -d "$RESULT_DIR" ]; then
    mkdir -p $RESULT_DIR

    # Change owner
    sudo chown -R $CUSTOM_USER $RESULT_DIR
fi

if [ "$#" -lt 2 ]; then
    help
    exit
fi

#
# You should run this script with sudoer privilege
#
if [ "$EUID" -ne 0 ]; then
    echo "Please run as root privilege"
    exit
fi


#
# Note that frequency is increasing, thus you should set up MAX frequency first
#

sudo echo "$MAX" > /sys/devices/17000000.gp10b/devfreq/17000000.gp10b/max_freq
sudo echo "$MIN" > /sys/devices/17000000.gp10b/devfreq/17000000.gp10b/min_freq

for AVAILABLE_FREQ in $AVAILABLE_GPU_FREQ_LIST
do
    sudo echo "$AVAILABLE_FREQ" > /sys/devices/17000000.gp10b/devfreq/17000000.gp10b/max_freq
    sudo echo "$AVAILABLE_FREQ" > /sys/devices/17000000.gp10b/devfreq/17000000.gp10b/min_freq
    echo "GPU frequency is set to $AVAILABLE_FREQ Hz"

    FREQ_IN_MHZ=$(echo $AVAILABLE_FREQ | sed -r 's/([[:digit:]]+)[[:digit:]]{6}/\1/g')
    STAT_FILE="$RESULT_DIR/$BENCHMARK_NAME"_"$FREQ_IN_MHZ"MHz.txt

    $POWER_MEASUREMENT_TOOL -c gpu -f $STAT_FILE $SHELL_COMMAND
    cd $POWER_MEASUREMENT_HOME
done;

echo END

#
# Run with default governor
#
sudo echo $MIN > /sys/devices/17000000.gp10b/devfreq/17000000.gp10b/min_freq
sudo echo $MAX > /sys/devices/17000000.gp10b/devfreq/17000000.gp10b/max_freq
sudo echo $DEFAULT_GPU_GOVERNOR > /sys/devices/17000000.gp10b/devfreq/17000000.gp10b/governor
echo "GPU frequency is recovered to default setting"

STAT_FILE="$RESULT_DIR/$BENCHMARK_NAME"_gpu_default_governor.txt

$POWER_MEASUREMENT_TOOL -c gpu -f $STAT_FILE $SHELL_COMMAND
