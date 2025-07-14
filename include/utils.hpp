#pragma once
#include <ogdf/basic/Graph.h>
#include "type.hpp"
// TODO customize the naming of the creation of a layout.
void createLayout(std::string nameFile, ogdf::Graph& G);

bool planarityCheck(equivalentClassesAssignement eqAs, equivalentClasses eq);

bool AcyclicRelation(equivalentClassesAssignement assignement);

// TODO fix the issue here of copying the equivalent class 
equivalentClassesAssignement fillEquivalentClasses(const equivalentClasses& eq);

void print_eq(const equivalentClasses& eq);
