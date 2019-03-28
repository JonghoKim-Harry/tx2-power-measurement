#!/bin/bash

CAFFE_HOME=$HOME/caffe

cd $CAFFE_HOME

$CAFFE_HOME/build/tools/caffe time -phase TEST -model $CAFFE_HOME/examples/mnist/lenet_train_test.prototxt -weights $CAFFE_HOME/examples/mnist/lenet_iter_10000.caffemodel -gpu all
