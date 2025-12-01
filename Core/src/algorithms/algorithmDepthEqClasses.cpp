#include "algorithmDepthEqClasses.hpp"
#include "contribution.hpp"
#include "GraphBuilder.h"
#include "type.hpp"
#include "utils.hpp"
#include <algorithm>
#include <utility>
#include <tuple>

void ContributionSimpleGreedyWithEquivalenceClassDepth::depthAndStartOfEquivalenceClasses(
        GraphBuilder& builder,
        equivalenceClasses& eq,
        std::vector<std::vector<int>>& startOfEquivalenceClasses, 
        int* depthOfEquivalenceClasses){
    size_t numberEquivalenceClasses = eq.disjointSets.getNumberOfElements();
    for(size_t i = 0; i < numberEquivalenceClasses; i++){
        depthOfEquivalenceClasses[i] = -1;
    }
    int levelId = 0;
    std::cout << "********** START of sorting "  << std::endl;
    for(const auto& level : builder.emb ){
        startOfEquivalenceClasses.emplace_back(); 
        for(std::vector<ogdf::node>::const_iterator itx = level.begin(); itx != level.end() ; itx++){
            for(std::vector<ogdf::node>::const_iterator ity = level.begin() ; ity != level.end(); ity++) {
                if((*itx)->index() == (*ity)->index())
                    continue;
                int representatifId = eqId((*itx)->index(), (*ity)->index(), 0, eq, levelId);
                std::cout << "u : " << (*itx)->index() << " and v : " << (*ity)->index() << " with representatif : " << representatifId << std::endl;
                if(depthOfEquivalenceClasses[representatifId] == -1){
                    startOfEquivalenceClasses[levelId].push_back(representatifId);
                }
                depthOfEquivalenceClasses[representatifId]++;
            }
        }
        levelId++;
    }
    std::cout << "********** END of sorting "  << std::endl;
}
int ContributionSimpleGreedyWithEquivalenceClassDepth::enforceTransitivity(GraphBuilder& builder, equivalenceClasses& eq){
// go through levels and store the equivalence class depth and also if it is already set. 
// sort them in that level
// go through levels, whenever assignement an equivalence class test the transitivity if it fails switch
    
    std::vector<std::vector<int>> startOfEquivalenceClasses;
    size_t numberEquivalenceClasses = eq.disjointSets.getNumberOfElements();
    int eqValue[numberEquivalenceClasses];
    for(int i = 0; i < numberEquivalenceClasses; i++){
        eqValue[i] = -1;
    }
    int* depthOfEquivalenceClasses = (int*) calloc(numberEquivalenceClasses, sizeof(int));  
    depthAndStartOfEquivalenceClasses(builder, eq, startOfEquivalenceClasses, depthOfEquivalenceClasses);
    auto moreDepth = [depthOfEquivalenceClasses](int a, int b){  return depthOfEquivalenceClasses[a] > depthOfEquivalenceClasses[b]; };
    /*struct
    {
        bool operator()(int a, int b) const { return depthOfEquivalenceClasses[a] > depthOfEquivalenceClasses[b]; }
    }
    moreDepth;
    */
    size_t numberLevels = builder.emb.size();
    for(size_t l = 0 ; l < numberLevels ; l++){
        std::sort(startOfEquivalenceClasses[l].begin(), startOfEquivalenceClasses[l].end(), moreDepth);          
    }
    delete depthOfEquivalenceClasses;
    // LOGGING START OF EQUI CLASSes
    for(size_t l =0 ; l < numberLevels; l++){
        std::cout << "L: " << l << " :::   "; 
        for(int rep : startOfEquivalenceClasses[l]){
            std::cout << " " << rep << " "; 
        }
        std::cout << std::endl;
    }

    boost::container::flat_map<int, std::vector<std::tuple<int,int,int>>> pairsByEquivalenceClass; 
    int levelId = 0;
    for(const auto& level: builder.emb){
        for(std::vector<ogdf::node>::const_iterator itx = level.begin(); itx != level.end(); itx++){
            for(std::vector<ogdf::node>::const_iterator ity = level.begin() ; ity != level.end(); ity++) {
                if((*itx) == (*ity))
                    continue;
                int representatifId = eqId((*itx)->index(), (*ity)->index(), 0, eq, levelId);
                std::cout << "u,v : " << (*itx)->index()  << "," << (*ity)->index() << " id : " << representatifId << std::endl;
                pairsByEquivalenceClass[representatifId].push_back(std::tuple((*itx)->index(), (*ity)->index(), levelId));
            }
        }
        levelId++;
    }
    for(size_t i = 0 ; i < numberEquivalenceClasses; i++){
        if(!pairsByEquivalenceClass[i].empty()){
            std::cout << "equivalence class: " << i << " :::   "; 
            for(auto& [u,v,l] : pairsByEquivalenceClass[i]){
                std::cout << " (" << u << "," << v << "," << l << ")  ";
            } 
            std::cout << std::endl;
        }
    }
    // TODO I need to see how i can test from transitivity 
    int maxNodeIndex = builder.G->maxNodeIndex();
    std::vector<GraphBuilder> orderGraphs;
    ogdf::node verticesPerLevelMapping[numberLevels][maxNodeIndex+1]; 
    std::cout << "max Node Index : " << maxNodeIndex << std::endl;
    std::vector<std::pair<int,ogdf::edge>> bufferEdges; 
    for(size_t l = 0; l < numberLevels; l++){
        for(size_t vid = 0; vid < maxNodeIndex + 1; vid++){
            verticesPerLevelMapping[l][vid] = nullptr;
        } 
    } 
    for(size_t l = 0; l < numberLevels; l++)
        orderGraphs.emplace_back(false);
    for(size_t l = 0 ; l < numberLevels ; l++){
        std:: cout << "Level : " << l << std::endl;
        for(int reprId : startOfEquivalenceClasses[l]){
            std::cout << "   size of pbec in " << reprId << " is " << pairsByEquivalenceClass[reprId].size() << std::endl;
            auto [u,w,levelRep] = pairsByEquivalenceClass[reprId].front(); 
            std::cout << "id eq class :"  << reprId << "u and w : "  << u << "," << w << std::endl;
            int reverseEqId = eqId(w,u,0, eq, levelRep);
            std::cout << "id eq class :"  << reprId << "u and w : "  << u << "," << w << " and reverse eq class id : " << reverseEqId << std::endl;
            if(eqValue[reverseEqId] == 1){
                eqValue[reprId] = 0;  
                continue;
            } else if(eqValue[reverseEqId] == 0){
                eqValue[reprId] = 0;  
                continue;
            }
            bufferEdges.clear();
            eqValue[reprId] = 1;
            for(auto& [src,trg,level] : pairsByEquivalenceClass[reprId]){
                std::cout << "****************** Treating a new equivalence class : " << reprId << " ************* with src: "  << src << ", trg: " << trg << std::endl;
                if(verticesPerLevelMapping[level][src] == nullptr) {
                    verticesPerLevelMapping[level][src] = orderGraphs[level].G->newNode(src);
                    std::cout << "creating new node : "  << src << " for level " << level << "the node index is : " << verticesPerLevelMapping[level][src]->index() << std::endl;
                }
                if(verticesPerLevelMapping[level][trg] == nullptr) {
                    verticesPerLevelMapping[level][trg] = orderGraphs[level].G->newNode(trg);
                    std::cout << "creating new node : "  << trg  << " for level " << level << "the node index is : " << verticesPerLevelMapping[level][trg]->index() << std::endl;
                }
                bufferEdges.push_back(std::pair(level,orderGraphs[level].G->newEdge(verticesPerLevelMapping[level][src], verticesPerLevelMapping[level][trg])));
                std::cout << "creating new edge : (" << src << "," << trg  << ") is it the same as : " << verticesPerLevelMapping[level][src]  << ", " << verticesPerLevelMapping[level][trg] << " for level " << level << std::endl;
            }
            bool reverseOrder = false;
            for(size_t p = 0; p < numberLevels; p++){
                if(!ogdf::isAcyclic(*orderGraphs[p].G)){
                    reverseOrder = true;
                }	
            }
            if(reverseOrder){
                std::cout << "cancel the last chunck of edges "<< std::endl;
                eqValue[reprId] = 0;
                for(auto& [p,e]: bufferEdges){
                    orderGraphs[p].G->delEdge(e);
                }
                for(auto& [src,trg,level] : pairsByEquivalenceClass[reprId]){
                    std::cout << "creating new edge : (" << trg << "," << src << ") is it the same as : " << verticesPerLevelMapping[l][trg]  << ", " << verticesPerLevelMapping[l][src] << " for level " << level << std::endl;
                    std::pair(level,orderGraphs[level].G->newEdge(verticesPerLevelMapping[level][trg], verticesPerLevelMapping[level][src]));
                }
                for(size_t p = 0; p < numberLevels; p++){
                    if(!ogdf::isAcyclic(*orderGraphs[p].G)){
                        std::cerr << "Transitivity can't be respected." << std::endl;
                        return 1;
                    }	
                }
                
            }
        } 
    }
    bool firstEqClass = true;
    int parentEquivalenceSet = -1;
    int uparent, vparent; 
    int levelparent;
    for(size_t l= 0; l < numberLevels; l++){
        for(int reprId : startOfEquivalenceClasses[l]){
            if(eqValue[reprId] != -1 && firstEqClass){
                parentEquivalenceSet = reprId; 
                std::tuple t  = pairsByEquivalenceClass[reprId].front();
                uparent = std::get<0>(t);
                vparent = std::get<1>(t);
                levelparent = std::get<2>(t);

                std::cout << "parent equivalence class : " << parentEquivalenceSet << " and this pair represent it: "  <<  uparent << " , " << vparent << std::endl;
                firstEqClass = false;
                continue;
            }
            if(eqValue[reprId] == 0) {
                auto [u,w,l] = pairsByEquivalenceClass[reprId].front(); 
                std::cout << "eq : " << reprId << " pair representer : " << w << " , " << u << std::endl;
                mergeTwoEqs(w, u, uparent, vparent, 0, eq, l, levelparent);
                //parentEquivalenceSet = eq.disjointSets.getRepresentative(eq.pairId[levelparent][localIndex(eq,uparent, vparent, levelparent)]); 
            } else if(eqValue[reprId] == 1) {
                auto [u,w,l] = pairsByEquivalenceClass[reprId].front(); 
                std::cout << "eq : " << reprId << " pair representer : " <<  u << " , " << w << std::endl;
                mergeTwoEqs(u, w, uparent, vparent, 0, eq, l, levelparent);
                //parentEquivalenceSet = eq.disjointSets.getRepresentative(eq.pairId[levelparent][localIndex(eq,uparent, vparent, levelparent)]); 
                /*
                   for(auto [key,value] : eq.pairId ){
                   std::cout << "******** AFTER merge: **********************" << std::endl; 
                   std::cout << "(" << key <<")u = " << key/nodesSize  << ", v = " <<  key%nodesSize << std::endl;
                   }
                   */
            }
        } 
    }
    std::cout << "BEG**************** Graphs" << std::endl;
    for(size_t l = 0 ; l < numberLevels ; l++){
        std::cout << "Graph of level " << l << std::endl;
        std::cout << "    ";
        for(ogdf::node v : orderGraphs[l].G->nodes){
            std::cout  << " " << v->index() << " " ;
        } 
        std::cout <<  std::endl;
    }
    std::cout << "END**************** Graphs" << std::endl;
     this->finalOrdering = (int**) calloc(eq.pairIdSize, sizeof(int*)); 
    this->isTotalOrder = true;
    std::cout <<  "topological numbering on each level"  << std::endl;
    for(size_t l = 0; l < numberLevels; l++){
         this->finalOrdering[l] = (int*) calloc(builder.G->maxNodeIndex() + 1, sizeof(int));
        for(int t = 0 ; t < builder.G->maxNodeIndex() + 1; t++){
             this->finalOrdering[l][t] = -1; 
        }
        ogdf::NodeArray<int> num(*orderGraphs[l].G);
        ogdf::topologicalNumbering(*orderGraphs[l].G, num);  
        orderGraphs[l].drawLevelGraph2();
        ogdf::GraphIO::write(*orderGraphs[l].GA, "graphs/inputs/svg/" + std::to_string(l) + ".svg", ogdf::GraphIO::drawSVG);
        std::cout <<  "Level " << l << std::endl;
        for(ogdf::NodeArray<int>::iterator it = num.begin(); it != num.end(); it++){
            std::cout << " " << it.key()->index()  <<  ":" << it.value() << " ";
             this->finalOrdering[l][it.key()->index()] = it.value();
        }
        std::cout << std::endl;
    }
    return 0;
}
