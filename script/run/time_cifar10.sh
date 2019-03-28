#!/bin/bash

CAFFE_HOME=$HOME/caffe

cd $CAFFE_HOME

$CAFFE_HOME/build/tools/caffe time -phase TEST -model $CAFFE_HOME/examples/cifar10/cifar10_full_train_test.prototxt -weights $CAFFE_HOME/examples/cifar10/cifar10_full_iter_70000.caffemodel.h5 -gpu all
