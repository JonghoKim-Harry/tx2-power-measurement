#!/bin/bash

#
# $1: Target directory
# $2: Output csv
#

TARGET_DIR=$1
OUTPUT_FILE=$2

# 
#     Caffelog File Format Example: CIFAR-10
#
#
# I0328 02:06:23.501402 20618 caffe.cpp:397] Average time per layer: 
# I0328 02:06:23.501420 20618 caffe.cpp:400]      cifar	forward: 0.0238304 ms.
# I0328 02:06:23.501436 20618 caffe.cpp:403]      cifar	backward: 0.00623936 ms.
# I0328 02:06:23.501453 20618 caffe.cpp:400] label_cifar_1_split	forward: 0.0230765 ms.
# I0328 02:06:23.501468 20618 caffe.cpp:403] label_cifar_1_split	backward: 0.0064256 ms.
# I0328 02:06:23.501484 20618 caffe.cpp:400]      conv1	forward: 5.55756 ms.
# I0328 02:06:23.501499 20618 caffe.cpp:403]      conv1	backward: 4.42195 ms.
# I0328 02:06:23.501515 20618 caffe.cpp:400]      pool1	forward: 1.39757 ms.
# I0328 02:06:23.501530 20618 caffe.cpp:403]      pool1	backward: 6.20635 ms.
# I0328 02:06:23.501545 20618 caffe.cpp:400]      relu1	forward: 0.229942 ms.
# I0328 02:06:23.501561 20618 caffe.cpp:403]      relu1	backward: 0.315624 ms.
# I0328 02:06:23.501576 20618 caffe.cpp:400]      norm1	forward: 41.2803 ms.
# I0328 02:06:23.501592 20618 caffe.cpp:403]      norm1	backward: 240.972 ms.
# I0328 02:06:23.501608 20618 caffe.cpp:400]      conv2	forward: 4.2422 ms.
# I0328 02:06:23.501623 20618 caffe.cpp:403]      conv2	backward: 8.76429 ms.
# I0328 02:06:23.501638 20618 caffe.cpp:400]      relu2	forward: 0.231828 ms.
# I0328 02:06:23.501654 20618 caffe.cpp:403]      relu2	backward: 0.317546 ms.
# I0328 02:06:23.501670 20618 caffe.cpp:400]      pool2	forward: 0.407078 ms.
# I0328 02:06:23.501685 20618 caffe.cpp:403]      pool2	backward: 1.34738 ms.
# I0328 02:06:23.501700 20618 caffe.cpp:400]      norm2	forward: 3.33396 ms.
# I0328 02:06:23.501715 20618 caffe.cpp:403]      norm2	backward: 14.9238 ms.
# I0328 02:06:23.501731 20618 caffe.cpp:400]      conv3	forward: 2.0027 ms.
# I0328 02:06:23.501746 20618 caffe.cpp:403]      conv3	backward: 4.00685 ms.
# I0328 02:06:23.501761 20618 caffe.cpp:400]      relu3	forward: 0.119938 ms.
# I0328 02:06:23.501776 20618 caffe.cpp:403]      relu3	backward: 0.169148 ms.
# I0328 02:06:23.501791 20618 caffe.cpp:400]      pool3	forward: 0.223323 ms.
# I0328 02:06:23.501807 20618 caffe.cpp:403]      pool3	backward: 0.7004 ms.
# I0328 02:06:23.501822 20618 caffe.cpp:400]        ip1	forward: 0.121467 ms.
# I0328 02:06:23.501837 20618 caffe.cpp:403]        ip1	backward: 0.111057 ms.
# I0328 02:06:23.501896 20618 caffe.cpp:400] ip1_ip1_0_split	forward: 0.00804544 ms.
# I0328 02:06:23.501915 20618 caffe.cpp:403] ip1_ip1_0_split	backward: 0.0242598 ms.
# I0328 02:06:23.501930 20618 caffe.cpp:400]   accuracy	forward: 0.218156 ms.
# I0328 02:06:23.501945 20618 caffe.cpp:403]   accuracy	backward: 0.00629952 ms.
# I0328 02:06:23.501960 20618 caffe.cpp:400]       loss	forward: 0.338092 ms.
# I0328 02:06:23.501976 20618 caffe.cpp:403]       loss	backward: 0.0700499 ms.
# I0328 02:06:23.502010 20618 caffe.cpp:408] Average Forward pass: 59.9877 ms.
# 

for FILE in $(ls $TARGET_DIR/*.caffelog.txt); do

    SETTING=$(echo $FILE | grep -Eo '[0-9]+Hz|[a-zA-Z]+_governor')
    LAYERS="$(grep 'forward:' $FILE | sed 's/.*][[:space:]]\+\([a-z0-9_]*\)[[:space:]]\+forward:.*/, \1/g'), ALL LAYERS"
    TIMES="$(grep 'forward:' $FILE | sed 's/.*forward:[[:space:]]\+\([0-9]\+\.[0-9]\+\)[[:space:]]\+ms.*/, \1/g'), $(grep 'Average Forward pass:' $FILE | sed 's/.*Average Forward pass: \([0-9]\+.[0-9]\+\)[[:space:]]\+ms.*/\1/g')"
    echo $SETTING $LAYERS $TIMES
done
