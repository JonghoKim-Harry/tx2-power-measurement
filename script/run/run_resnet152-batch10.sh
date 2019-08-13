#!/bin/bash
export CAFFE_ROOT=$HOME/caffe
cd $CAFFE_ROOT

NETWORK_PATH=$CAFFE_ROOT/models/modelzoo_resnet
PROTO=$NETWORK_PATH/ResNet-152-train_val.prototxt
WEIGHTS=$NETWORK_PATH/ResNet-152-model.caffemodel

COMMAND="$CAFFE_ROOT/build/tools/caffe test -model $PROTO  -weights $WEIGHTS -gpu all"

echo "$0: $COMMAND"
$COMMAND
