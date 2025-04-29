> [!Definition]
> Given a boolean formula $F(x_1,\ldots,x_n)$, expressed in so called "conjunctive normal form" as an $\mathrm{AND}$ of $\mathrm{OR}$s, can we "satisfy" $F$ by assigning values to its variables in such a way that $F(x_1, \ldots, x_n) = 1$ 

[[Satisfiability]] is important chiefly because Boolean algebra is so versatile. Almost any problem can be formulated i terms of basic logical operations,
and the formulation is particularly simple in a great many cases.

The story of the [[Satisfiability]] is the tale of a triumph of software engineering, blended with rich doses of beautiful mathematics. Thanks to elegant new data structures and other techniques, [[SAT]] solvers are able to deal routinely with practical problems that involve many thousands of variables, although such problems were regarded as hopeless just a few years ago.

The [[Satisfiability|satisfiability]] problem is equivalent to the [[covering problem]] 

>[!Lemma]
>a family $F$ of clauses is [[Satisfiability|satisfiable]] if and only if can be [[covering problem|covered]] by a set $L$ of [[strictly distinct]] [[Literal|literals]] 

the [[Satisfiability]] problem is the same as the problem of [[switching signs]] so that all-negative clauses remain.

Another problem equivalent to [[Satisfiability]] is obtained by going back to the [[Boolean interpretation]] and complementing both sides of the equation, thus the [[tautology]] problem and the [[Satisfiability|satisfiability]] problem are essentially the same. 

>[!Definition]
>In general, $\mathrm{kSAT}$ is the satisfiability problem restricted to instances where the no clause has more than $k$ literals. 

