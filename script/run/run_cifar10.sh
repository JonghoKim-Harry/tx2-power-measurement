#!/bin/bash
export CAFFE_ROOT=$HOME/caffe
cd $CAFFE_ROOT

$CAFFE_ROOT/build/tools/caffe test -model $CAFFE_ROOT/examples/cifar10/cifar10_full_train_test.prototxt -weights $CAFFE_ROOT/examples/cifar10/cifar10_full_iter_70000.caffemodel.h5 -gpu all
