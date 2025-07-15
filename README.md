# Reduction of the equivalent classes of 2SAT formula of level planar embedding
## Build the Project
```
    mkdir build; 
    cmake -B build; 
```
## Compile the project 
```
    cmake --build .
```
## Run the executable 
```
./2SATEquivalenceReduction [inputGraph: gml file]
```
If no file is specified, it will run `graphs/inputs/gml/counterexample.gml`

The executable generates three additional files 

- `graphs/inputs/svg/input_graph.svg` a drawing of the `inputGraph` .
- `graphs/outputs/gml/relation_assignement[0-9]+.gml` graphs of all possible relative orders between vertices from the `inputGraph`.
- `graphs/outputs/svg/relation_assignement[0-9]+.svg` all drawing of the relation graph. 

## Contribution 
The main contribution of the thesis is in the `Contribution` class 
