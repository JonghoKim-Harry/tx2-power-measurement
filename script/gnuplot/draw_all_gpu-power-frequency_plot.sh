#!/bin/bash

#
# $1: Target directory
# $2: Benchmark name (for tagging)
#

SCRIPT_PATH=$(dirname "$0")
TARGET_SCRIPT=$(realpath $SCRIPT_PATH/gpu-power-frequency.plot)

TARGET_DIR=$1
BENCHMARK_NAME=$2

for FILE in $(ls $TARGET_DIR/*.txt); do

    INPUT_DATA_FILE=$(realpath $FILE)
    OUTPUT_PLOT_FILE=$(echo $(realpath $INPUT_DATA_FILE) | sed 's/txt/png/g')
    echo $INPUT_DATA_FILE $OUTPUT_PLOT_FILE $BENCHMARK_NAME | xargs gnuplot -c $TARGET_SCRIPT
done
