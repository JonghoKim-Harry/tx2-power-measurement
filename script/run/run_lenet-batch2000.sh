#!/bin/bash

export CAFFE_ROOT=$HOME/caffe
cd $CAFFE_ROOT

PROTO=$CAFFE_ROOT/examples/mnist/batch2000_lenet_train_test.prototxt
WEIGHTS=$CAFFE_ROOT/examples/mnist/lenet_iter_10000.caffemodel

COMMAND="$CAFFE_ROOT/build/tools/caffe test -model $PROTO -weights $WEIGHTS -gpu all"

echo "$0: $COMMAND"
$COMMAND
