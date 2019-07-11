#!/bin/bash
export CAFFE_ROOT=$HOME/caffe
cd $CAFFE_ROOT

if [[ "$#" -lt 2 ]]; then
    echo "Usage: $0 [dest_dir] [layer]"
    echo "Please give destination directory and target layer."
    echo "Note that the destination should not exist because the caffe scripts will terminate if mkdir fails"
    exit
fi;

# Note that the caffe script extract_feature.bin will use paths relative to caffe
DEST_DIR=$(realpath $1)
LAYER=$2

echo "$0: DEST_DIR: $DEST_DIR"
echo "$0: LAYER: $LAYER"

if [ -d "$DEST_DIR" ]; then
    echo "$0: Remove $DEST_DIR and its children in order to prevent mkdir fail"
    rm -rf $DEST_DIR
fi

BATCH_SIZE=50

$CAFFE_ROOT/build/tools/extract_features $CAFFE_ROOT/models/bvlc_alexnet/bvlc_alexnet.caffemodel $CAFFE_ROOT/models/bvlc_alexnet/train_val.prototxt $LAYER $DEST_DIR $BATCH_SIZE lmdb GPU all
