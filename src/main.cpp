#include <stdio.h>
#include <ogdf/basic/Graph.h>
#include <ogdf/basic/graph_generators.h>
#include <ogdf/fileformats/GraphIO.h>
#include <ogdf/layered/DfsAcyclicSubgraph.h>
#include <ogdf/basic/GraphAttributes.h>
#include <ogdf/basic/geometry.h>
#include <cassert>
#include <string>
#include "NodePartition.h"

using namespace ogdf;

void from_cluster(ClusterGraph* CG, NodePartition* LVL){
    assert(CG->rootCluster()->nodes.size() == 0);
    int i = 0;
    for(auto c :CG->rootCluster()->children){
        assert(c->children.size() == 0);
        if (i >= LVL->size()) {
            LVL->newCell(); 
        }
        for(auto n : c->nodes){
            LVL->moveToCell(n,i);
        }
        i++;
    }
}
//def from_cluster(CG, LVL):
//  assert CG.rootCluster().nodes.empty()
// for i, c in enumerate(CG.rootCluster().children):
//    assert c.children.empty()
//   if i >= LVL.size():
//      LVL.newCell()
//     for n in c.nodes:
//    LVL.moveToCell(n, i)
//
void drawLevelGraph(ogdf::GraphAttributes* GA, std::vector<std::vector<ogdf::NodeElement*>> emb, int scaleX=50, int scaleY=50 ){
    size_t maxlvl = 0; 
    for(const auto& level: emb) {
        if(level.size() > maxlvl){
            maxlvl = level.size(); 
        } 
    }
    size_t y = 0, x = 0;

    for(const auto& level: emb){
        float offs = (maxlvl - level.size())* scaleX / 2;  
        x = 0;
        for(const auto& node: level){
            GA->x(node) =  x * scaleX + offs;
            GA->y(node) =  y * scaleY;
            x++;
        }
        y++;
    }

}

void compute2SATClasses(std::vector<std::vector<NodeElement*>>emb){
    // sync 
    std::vector<ogdf::edge> E; 
    for(const auto& nodes : emb){
        for(const auto& n : nodes){
            for(const auto& adj : n->adjEntries) {
                if(adj->isSource()) {
                    E.push_back(adj->theEdge());
                }
            }
        }
        // TODO declare the sync 
        ogdf::edge f, s; 
        ogdf::node a,b,c,d ; 
        for(size_t i = 0; i < E.size(); i++){
            for(size_t j = i + 1; j < E.size(); j++){
                f = E[i];  
                s = E[j];
                a = f->source(); b = s->source();
                c = f->target(); d = s->target(); 
                if(a == b || c == d){
                    // TODO fill the sync map. 
                }
            } 
        }
    }
}

int main(){
    Graph G;
    GraphAttributes GA (G, GraphAttributes::all);
    ogdf::NodePartition LVL(G); 
    //NodePartition partition(G);

    //CG = ogdf.ClusterGraph(G)
    //CGA = ogdf.ClusterGraphAttributes(CG, ogdf.ClusterGraphAttributes.all)
    //ogdf.GraphIO.read(CGA, CG, G, "counterexample.gml")
    //GA.__assign__(CGA)
    //from_cluster(CG, LVL)
    //
    ogdf::ClusterGraph CG(G); 
    ogdf::ClusterGraphAttributes CGA(CG,ClusterGraphAttributes::all);
    ogdf::GraphIO::read(CGA, CG, G, "counterexample.gml"); 
    GA = CGA;

    from_cluster(&CG, &LVL);


    std::vector<std::vector<NodeElement*>> emb = LVL.cells();
    drawLevelGraph(&GA, emb, 50, 100);
    const NodeArray<int> lvl = LVL.cellAssignment();
    for(const auto& e: G.edges){
        if(lvl[e->source()] > lvl[e->target()]){
            G.reverseEdge(e);
        }
    }
    for(const auto& n : G.nodes){
        GA.label(n) = std::to_string(n->index()); 
    }
    ogdf::GraphIO::write(GA, "output-acyclic-graph.gml", GraphIO::writeGML);
    GraphIO::write(GA, "output-acyclic-graph.svg", GraphIO::drawSVG);


    return 0;
}
