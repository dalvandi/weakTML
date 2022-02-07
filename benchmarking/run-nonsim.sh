#!/bin/bash
n=$3
stamp=$2
name=$1
#"../../../STAMP"
echo "Running genome"
bash benchmark.sh $stamp genome "-g16384 -s64 -n16777216" $n 5 > output/$name-genome.txt
echo "Running intruder"
bash benchmark.sh $stamp intruder "-a10 -l128 -n262144 -s1" $n 5 > output/$name-intruder.txt
echo "Running labyrinth"
bash benchmark.sh $stamp labyrinth "-i $stamp/labyrinth/inputs/random-x512-y512-z7-n512.txt" $n 3 > output/$name-labyrinth.txt
echo "Running ssca2"
bash benchmark.sh $stamp ssca2 "-s20 -i1.0 -u1.0 -l3 -p3" $n 32 > output/$name-ssca2.txt
echo "Running vacation (low)"
bash benchmark.sh $stamp vacation "-n2 -q90 -u98 -r1048576 -t4194304" $n 11 > output/$name-vacation-low.txt
echo "Running vacation (high)"
bash benchmark.sh $stamp vacation "-n4 -q60 -u90 -r1048576 -t4194304" $n 11 > output/$name-vacation-high.txt
echo "Running yada"
bash benchmark.sh $stamp yada "-a15 -i $stamp/yada/inputs/ttimeu1000000.2" $n 5 > output/$name-yada.txt
