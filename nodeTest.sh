#!/bin/bash

outputFile="./results/resultsNodes30Ants-pPh.csv"
ants=30
alpha=1
beta=3
iterations=300
for nodes in `seq 2 20`;
do
	testNodes=$((nodes*5))
	running="./ants ${ants} 0.1 ${alpha} ${beta} ${iterations} ${testNodes} 10 100 -vB -1 -pPh"
	printf "running nodes: ${testNodes}"
	for repeat in `seq 1 5`;
	do
		printf "."
		$running >> ${outputFile}
	done
	printf "\n"
done