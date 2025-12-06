#!/bin/sh
cd ../Core/build && ./2SATEquivalenceReduction -f ../../API/$1 -a gws -g
cp graphs/inputs/svg/${1%.*}* ../../API/outputs
cd ../../API/outputs && mv $(ls | grep -v "${1%.*}.svg") result.svg  
