#!/bin/bash

export CAFFE_ROOT=/home/nvidia/caffe

INPUT_IMAGE=$1

$CAFFE_ROOT/build/examples/cpp_classification/classification.bin $CAFFE_ROOT/models/bvlc_reference_caffenet/deploy.prototxt $CAFFE_ROOT/models/bvlc_reference_caffenet/bvlc_reference_caffenet.caffemodel $CAFFE_ROOT/data/ilsvrc12/imagenet_mean.binaryproto $CAFFE_ROOT/data/ilsvrc12/synset_words.txt $INPUT_IMAGE
