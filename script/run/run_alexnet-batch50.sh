#!/bin/bash
export CAFFE_ROOT=$HOME/caffe
cd $CAFFE_ROOT

PROTO=$CAFFE_ROOT/models/bvlc_alexnet/batch50_train_val.prototxt
WEIGHTS=$CAFFE_ROOT/models/bvlc_alexnet/bvlc_alexnet.caffemodel

COMMAND="$CAFFE_ROOT/build/tools/caffe test -model $PROTO  -weights $WEIGHTS -gpu all"

echo "$0: $COMMAND"
$COMMAND
