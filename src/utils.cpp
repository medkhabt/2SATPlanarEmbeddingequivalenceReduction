#include <ogdf/basic/GraphAttributes.h>
#include <ogdf/basic/geometry.h>
#include <ogdf/basic/simple_graph_alg.h>
#include <ogdf/fileformats/GraphIO.h>
#include <string>
#include "type.hpp"
#include "utils.hpp"
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
    ogdf::GraphIO::write(GA, "graphs/outputs/gml/relation.gml", ogdf::GraphIO::writeGML);
    std::cout << " >> Generated 'graph/outputs/gml/relation.gml' graph which represent the relative order between vertices of input Graph" << std::endl;
    ogdf::GraphIO::write(GA, "graphs/outputs/svg/relation.svg", ogdf::GraphIO::drawSVG);
    std::cout << " >>  Generated 'graph/outputs/svg/relation.svg' drawing of the graph 'graph/outputs/gml/relation.gml'  " << std::endl; 
    std::cout << std::endl;

}

bool planarityCheck(equivalentClassesAssignement eqAs, equivalentClasses eq){
    for(auto& [key, equivalentset] : eq){
        for(auto& pair : *equivalentset){
            if(eqAs[key]!= eqAs[pair]){
                std::cout << "conflit between " << key.first << "," << key.second << " and " << pair.first << "," << pair.second << std::endl;
                return false ;
            }
        }
    }
    return true;
}
bool AcyclicRelation(equivalentClassesAssignement assignement){
    std::map<int, ogdf::node> nodes;
    ogdf::Graph G; 
    ogdf::GraphAttributes GA(G, ogdf::GraphAttributes::all);
    for(const auto& [pair, relation]: assignement){
        int u = pair.first; 
        int v = pair.second; 
        if(u < v){
            if(nodes.find(u) == nodes.end()){
                nodes[u] = G.newNode(u); 
            }
            if(nodes.find(v) == nodes.end()){
                nodes[v] = G.newNode(v); 
            }
            if(relation){
                G.newEdge(nodes[u], nodes[v]);
            }else{
                G.newEdge(nodes[v], nodes[u]);
            }
        }
    }

    createLayout("test", G);
    return ogdf::isAcyclic(G);
}

void print_eq(const equivalentClasses& eq){
    for(const auto& [pair, pset] : eq){
        std::cout << "[(" << pair.first << "," << pair.second << ")] = " ;   
        for(const auto& eqPair : *pset){
            std::cout << " (" << eqPair.first << "," << eqPair.second << ")";  
        }
        std::cout << std::endl;
    }
}
equivalentClassesAssignement fillEquivalentClasses(const equivalentClasses& eq){
    equivalentClassesAssignement eqAs; 
    // initiate the state of each order assignement to undertermined (-1) 
    for(const auto& [key, value] : eq){
        eqAs[key] = -1; 
    }

    /* TODO check all the possible drawings from the reduced eq classse.
       std::map<nodePair, int> equivalentClassesClusters; 
       int counter = 0;
       for(auto& [key, value] : eq){
       if(equivalentClassesClusters.find(key) == equivalentClassesClusters.end())
       equivalentClassesClusters[key] = counter++;
       for(auto& [u,w]: *value){
       equivalentClassesClusters[std::pair(u,w)] = equivalentClassesClusters[key]; 
       }
       }
       */

    for(auto& [key, value] : eq){
        if(eqAs[key] == -1){
            const auto u = key.first; 
            const auto w = key.second; 
            std::pair key_inverse(w,u);
            eqAs[key] = 1; 
            eqAs[key_inverse]= 0;
            for(auto& [u,w]: *value){
                std::pair<int,int> pair(u,w);
                std::pair<int,int> pairInverse(w,u);
                eqAs[pair] = 1; 
                eqAs[pairInverse] = 0; 
            }
        }
    }

    return eqAs; 

}
