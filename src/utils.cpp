#include <ogdf/basic/GraphAttributes.h>
#include <ogdf/basic/geometry.h>
#include <ogdf/basic/simple_graph_alg.h>
#include <ogdf/fileformats/GraphIO.h>
#include <string>
#include "type.hpp"
#include "utils.hpp"
#include "Tracy.hpp"
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

//TODO Parallelize this.
bool planarityCheck(std::vector<equivalentClassesAssignement>& eqAs, equivalentClasses& eq){
    ZoneScoped; 
    for(auto& [key, equivalentset] : eq){
        for(auto& pair : *equivalentset){
            for(size_t i = 0; i < eqAs.size(); i++){
                if(eqAs[i][key]!= eqAs[i][pair]){
                    std::cout << "conflit in eq assignement " << i << " between " << key.first << "," << key.second << " and " << pair.first << "," << pair.second << std::endl;
                    return false ;
                }
            }
        }
    }
    return true;
}
//std::map<nodePair, int>;
std::string to_string(equivalentClassesAssignement& assignement){
    std::string stringRepr;
    std::stringstream ss; 
    for(auto& [pair, value]: assignement){
        stringRepr += "(" + std::to_string(pair.first) + "," + std::to_string(pair.second) + ") = " + std::to_string(value) + "\n";
    }
    return stringRepr;
}
bool AcyclicRelation(std::string title, std::vector<equivalentClassesAssignement>& assignement){
    ZoneScoped;
    for(size_t i = 0 ; i < assignement.size(); i++){
        std::map<int, ogdf::node> nodes;
        ogdf::Graph G; 
        ogdf::GraphAttributes GA(G, ogdf::GraphAttributes::all);
        for(const auto& [pair, relation]: assignement[i]){
            int u = pair.first; 
            int v = pair.second; 
            if(u < v){
                if(nodes.find(u) == nodes.end()){
                    nodes[u] = G.newNode(u); 
                    //GA.label(nodes[u]) = std::to_string(u);
                }
                if(nodes.find(v) == nodes.end()){
                    nodes[v] = G.newNode(v); 
                    //GA.label(nodes[v]) = std::to_string(v);
                }
                if(relation){
                    //std::cout << "edge created between : "  << u << " and " << v << std::endl;
                    G.newEdge(nodes[u], nodes[v]);
                }else{
                    //std::cout << "edge created between : "  << v << " and " << u << std::endl;
                    G.newEdge(nodes[v], nodes[u]);
                }
            }
        }
        std::string assignementTitle = title + "_relation_assignement" + std::to_string(i);
        //createLayout(assignementTitle, G);
        //TODO also check for profiling 
        if(!ogdf::isAcyclic(G)){
        std::ofstream wrongAssignements = std::ofstream("graphs/outputs/log/wrong_assignement_" + assignementTitle); 
        if(!wrongAssignements){
            std::cerr << "Unable to open wrong assigment file" << std::endl;
        }
            std::cout << "Cyclic relation in the assignement " << i << std::endl;
            wrongAssignements << to_string(assignement[i]);
            //std::cout << to_string(assignement[i]) << std::endl;

            wrongAssignements.close();
            return false; 
        }
    }

    return true; 
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
std::vector<equivalentClassesAssignement> fillEquivalentClasses(const equivalentClasses& eq){
    equivalentClassesAssignement eqAs; 
    // initiate the state of each order assignement to undertermined (-1) 
    ZoneScopedN("filling"); 
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
    std::vector<std::pair<int,int>> combinations;
    for(auto& [key, value] : eq){
        if(eqAs[key] == -1){
            combinations.push_back(key);
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

    int size = combinations.size();
    int instantiations = pow(2,size);
    std::vector<equivalentClassesAssignement> allAssignement(instantiations);
    //all ready went through the 2^size - 1 case above
    for(int instantiation = 0 ; instantiation < instantiations; instantiation++){
        auto& pInstantiationAssignement = allAssignement[instantiation];
        for(int i = 0 ; i < size ; i++ ){
            int value = (instantiation >> i) & 1;
            pInstantiationAssignement[combinations[i]] = value;  
            pInstantiationAssignement[std::pair(combinations[i].second, combinations[i].first)] = (value + 1) & 1; 
            auto& setEqAtCombI = *(eq.at(combinations[i]));
            for(const auto& [u,w]: setEqAtCombI){
                std::pair<int,int> pair(u,w);
                std::pair<int,int> pairInverse(w,u);
                pInstantiationAssignement[std::pair(u,w)] = value; 
                pInstantiationAssignement[std::pair(w,u)] = (value + 1) & 1; 
            }
        } 
    }

    eqAs.clear();
    return allAssignement; 


}
