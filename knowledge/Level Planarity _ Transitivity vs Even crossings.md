
> [!IMPORTANT] Contribution
>the result of [[Randerath et al. , A satisfiability formulation of Problems on Level Graphs]] is equivalent to the strong [[Hanani-Tutte]] theorem for the [[Level Planarity]]. 
>it maps between the [[Randerath et al. , A satisfiability formulation of Problems on Level Graphs]] ($G^+$)constraint system and [[Fulek et al., Hanani-Tutte, Monotone drawings and level planarity]] ($G^*$) 

#paper 


[[critical]]
![[Pasted image 20250428102140.png]]
Any pair of [[independent]] edges in a level drawing of $G$ is a pair of [[critical]]  edges. 

## Considerations 
- Two distinct vertices are not drawn in the same single point. 
- No edge passes through a vertex. 
- no three (or more) edges cross in a single point.

# Level Planarity 
the key claim of [[Randerath et al. , A satisfiability formulation of Problems on Level Graphs]] is that $S'(G)$ is satisfiable if and only iff $S(G)$ is satisfiable, i.e., dropping the transitivity clauses does not changes the satisfiability of $S'(G)$. 

 ## Lemma 3 
 I still have issues with the lemma 3. From my understanding, I think we can $\varphi^+\left(u'',v''\right) = \psi^+\left(u'',v''\right)$ in case one of the vertices is part of $V(G^*)$ is because if we are on a 
 level of $G^*$ which is true because one of the vertices is part of the $V(G^*)$ and the two vertices are on the same level in $G^+$, so we are not afraid of the even crossing that occurs between levels of $G^*$ even though the drawing can be a level planar drawing.  

for the case of where both vertices are not part of the $G^*$ then we do this ![[Pasted image 20250429160947.png]]
 

 
 
# Notes
![[Pasted image 20250428110832.png]]![[Pasted image 20250428110840.png]]