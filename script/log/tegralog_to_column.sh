#!/bin/bash

#
# The purpose of this script is to generate MS Excel-friendly text file
# from the logfile of NVidia tegrastats
#

#RAM 1710/7854MB (lfb 84x4MB) CPU [0%@499,off,off,0%@499,0%@499,0%@498] EMC_FREQ 0%@665 GR3D_FREQ 0%@140 APE 150 BCPU@32C MCPU@32C GPU@32C PLL@32C Tboard@29C Tdiode@29.25C PMIC@100C thermal@32C VDD_IN 1183/1183 VDD_CPU 152/152 VDD_GPU 152/152 VDD_SOC 381/381 VDD_WIFI 0/0 VDD_DDR 192/192

#
# ARGUMENTS
#    $1: Tegralog file
#    $2: Output file
#
usage() {
    echo "SCRIPT USAGE: $0 <tegralog file> <output file>"
}

TEGRALOG_FILE=$1
OUTPUT_FILE=$2

CELSIUS=$'\xe2\x84\x83'

cpu_informations() {

    LINE=$@
    CPU_POWER=$(echo $LINE | sed -rn 's/.*VDD_CPU[[:space:]]+([[:digit:]]+).*/\1/p')
    CPU0_UTIL=$(echo $LINE | sed -rn 's/.*CPU[[:space:]]+\[([[:digit:]]+)%.*/\1/p')
    CPU0_FREQ=$(echo $LINE | sed -rn 's/.*CPU[[:space:]]+\[[[:digit:]]+%@([[:digit:]]+).*/\1/p')

    if [[ "$CPU0_UTIL" -eq "" ]]; then
        CPU0_UTIL="#N/A"
    fi
    if [[ "$CPU0_FREQ" -eq "" ]]; then
        CPU0_FREQ="#N/A"
    fi
    if [[ "$CPU_POWER" -eq "" ]]; then
        CPU_POWER="#N/A"
    fi

    echo -e "$INTENTIONAL_WHITESPACE $CPU0_UTIL   $CPU0_FREQ   $CPU_POWER"
}

gpu_informations() {

    LINE=$@
    GPU_UTIL=$(echo $LINE | sed -rn 's/.*GR3D_FREQ[[:space:]]+([[:digit:]]+)%.*/\1/p')
    GPU_FREQ=$(echo $LINE | sed -rn 's/.*GR3D_FREQ[[:space:]]+[[:digit:]]+%@([[:digit:]]+).*/\1/p')
    GPU_POWER=$(echo $LINE | sed -rn 's/.*VDD_GPU[[:space:]]+([[:digit:]]+).*/\1/p')
    GPU_TEMP=$(echo $LINE | sed -rn 's/.*GPU@([\.[:digit:]]+)C.*/\1/p')

    if [[ "$GPU_UTIL" -eq "" ]]; then
        GPU_UTIL="#N/A"
    fi
    if [[ "$GPU_FREQ" -eq "" ]]; then
        GPU_FREQ="#N/A"
    fi
    if [[ "$GPU_POWER" -eq "" ]]; then
        GPU_POWER="#N/A"
    fi
    if [[ "$GPU_TEMP" -eq "" ]]; then
        GPU_TEMP="#N/A"
    fi

    echo -e "$INTENTIONAL_WHITESPACE $GPU_UTIL   $GPU_FREQ   $GPU_POWER   $GPU_TEMP\c"
}

ddr_informations() {

    LINE=$@
    EMC_UTIL=$(echo $LINE | sed -rn 's/.*EMC_FREQ[[:space:]]+([[:digit:]]+)%.*/\1/p')
    EMC_FREQ=$(echo $LINE | sed -rn 's/.*EMC_FREQ[[:space:]]+[[:digit:]]+%@([[:digit:]]+).*/\1/p')
    DDR_POWER=$(echo $LINE | sed -rn 's/.*VDD_DDR[[:space:]]+([[:digit:]]+).*/\1/p')

    if [[ "$EMC_UTIL" -eq "" ]]; then
        EMC_UTIL="#N/A"
    fi
    if [[ "$EMC_FREQ" -eq "" ]]; then
        EMC_FREQ="#N/A"
    fi
    if [[ "$DDR_POWER" -eq "" ]]; then
        DDR_POWER="#N/A"
    fi

    echo -e "$INTENTIONAL_WHITESPACE $EMC_UTIL   $EMC_FREQ   $DDR_POWER\c"
}


# MS Excel-friendly
INTENTIONAL_WHITESPACE=" "
NULL="\"\""

if [ "$#" -lt 2 ]; then
#    echo "You gave too less arguments"
#    usage
    OUTPUT_FILE=result.txt
#    exit
fi

# For fast testing
if [ "$#" -lt 1 ]; then
    TEGRALOG_FILE=.tegralog.txt
fi


# Print header raw I
echo -e "$INTENTIONAL_WHITESPACE $NULL \"GPU Informations\" $NULL $NULL $NULL\c"
echo -e "$INTENTIONAL_WHITESPACE \"DDR Informations\" $NULL $NULL\c"
echo -e "$INTENTIONAL_WHITESPACE \"CPU Informations\" $NULL $NULL\c"
echo ""

# Print header raw II
echo -e "$INTENTIONAL_WHITESPACE Log#   GPU-util(%)   GPU-freq(MHz)   GPU-power(mW)   GPU-temp($CELSIUS)\c"
echo -e "$INTENTIONAL_WHITESPACE EMC-util(%)   EMC-freq(MHz)   DDR-power(mW)\c"
echo -e "$INTENTIONAL_WHITESPACE CPU0-util(%)   CPU0-freq(MHz)   CPU-power(mW)\c"
echo ""

# Line by line
IDX=0
exec < $TEGRALOG_FILE
while read LINE; do
    IDX=$(( $IDX + 1 ))
    echo -e "$INTENTIONAL_WHITESPACE $IDX\c"
    gpu_informations $LINE
    ddr_informations $LINE
    cpu_informations $LINE
    echo ""
done
