#pragma once
#include <ogdf/basic/Graph.h>
#include <ogdf/basic/simple_graph_alg.h>
#include "GraphBuilder.h"
#include "type.hpp"
class Contribution{
    static std::pair<std::map<int, std::set<int>>, std::map<int, int>> connetectedCompsVerticesMap(const ogdf::NodeArray<int>& connectedcomps, const ogdf::Graph& G, const std::vector<ogdf::NodeElement*>& level);
    public : 
        //static void addAdjacentEdgesRestrition(const std::vector<ogdf::NodeElement*>& level, equivalentClasses& eq, const ogdf::node& v, const std::vector<int>& adjOut, const std::vector<int>& adjIn, std::map<int,int>& orderOut, std::map<int, int>& orderIn);
        static void addAdjacentEdgesRestrition(equivalenceClasses& eq, const std::vector<int>& adjOut, const std::vector<int>& adjIn, int nodesSize);
        static void enforceTransitivity(GraphBuilder& builder, equivalenceClasses& eq);
};
