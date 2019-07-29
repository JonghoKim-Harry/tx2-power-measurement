#!/bin/bash
export CAFFE_ROOT=$HOME/caffe
cd $CAFFE_ROOT

NETWORK_PATH=$CAFFE_ROOT/models/bvlc_googlenet
PROTO=$NETWORK_PATH/batch50_train_val.prototxt
WEIGHTS=$NETWORK_PATH/bvlc_googlenet.caffemodel

COMMAND="$CAFFE_ROOT/build/tools/caffe test -model $PROTO  -weights $WEIGHTS -gpu all"

echo "$0: $COMMAND"
$COMMAND
