#!/bin/bash
n=$3
stamp=$2
name=$1
#"../../../STAMP"
echo "Running genome"
bash benchmark.sh $stamp genome "-g256 -s16 -n16384" $n 5 > output/$name-genome.txt
echo "Running intruder"
bash benchmark.sh $stamp intruder "-a10 -l4 -n2038 -s1" $n 5 > output/$name-intruder.txt
echo "Running labyrinth"
bash benchmark.sh $stamp labyrinth "-i $stamp/labyrinth/inputs/random-x32-y32-z3-n96.txt" $n 3 > output/$name-labyrinth.txt
echo "Running ssca2"
bash benchmark.sh $stamp ssca2 "-s13 -i1.0 -u1.0 -l3 -p3" $n 32 > output/$name-ssca2.txt
echo "Running vacation (low)"
bash benchmark.sh $stamp vacation "-n4 -q60 -u90 -r16384 -t4096" $n 11 > output/$name-vacation-low.txt
echo "Running vacation (high)"
bash benchmark.sh $stamp vacation "-n4 -q60 -u90 -r16384 -t4096" $n 11 > output/$name-vacation-high.txt
echo "Running yada"
bash benchmark.sh $stamp yada "-a20 -i $stamp/yada/inputs/633.2" $n 5 > output/$name-yada.txt
