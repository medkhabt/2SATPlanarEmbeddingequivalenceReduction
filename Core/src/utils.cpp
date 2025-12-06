#include <ogdf/basic/GraphAttributes.h>
#include <ogdf/basic/geometry.h>
#include <ogdf/basic/simple_graph_alg.h>
#include <ogdf/fileformats/GraphIO.h>
#include <string>
#include "type.hpp"
#include "utils.hpp"

#ifdef BUILD_PROFILING 
#include <tracy/Tracy.hpp>
#endif

#include "GraphBuilder.h"
#include "2SatCompute.hpp"
#include <random> 
#include <set>

void createLayout(std::string nameFile, ogdf::Graph& G){
    ogdf::GraphAttributes GA(G,
            ogdf::GraphAttributes::all );
    ogdf::NodeArray<int> mappings(G); 
    ogdf::Array<ogdf::node> nodes; 
    G.allNodes(nodes);
    int n_connectedcomps = ogdf::connectedComponents(G, mappings);
    std::map<int, int>counterx; 
    std::map<int, int>countery; 
    for(int i = 0 ; i < n_connectedcomps; i++){
        counterx[i] = 0; 
        countery[i] = 0; 
    }
    for(const auto& node : nodes){
        if(counterx[mappings[node]] > 200 ){
            counterx[mappings[node]] = 0 ;
        }
        GA.x(node) = mappings[node] * 600 + ((counterx[mappings[node]]% 2) ? -1 : 1) *  countery[mappings[node]] * 0.25  * 50 * counterx[mappings[node]] ;    
        GA.label(node)= std::to_string(node->index());
        counterx[mappings[node]]++;
        GA.y(node) = 50 * countery[mappings[node]]; 
        countery[mappings[node]]++;
    }
    //ogdf::GraphIO::write(GA, "graphs/outputs/gml/" + nameFile + ".gml", ogdf::GraphIO::writeGML);
    //TODO better way of logging this.
    //std::cout << " >> Generated 'graph/outputs/gml/ " << nameFile << ".gml' graph which represent the relative order between vertices of input Graph" << std::endl;
    //ogdf::GraphIO::write(GA, "graphs/outputs/svg/" + nameFile + ".svg", ogdf::GraphIO::drawSVG);
    //std::cout << " >>  Generated 'graph/outputs/svg/'" << nameFile << ".svg' drawing of the graph 'graph/outputs/gml/relation.gml'  " << std::endl; 
    //std::cout << std::endl;

}

std::string to_string(equivalenceClassesAssignement& assignement, int n){
    std::string stringRepr;
    std::stringstream ss; 
    for(auto& [eqId, value]: assignement){
        stringRepr += "(" + std::to_string(eqId / n) + "," + std::to_string(eqId % n) + ") = " + std::to_string(value) + "\n";
    }
    return stringRepr;
}
// make the arg const if the function getNumberOfSets is marked as const in the future.
bool testEmbedding(GraphBuilder& builder, equivalenceClasses& eq, std::string title, bool canGenerate, int** ordering,  bool isTotalOrder){
    equivalenceClassesAssignement eqas; 
    boost::container::flat_map<int, int> roots_values;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dist(0, 1);
    if(!isTotalOrder){
        int nodesSize = builder.G->numberOfNodes();
        for(size_t key = 0 ; key < eq.pairIdSize ; key++){
            for(size_t i = 0 ; i < eq.pairIdArraySize[key]; i++){
                int value = eq.pairId[key][i];
                if(value > -1 ){
                    int root = eq.disjointSets.getRepresentative(value); 
                    if(roots_values.find(root) == roots_values.end()){
                        roots_values[root] = dist(gen);
                        auto [u,v] = localIndexInverse(eq, i, key);
                        int reversePairSetId = eq.pairId[key][localIndex(eq,v, u, key)];
                        roots_values[eq.disjointSets.getRepresentative(reversePairSetId)] = 1 - roots_values[root];
                    }

                }
            }
        }
        boost::container::flat_map<int, ogdf::node>nodes; 
        int** nums = (int**) calloc(eq.pairIdSize, sizeof(int*)); 
        //TODO change here.
        for(size_t key = 0 ; key < eq.pairIdSize ; key++){            
            ogdf::Graph G; 
            ogdf::NodeArray<int> num(G);
            for(size_t i = 0 ; i < eq.pairIdArraySize[key]; i++){
                auto [u,v] = localIndexInverse(eq, i, key);
                if(u < v && eq.pairId[key][i] != -1){
                    if(nodes.find(u) == nodes.end()){
                        nodes[u] = G.newNode(u); 
                        //GA.label(nodes[u]) = std::to_string(u);
                    }
                    if(nodes.find(v) == nodes.end()){
                        nodes[v] = G.newNode(v); 
                        //GA.label(nodes[v]) = std::to_string(v);
                    }
                    if(roots_values[eqId(u,v,nodesSize, eq, key)]){
                        //std::cout << "edge created between : "  << u << " and " << v << std::endl;
                        G.newEdge(nodes[u], nodes[v]);
                    }else{
                        //std::cout << "edge created between : "  << v << " and " << u << std::endl;
                        G.newEdge(nodes[v], nodes[u]);
                    }
                }
            }
            ogdf::List<ogdf::edge> backedges; 
            bool acyclic = ogdf::isAcyclic(G, backedges);
            if(!acyclic){
                std::cout << "FAILED !" << std::endl;
                std::cout << "Backedges: with size : " << backedges.size() << std::endl;
                while(backedges.size()){
                    ogdf::edge e = backedges.popBackRet(); 
                    std::cout << "source : " << e->source()->index() << ", target : " << e->target()->index() << std::endl; 
                }
                delete nums;
                return false ;
            }
            ogdf::topologicalNumbering(G, num);
            nums[key] = (int*) calloc(G.maxNodeIndex() + 1, sizeof(int));
            for(int t = 0 ; t < G.maxNodeIndex() + 1; t++){
                nums[key][t] = -1; 
            }
            for(const auto& vertex: G.nodes){
                nums[key][vertex->index()] = num[vertex];
            }
        }

        builder.drawLevelGraph(nums, 50, 50);
        for(size_t key = 0; key < eq.pairIdSize ; key++){
            free(nums[key]) ;
        }
        free(nums);
        //ogdf::GraphIO::write(GA, "../graphs/inputs/svg/test.svg", ogdf::GraphIO::drawSVG);

    } else {
        builder.drawLevelGraph(ordering, 50, 50);
        for(size_t key = 0; key < eq.pairIdSize ; key++){
            free(ordering[key]) ;
        }
        free(ordering);
    }
    if(canGenerate){
        ogdf::GraphIO::write(*builder.GA, "graphs/inputs/svg/" + title +"_fixed.svg", ogdf::GraphIO::drawSVG);
    }
    std::cout << "PASSED ! " << std::endl; 
    return true;

}
