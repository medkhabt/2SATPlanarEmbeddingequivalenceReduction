# Spike in execution time 
there is spike in execution time for proper level graph with small number of levels but relatively larger number of vertices 
![3d plot](spikeLowLevel/time_plot.svg)

the spike is fixed, it was probably some deep copy that I didn't intend to do somewhere. I refactored somethings and the runetime became 'stable' in lower levels.  

# The special case of a connected components encapsulated in an other
My algorithm fails for now in the special case described in the title, ( i didn't implement it yet, wasn't sure if it was needed ), this example is a proof that it 
is needed ![example_case_encapsulation](specialEncapsulation/randomProperLevelGraph_v_13_l_4.svg)

You can also find the graph in gml format in `specialEncapsulation/randomProperLevelGraph_v_13_l_4.svg`


# Profiling to improve time execution 
I found out there are some functions that i forgot to move some args with big types by ref. 

Also I found a function call that involves an overhead after. It's after calling the reduction function, I 
still can't figure out what's the issue. ![flame](firstProfiling/flames.png) 
the source code is basically the bloc calling the function, so it's just some overhead that I can't explain yet. ![source](firstProfiling/source_code.png)

I am still in debug mode, it could be the reason behind it. I can't switch to release mode, as i get some assertion problems that I can't debug yet.


