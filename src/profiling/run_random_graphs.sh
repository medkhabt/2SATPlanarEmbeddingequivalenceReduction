#!/bin/bash

if [ -z "$1" ]; then 
    levels=10 
else 
    levels=$1
fi

if [ -z "$2" ]; then 
    nodes=30 
else 
    nodes=$2

fi

../2SATEquivalenceReduction -r -n ${nodes} -l ${levels} 

gnuplot level_nodes_to_time.gp 

display time_plot.svg &
