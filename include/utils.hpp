#pragma once
#include <ogdf/basic/Graph.h>
#include "type.hpp"
// TODO customize the naming of the creation of a layout.
void createLayout(std::string nameFile, ogdf::Graph& G);

bool planarityCheck(std::vector<equivalentClassesAssignement>& eqAs, equivalentClasses& eq);

bool AcyclicRelation(std::string title, std::vector<equivalentClassesAssignement>& assignement, std::ofstream& wrongAssignementsFile);

std::vector<equivalentClassesAssignement> fillEquivalentClasses(const equivalentClasses& eq);

void print_eq(const equivalentClasses& eq);
