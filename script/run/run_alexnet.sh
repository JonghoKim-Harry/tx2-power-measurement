#!/bin/bash
export CAFFE_ROOT=$HOME/caffe
cd $CAFFE_ROOT

$CAFFE_ROOT/build/tools/caffe test -model $CAFFE_ROOT/models/bvlc_alexnet/train_val.prototxt -weights $CAFFE_ROOT/models/bvlc_alexnet/bvlc_alexnet.caffemodel -gpu all
