#!/bin/sh
echo "filename  inside the script file $1"
cd ../Core/build && ./2SATEquivalenceReduction -f ../../API/$1 -a gws -g
pwd
cp graphs/inputs/svg/${1%.*}* ../../API/
rm graphs/inputs/svg/${1%.*}*

