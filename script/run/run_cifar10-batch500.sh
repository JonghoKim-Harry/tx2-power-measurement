#!/bin/bash
export CAFFE_ROOT=$HOME/caffe
cd $CAFFE_ROOT

PROTO=$CAFFE_ROOT/examples/cifar10/batch500_cifar10_full_train_test.prototxt
WEIGHTS=$CAFFE_ROOT/examples/cifar10/cifar10_full_iter_70000.caffemodel.h5

COMMAND="$CAFFE_ROOT/build/tools/caffe test -model $PROTO -weights $WEIGHTS -gpu all"

echo "$0: $COMMAND"
$COMMAND
