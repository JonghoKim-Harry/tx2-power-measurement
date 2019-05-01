#!/bin/bash
export CAFFE_ROOT=$HOME/caffe

#IMAGE_FILE=$CAFFE_ROOT/data/mnist/t10k-images-idx3-ubyte
#LABEL_FILE=$CAFFE_ROOT/data/mnist/t10k-labels-idx1-ubyte
#OUTPUT_FILE=$(dirname $0)/mnist_test_reorder26500125_lmdb
IMAGE_FILE=$1
LABEL_FILE=$2
OUTPUT_FILE=$3

$CAFFE_ROOT/build/examples/mnist/convert_mnist_data.bin \
    $IMAGE_FILE $LABEL_FILE $OUTPUT_FILE --backend=lmdb
