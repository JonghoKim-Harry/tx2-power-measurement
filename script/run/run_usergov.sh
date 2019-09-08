#!/bin/bash

help() {

    echo "Usage: $0 [gov name]"
}

if [ "$#" -lt 1 ]; then
    help
    exit
fi

echo "$0: \$1=$1"

GOV_NAME=$1
CNN_LIST=( "lenet-batch100" "cifar10-batch100" "alexnet-batch50" "vggnet16-batch25" "vggnet19-batch25" "googlenet-batch50" "resnet50-batch25" "resnet101-batch10" "resnet152-batch10" )

for CNN_NAME in "${CNN_LIST[@]}"; do
    sudo ./power_measurement -g $GOV_NAME -f exp_result/$GOV_NAME/$CNN_NAME-$GOV_NAME.txt ./script/run/run_$CNN_NAME.sh
done
