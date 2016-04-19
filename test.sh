#!/bin/bash

outputFile="results.csv"
ants=10
for alpha in `seq 1 1`;
do
	for beta in `seq 1 1`;
	do
		printf "$ants"
		running="./ants $ants 0.1 ${alpha} ${beta} 300 500 10 100 -vB -1 "
		printf "$running \n"
		printf "running alpha: ${alpha} beta: ${beta}"
		for repeat in `seq 1 5`;
		do
			printf "."
			$running >> ${outputFile}
		done
		printf "\n"
	done
done
# echo "$data"

# regex=".*Productivity\s\s(\d+\.\d+)\%.*"

# [[ $data =~ $regex ]]
# name="${BASH_REMATCH[1]}"
# echo "hihii"
# echo "$name"    # concatenate strings

