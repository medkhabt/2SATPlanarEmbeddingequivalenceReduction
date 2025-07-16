# Spike in execution time 
there is spike in execution time for proper level graph with small number of levels but relatively larger number of vertices 
![3d plot](spikeLowLevel/time_plot.svg)

the spike is fixed, it was probably some deep copy that I didn't intend to do somewhere. I refactored somethings and the runetime became 'stable' in lower levels.  

# The special case of a connected components encapsulated in an other
My algorithm fails for now in the special case described in the title, ( i didn't implement it yet, wasn't sure if it was needed ), this example is a proof that it 
is needed ![specialEncapsulation/randomProperLevelGraph_v_4_l_2.svg]

You can also find the graph in gml format in `specialEncapsulation/randomProperLevelGraph_v_4_l_2.gml`

