#!/bin/bash

export CAFFE_ROOT=/home/nvidia/caffe
cd $CAFFE_ROOT

$CAFFE_ROOT/build/tools/caffe test -model $CAFFE_ROOT/examples/mnist/lenet_train_test.prototxt -weights $CAFFE_ROOT/examples/mnist/lenet_iter_10000.caffemodel -gpu all
