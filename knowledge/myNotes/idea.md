* adjacent edges: 
    we should synchronize the relative order of vertices that are distinct and  
are endpoints of adjacent edges and are in the same level.

    If we find a combination of edges that are adjacent, we should synchroonize 
it with the (v,v) order.

basically if we have (a,b) (a,c) (a,d), 
    sync[(a,a)].add((b,c)),
    sync[(a,a)].add((b,d)),
    sync[(a,a)].add((c,d))


