# Reduction of the equivalent classes of 2SAT formula of level planar embedding
## Build the Project

First specify the path where the ogdf library is, in my project I assume the built is in-source.

```
    mkdir build; 
    cmake -B build; 
```
## Compile the project 
```
    cd build 
    cmake --build .
```
## Run the executable 
```
Usage: ./2SATEquivalenceReduction [ [-f | --file <inputGraph>] | [-r | --random] ]
If no flags are provided, the program runs with a custom input graph file `graphs/inputs/gml/counterexample.gml`.
Options:
  -f, --file <inputGraph>   Run the program with a custom input graph file gml.
  -r, --random              Run the program with randomely generated proper level planar graphs.
```
the executable generates 
The executable generates three additional files 

- A drawing of the input graph(es) in : `graphs/inputs/svg/input_graph.svg` a drawing of the `inputGraph` .
- The relation graphs of all the assignement from the reduced equivalence class in `graphs/outputs/gml/`.
- A drawing of the relation graphs in `graphs/outputs/svg`.
- Logs of each input graph execution in `graphs/outputs/log`. 

## Contribution 
The main contribution of the thesis is in the `Contribution` class 

## Results 
Main checkpoints and progress are logged in results/README.md


## TODO
- [X] Fix a bug with the graph (v=45, l=9),  
- [ ] Switch to Release mode and see what assert is triggered.
- [X] planarity test eaiser (just test equivalence classes to each other)? , drop it of ? YES
- [X] implement the weak hanani tutte in the right level. example (29,7)
- [ ] FIX implementation of weak hanani tutte case, so it solves the graph (v=300, l=25) (explained it in results/README.md)
