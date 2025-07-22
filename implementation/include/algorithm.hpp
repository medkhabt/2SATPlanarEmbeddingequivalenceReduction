#pragma once
#include <ogdf/basic/Graph.h>
#include <ogdf/basic/simple_graph_alg.h>
#include "type.hpp"
class Contribution{
    static std::pair<std::map<int, std::set<int>>, std::map<int, int>> connetectedCompsVerticesMap(const ogdf::NodeArray<int>& connectedcomps, const ogdf::Graph& G, const std::vector<ogdf::NodeElement*>& level);
    public : 
        static void addAdjacentEdgesRestrition(const std::vector<ogdf::NodeElement*>& level, equivalentClasses& eq, const ogdf::node& v, const std::vector<int>& adjOut, const std::vector<int>& adjIn, std::map<int,int>& orderOut, std::map<int, int>& orderIn);
        static void addWeakHananiTutteSpecialCase(const std::vector<ogdf::NodeElement*>& level, const std::vector<ogdf::NodeElement*>& previousLevel, equivalentClasses& eq, const ogdf::node& v, ogdf::Graph& G,  const std::vector<int>& adjIn, std::map<int, ogdf::node>& gVertices, ogdf::NodeArray<int>& verticeLevel, int levelIndex, const std::map<int,int>& orderIn);
        static void reduceEquivalentClasses(std::vector<std::vector<ogdf::NodeElement*>>& emb, equivalentClasses& eq);
};
