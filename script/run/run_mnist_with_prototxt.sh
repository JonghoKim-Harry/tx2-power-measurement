#!/bin/bash

export CAFFE_ROOT=/home/nvidia/caffe
cd $CAFFE_ROOT
PROTOTXT=$1

$CAFFE_ROOT/build/tools/caffe test -model $PROTOTXT -weights $CAFFE_ROOT/examples/mnist/lenet_iter_10000.caffemodel -gpu all
