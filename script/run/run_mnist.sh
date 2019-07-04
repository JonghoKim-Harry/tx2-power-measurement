#!/bin/bash

export CAFFE_ROOT=$HOME/caffe
cd $CAFFE_ROOT

COMMAND="$CAFFE_ROOT/build/tools/caffe test -model $CAFFE_ROOT/examples/mnist/lenet_train_test.prototxt -weights $CAFFE_ROOT/examples/mnist/lenet_iter_10000.caffemodel -gpu all"

echo "$0: $COMMAND"
$COMMAND
