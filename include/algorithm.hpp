#pragma once
#include <ogdf/basic/Graph.h>
#include <ogdf/basic/simple_graph_alg.h>
#include "type.hpp"
class Contribution{
    static std::pair<std::map<int, std::set<int>>, std::map<int, int>> connetectedCompsVerticesMap(const ogdf::NodeArray<int>& connectedcomps, const ogdf::Graph& G, const std::vector<ogdf::NodeElement*>& level);
    public : 
        static /*equivalentClasses*/ std::vector<int> addAdjacentEdgesRestrition(const std::vector<ogdf::NodeElement*>& level, const equivalentClasses& eqOrg, equivalentClasses& eq, const ogdf::node& v, const std::vector<int>& adjOut, const std::vector<int>& adjIn );
        static void addWeakHananiTutteSpecialCase(const std::vector<ogdf::NodeElement*>& level, const std::vector<ogdf::NodeElement*>& previousLevel, const equivalentClasses& eqOrg, equivalentClasses& eq, const ogdf::node& v, ogdf::Graph& G, ogdf::NodeArray<int>& connectedComps, const std::vector<int>& adjIn, std::map<int, ogdf::node>& gVertices);
        static equivalentClasses reduceEquivalentClasses(std::vector<std::vector<ogdf::NodeElement*>>& emb, const equivalentClasses& eqOrg);
};
