#!/bin/bash

#
# $1: Target directory
# $2: Target gnuplot script
# $3: Output file suffix
# $4: Benchmark name (for tagging)
#

TARGET_DIR=$1
TARGET_SCRIPT=$2
OUTPUT_SUFFIX=$3
BENCHMARK_NAME=$4

for FILE in $(ls $TARGET_DIR/*.txt); do

    INPUT_DATA_FILE=$(realpath $FILE)
    OUTPUT_PLOT_FILE=$(dirname $INPUT_DATA_FILE)/$(echo $INPUT_DATA_FILE | grep -Eo "[0-9]+Hz|[^_]*_governor")_"$BENCHMARK_NAME"_"$OUTPUT_SUFFIX".png
    echo "Plotting to: $OUTPUT_PLOT_FILE"
    echo $INPUT_DATA_FILE $OUTPUT_PLOT_FILE $BENCHMARK_NAME | xargs gnuplot -c $TARGET_SCRIPT
done
