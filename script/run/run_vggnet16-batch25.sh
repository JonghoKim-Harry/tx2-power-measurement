#!/bin/bash
export CAFFE_ROOT=$HOME/caffe
cd $CAFFE_ROOT

NETWORK_PATH=$CAFFE_ROOT/models/modelzoo_vggnet
PROTO=$NETWORK_PATH/VGG_ILSVRC_16_layers_train_val-batch25.prototxt
WEIGHTS=$NETWORK_PATH/VGG_ILSVRC_16_layers.caffemodel

COMMAND="$CAFFE_ROOT/build/tools/caffe test -model $PROTO  -weights $WEIGHTS -gpu all"

echo "$0: $COMMAND"
$COMMAND
