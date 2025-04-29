## Multi-layer crossing minimization problem and one-layer crossing minimization problem 

![[Pasted image 20250423172734.png]]
## ILP presentation of the multi-level crossing minimization problem 

it is a starting point to form the new abstraction presented on the paper. 

![[Pasted image 20250423172940.png]]
![[Pasted image 20250423173009.png]]
It has a quadratic integer objective so it's difficult to solve with the available solvers
Juenger tried to lineare the objective function by introducing $c$ the crossing variable

![[Pasted image 20250424165537.png]][2]

## Vertex-exchange graph
We do not need to formulate nor solve an [[ILP]] in order to assign binaries to linear order variable $x_{i,j}$  

![[Pasted image 20250424173305.png]]
![[Pasted image 20250424173324.png]]
## Understanding the new constraints after changing the ILP to contain edge crossing variable 
![[Pasted image 20250425172858.png]]
![[Pasted image 20250425172927.png]]
## change the (6) and (7) to more simpler constraints 
![[Pasted image 20250425173007.png]]
![[Pasted image 20250425173026.png]]
![[Pasted image 20250425173037.png]]
## lemma closed inconsistent chain to level planar drawing 
![[Pasted image 20250425173135.png]]
## Example of the algorithm for texting the level planarity 
![[Pasted image 20250425173224.png]]
# Questions
- why avoiding only 3-cycles in the [[ILP]] representation
- why do we need to add an extra constraint between the crossing variable and linear ordering of vertices [2]
