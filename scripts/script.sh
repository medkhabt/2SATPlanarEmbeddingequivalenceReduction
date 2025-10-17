#! /bin/bash
#rm log_30

rm log_30* 2>/dev/null
rm graphs/inputs/svg/BLINDLY_RANDOM/*
rm graphs/inputs/svg/PROPER_MAXIMAL_PLANAR/*
rm graphs/inputs/svg/RANDOM_HIERACHY/*
rm graphs/inputs/gml/BLINDLY_RANDOM/*
rm graphs/inputs/gml/PROPER_MAXIMAL_PLANAR/*
rm graphs/inputs/gml/RANDOM_HIERACHY/*

for (( i = 60; i < 3200 ; i++ )); do 
    (./2SATEquivalenceReduction -g -r hiearchy -n ${i} -l 30) >> log_30_hiearchy; 
    (./2SATEquivalenceReduction -g -r maximal -n ${i} -l 30) >> log_30_maximal; 
    #(./2SATEquivalenceReduction -g -r custom -n ${i} -l 30) >> log_30_custom; 
    echo -n "hierachy: "
    cat log_30_hiearchy | grep "FAILED" | wc -l
    echo -n "maximal: "
    cat log_30_maximal | grep "FAILED" | wc -l
    done; 

