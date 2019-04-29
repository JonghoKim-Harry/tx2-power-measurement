#!/bin/bash

#
# If you don't know shift command of shell, see:
# https://tiswww.case.edu/php/chet/bash/bashref.html#Bourne-Shell-Builtins
#

usage() {

    echo "Usage: $0 [options] <column index> <column name>..."
    echo ""
    echo "Options:"
    echo "   -h, --help               Print usage"
    echo "   -i, --input-file         Input file"
    echo "   -o, --output-file        Output file"
    echo "   -s, --setup-script       Setup gnuplot script"
    echo "   -t, --title              Title of the chart"
    echo "   -x, --x-axis             Index of x-axis"
}

COLUMN=()
COLUMN_IDX=
COLUMN_NAME=
SKIP=
        
for opt in "$@"; do
    case $opt in
        -h|--help)
            usage
            exit
            ;;

        -i|--input-file)
            INPUT_FILE=$2
            SKIP=YES
            shift 2
            ;;

        -o|--output-file)
            OUTPUT_FILE=$2
            SKIP=YES
            shift 2
            ;;

        -s|--setup-script)
            SETUP_SCRIPT=$2
            SKIP=YES
            shift 2
            ;;
 
        -t|--title)
            TITLE=$2
            SKIP=YES
            shift 2
            ;;

        -x|--x-axis)
            X_AXIS_IDX=$2
            SKIP=YES
            shift 2
            ;;

        -*)
            echo "Unknown Option: $opt"
            if [[ "$#" -gt 0 ]]; then shift; fi
            ;;

        *)
            if [[ -z "$SKIP" ]]; then
                COLUMN_IDX=$1
                COLUMN_NAME=$2
                COLUMN+=(":$COLUMN_IDX title \"$COLUMN_NAME\" with lines")
                shift 2
                SKIP=YES
            else
                SKIP=
            fi
            ;;
    esac
done

# You should give input file
if [[ -z "$INPUT_FILE" ]]; then
    echo "You should give us input file"
    usage
    exit
fi

# You should give output file
if [[ -z "$OUTPUT_FILE" ]]; then
    echo "You should give us output file"
    usage
    exit
fi

# Default setup gnuplot script
if [[ -z "$SETUP_SCRIPT" ]]; then
    SETUP_SCRIPT=$(dirname $0)/setup_eps.gnuplot
fi

# Default x-axis index is 2
if [[ -z "$X_AXIS_IDX" ]]; then
    X_AXIS_IDX="2"
fi

GNUPLOT_FILE=$(echo $OUTPUT_FILE | sed -r 's/(.*)\.[^.]*/\1.gnuplot/g')

cp $SETUP_SCRIPT $GNUPLOT_FILE

echo -e "set output \"$OUTPUT_FILE\"" >> $GNUPLOT_FILE
echo -e "set title \"$TITLE\"" >> $GNUPLOT_FILE

PLOT=
for COL in "${COLUMN[@]}"; do
    if [[ -z "$PLOT" ]]; then
        echo -e "plot \"$INPUT_FILE\" using $X_AXIS_IDX$COL" >> $GNUPLOT_FILE
        PLOT=YES
    else
        echo -e "replot \"$INPUT_FILE\" using $X_AXIS_IDX$COL" >> $GNUPLOT_FILE
    fi
done

echo -e "exit" >> $GNUPLOT_FILE
gnuplot -c $GNUPLOT_FILE
