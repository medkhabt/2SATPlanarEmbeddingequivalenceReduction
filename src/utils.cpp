#include <ogdf/basic/GraphAttributes.h>
#include <ogdf/basic/geometry.h>
#include <ogdf/basic/simple_graph_alg.h>
#include <ogdf/fileformats/GraphIO.h>
#include <string>
#include "type.hpp"
#include "utils.hpp"
<<<<<<< Updated upstream
#include "Tracy.hpp"
=======
>>>>>>> Stashed changes
#include "GraphBuilder.h"
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

//TODO Parallelize this.
/*
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
*/
//std::map<nodePair, int>;
std::string to_string(equivalenceClassesAssignement& assignement, int n){
    std::string stringRepr;
    std::stringstream ss; 
    for(auto& [eqId, value]: assignement){
        stringRepr += "(" + std::to_string(eqId / n) + "," + std::to_string(eqId % n) + ") = " + std::to_string(value) + "\n";
    }
    return stringRepr;
}
bool AcyclicRelation(std::string title, std::vector<equivalenceClassesAssignement>& assignement, GraphBuilder builder){
    ZoneScoped;
    int nodesSize = builder.G.numberOfNodes();
    for(size_t i = 0 ; i < assignement.size(); i++){
        std::map<int, ogdf::node> nodes;
        ogdf::Graph G; 
        ogdf::GraphAttributes GA(G, ogdf::GraphAttributes::all);
        for(const auto& [eqId, value]: assignement[i]){
            int u = eqId / nodesSize; 
            int v = eqId % nodesSize; 

            if(u < v){
                if(nodes.find(u) == nodes.end()){
                    nodes[u] = G.newNode(u); 
                    //GA.label(nodes[u]) = std::to_string(u);
                }
                if(nodes.find(v) == nodes.end()){
                    nodes[v] = G.newNode(v); 
                    //GA.label(nodes[v]) = std::to_string(v);
                }
                if(value){
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
            wrongAssignements << to_string(assignement[i], nodesSize);
            //std::cout << to_string(assignement[i]) << std::endl;

            wrongAssignements.close();
            return false; 
        }
    }

    return true; 
}

/*
void print_eq(const equivalentClasses& eq){
    for(const auto& [pair, pset] : eq){
        std::cout << "[(" << pair.first << "," << pair.second << ")] = " ;   
        for(const auto& eqPair : *pset){
            std::cout << " (" << eqPair.first << "," << eqPair.second << ")";  
        }
        std::cout << std::endl;
    }
}
*/
// make the arg const if the function getNumberOfSets is marked as const in the future.
bool testEmbedding(GraphBuilder& builder, equivalenceClasses& eq, std::string title){
    equivalenceClassesAssignement eqas; 
    boost::container::flat_map<int, int> roots_values;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dist(0, 1);

    if(builder.G.empty()){
        std::cout << "empty graph" << std::endl;
        return false; 
    }
    int nodesSize = builder.G.numberOfNodes();
    for(const auto& [key, value] : eq.pairId){
        int root = eq.disjointSets.getRepresentative(value); 
        if(roots_values.find(root) == roots_values.end()){
            roots_values[root] = dist(gen);
            int u = key / nodesSize ; 
            int v = key % nodesSize ;  
            int reversePairSetId = eq.pairId[v * nodesSize + u];
            roots_values[eq.disjointSets.getRepresentative(reversePairSetId)] = 1 - roots_values[root];
        }
    }

        std::map<int, ogdf::node> nodes;
        ogdf::Graph G; 
        ogdf::GraphAttributes GA(G, ogdf::GraphAttributes::all);
    
        for(const auto& [key, value]: eq.pairId){

            int u = key / nodesSize; 
            int v = key % nodesSize; 

            if(u < v){
                if(nodes.find(u) == nodes.end()){
                    nodes[u] = G.newNode(u); 
                    //GA.label(nodes[u]) = std::to_string(u);
                }
                if(nodes.find(v) == nodes.end()){
                    nodes[v] = G.newNode(v); 
                    //GA.label(nodes[v]) = std::to_string(v);
                }
                if(roots_values[eqId(u,v,nodesSize, eq)]){
                    //std::cout << "edge created between : "  << u << " and " << v << std::endl;
                    G.newEdge(nodes[u], nodes[v]);
                }else{
                    //std::cout << "edge created between : "  << v << " and " << u << std::endl;
                    G.newEdge(nodes[v], nodes[u]);
                }
            }
        }
        ogdf::NodeArray<int> num(G);
        bool acyclic = ogdf::isAcyclic(G);
        ogdf::topologicalNumbering(G, num);
        builder.drawLevelGraph(num, nodes, 50, 50);
        //ogdf::GraphIO::write(GA, "../graphs/inputs/svg/test.svg", ogdf::GraphIO::drawSVG);
        ogdf::GraphIO::write(builder.GA, "graphs/inputs/svg/" + title +"_fixed.svg", ogdf::GraphIO::drawSVG);

        for(ogdf::node v : G.nodes){
            std::cout << "v= " << v->index() << ", order="  << num[v] << std::endl;
        }
        if(!acyclic){
            std::cout << "FAILED !" << std::endl;
            return false ;
        } else {
            std::cout << "PASSED ! " << std::endl; 
            return true;
        }
}

/*
std::vector<equivalentClassesAssignement> fillEquivalentClasses(const equivalenceClasses& eq){
    equivalenceClassesAssignement eqAs; 
    // initiate the state of each order assignement to undertermined (-1) 
    ZoneScopedN("filling"); 


    // TODO check all the possible drawings from the reduced eq classse.
    //   std::map<nodePair, int> equivalentClassesClusters; 
    //   int counter = 0;
    //   for(auto& [key, value] : eq){
     //  if(equivalentClassesClusters.find(key) == equivalentClassesClusters.end())
    //   equivalentClassesClusters[key] = counter++;
    //   for(auto& [u,w]: *value){
    //   equivalentClassesClusters[std::pair(u,w)] = equivalentClassesClusters[key]; 
    //   }
    //   }
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
<<<<<<< Updated upstream

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


=======
        boost::container::flat_map<int, ogdf::node>nodes; 
        int** nums = (int**) calloc(eq.pairIdSize, sizeof(int*)); 
        //TODO change here.
        for(size_t key = 0 ; key < eq.pairIdSize ; key++){            
            ogdf::Graph G; 
            ogdf::NodeArray<int> num(G);
            //std::cout << "***** New Level " << std::endl;
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
            bool acyclic = ogdf::isAcyclic(G);
            if(!acyclic){
                std::cout << "FAILED ! on level " << key << std::endl;
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
        
        builder.drawLevelGraph(nums, 50, 100);
        for(size_t key = 0; key < eq.pairIdSize ; key++){
            free(nums[key]) ;
        }
        free(nums);
        //ogdf::GraphIO::write(GA, "../graphs/inputs/svg/test.svg", ogdf::GraphIO::drawSVG);
        if(canGenerate){
            ogdf::GraphIO::write(builder.GA, "graphs/inputs/svg/" + title +"_fixed.svg", ogdf::GraphIO::drawSVG);
        }
        std::cout << "PASSED ! " << std::endl; 
        return true;
>>>>>>> Stashed changes
}
*/
