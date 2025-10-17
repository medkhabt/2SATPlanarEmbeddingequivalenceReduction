#! /bin/bash

# pick a number of vertices , divide the number by 10 ? that's l, then start decrementing till you reach l = 10

n_vertices=$1
max_levels=$(( $1/10 ))
for (( l = max_levels; l > 10; l=l-10 )); 
do 
    echo "Generating the graph (v=${n_vertices},l=${l}"
    ./2SATEquivalenceReduction -r -n ${n_vertices} -l ${l} ; 
done; 
