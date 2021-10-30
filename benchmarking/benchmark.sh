#!/bin/bash

allThreads=(1 2 4 8 16 32)

stamp=$1
bm=$2
args=$3

if [[ "$bm" == "vacation" ]]; then
    for tn in ${allThreads[@]}; do
        for ((counter = 1; counter <= $4; counter++)); do
            i=0
            while read -r line; do
                if [[ "$i" -eq $5 ]]; then
                    echo "t=$tn," "$line"
                fi
                ((i = i + 1))
            done < <(./$stamp/$bm/$bm $args -c$tn)
        done
    done
else
    for tn in ${allThreads[@]}; do
        for ((counter = 1; counter <= $4; counter++)); do
            i=0
            while read -r line; do
                if [[ "$i" -eq $5 ]]; then
                    echo "t=$tn," "$line"
                fi
                ((i = i + 1))
            done < <(./$stamp/$bm/$bm $args -t$tn)
        done
    done
fi
