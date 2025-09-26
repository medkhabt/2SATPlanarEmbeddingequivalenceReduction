#pragma once
#include <ogdf/basic/DisjointSets.h>
#include <boost/container/flat_map.hpp>

struct equivalenceClasses{
    ogdf::DisjointSets<> disjointSets;
    int *pairId; 
    size_t pairIdSize = -1 ; 
    //boost::container::flat_map<int, int> pairId; 
};   
struct equivalenceClass{
    int value;
    int reverseValue; 
};
// each equivalence class (disjoing set) and its assignment.
using equivalenceClassesAssignement = boost::container::flat_map<int, int>;

