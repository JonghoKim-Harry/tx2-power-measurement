#!/bin/bash

#
# This script iterate all possible frequencies as fixed frequency,
# then run a default governor
#
# $1: Benchmark Name
# $2~Last Argument: Caffe Command
#

help() {

    echo -e "Usage for: $(basename $0)"
    echo -e "\$1: Benchmark Name"
    echo -e "\$2 ~ Last Argument: Caffe Command"
    echo -e "Note that you should run this script with root privilege"
}

BENCHMARK_NAME=$1
CAFFE_COMMAND="${@:2}"

MIN="140250000"
MAX="1122000000"
DEFAULT_GOVERNOR=nvhost_podgov
AVAILABLE_GPU_FREQ_LIST=$(cat /sys/devices/17000000.gp10b/devfreq/17000000.gp10b/available_frequencies)

POWER_MEASUREMENT_HOME=$(realpath $(dirname $0)/../..)
POWER_MEASUREMENT_TOOL=$POWER_MEASUREMENT_HOME/tx2_power_measurement
CAFFE_HOME=$(realpath "$HOME/caffe")
HERE=$(realpath $(pwd))
RESULT_DIR="$POWER_MEASUREMENT_HOME/test_result/$BENCHMARK_NAME"

echo "HERE = $HERE"
echo "CAFFE_COMMAND = $CAFFE_COMMAND"

if [ ! -d $RESULT_DIR ]; then
    mkdir -p $RESULT_DIR
    chown nvidia $RESULT_DIR
fi

cd $CAFFE_HOME

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

    STAT_FILE="$RESULT_DIR/$BENCHMARK_NAME"_"$AVAILABLE_FREQ"Hz.txt
    LOG_FILE="$RESULT_DIR/$BENCHMARK_NAME"_"$AVAILABLE_FREQ"Hz.caffelog

    cd $HERE
    $POWER_MEASUREMENT_TOOL -c gpu -f $STAT_FILE $CAFFE_COMMAND 2>$LOG_FILE
done;

echo END

#
# Run with default governor
#
sudo echo $MIN > /sys/devices/17000000.gp10b/devfreq/17000000.gp10b/min_freq
sudo echo $MAX > /sys/devices/17000000.gp10b/devfreq/17000000.gp10b/max_freq
sudo echo $DEFAULT_GOVERNOR > /sys/devices/17000000.gp10b/devfreq/17000000.gp10b/governor

STAT_FILE="$RESULT_DIR/$BENCHMARK_NAME"_gpu_default_governor.txt
LOG_FILE="$RESULT_DIR/$BENCHMARK_NAME"_gpu_default_governor.caffelog

cd $HERE
$POWER_MEASUREMENT_TOOL -c gpu -f $STAT_FILE $CAFFE_COMMAND 2>$LOG_FILE
