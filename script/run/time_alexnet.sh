#!/bin/bash

CAFFE_HOME=$HOME/caffe

cd $CAFFE_HOME

$CAFFE_HOME/build/tools/caffe time -phase TEST -model $CAFFE_HOME/models/bvlc_alexnet/deploy.prototxt -weights $CAFFE_HOME/models/bvlc_alexnet/bvlc_alexnet.caffemodel -gpu all
