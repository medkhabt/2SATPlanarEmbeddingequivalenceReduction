#include "algorithmSimplev2.hpp"
#include "GraphBuilder.h"
#include "utils.hpp"
#include "type.hpp"
void ContributionSimpleGreedyWithSortApproach::verticesWithNeighborsFromUpperLevelFirst(const GraphBuilder& builder, std::vector<ogdf::node>& vertices){
    size_t size = vertices.size();
    size_t splitter = 0; 
    for(size_t i = 0 ; i < size ; i++){
        if(vertices[i]->indeg()) {
            if(i != splitter){
                std::swap(vertices[splitter], vertices[i]);
            }
            splitter++;
        } 
    } 
}
void ContributionSimpleGreedyWithSortApproach::enforceTransitivity(GraphBuilder& builder, equivalenceClasses& eq){
    auto& emb = builder.emb;    
    int nodesSize = builder.G.numberOfNodes();
    ogdf::Graph G; 
    ogdf::NodeArray<int> vertexlevel(G);
    std::vector<int> adjIn, adjOut;
    //TODO started caring less about the structure, this needs refactoring
    std::map<int, ogdf::node> gVertices; 
    // the instantiation doesn't change much here, I just don't want it to be null.
    std::vector<ogdf::node> previousLevel = emb[0];
    int levelIndex = 0;

    int counter = 0;
    std::list<int> visited;  
    int parentEquivalenceSet = -1;  
    int parentEquivalenceKey = -1; 
    bool reverse = false; 
    int uparent = -1; 
    int vparent = -1;
    int levelparent = -1; 
    int l = 0;
    for(auto& level: emb){
        visited.clear();
        // Reorder the level vertices
        verticesWithNeighborsFromUpperLevelFirst(builder, level);
        for(const auto& v: level) {
            reverse = false;
            int w = v->index();
            if(visited.empty()) {
                visited.push_back(w); 
                // Didn't set the parent equivalence class yet.
                // this stays fixed for the entire graph
            } else if(parentEquivalenceSet == -1){
                int u = visited.front();  
                uparent = u ;
                vparent = w ;
                parentEquivalenceKey = u * nodesSize + w;
                levelparent = l;
                parentEquivalenceSet = eq.disjointSets.getRepresentative(eq.pairId[l][localIndex(eq, uparent, vparent, l)]); 
                visited.push_back(w);
            } else {
                for(std::list<int>::iterator it = visited.begin(); it != visited.end() ; it++ ){
                    int u = *it;
                    if(eqId(w,u,nodesSize, eq, l) == parentEquivalenceSet){
                        reverse = true; 
                        // insert w before u in visited.
                        visited.insert(it, w); 
                    }
                    if(eqId(w,u,nodesSize, eq, l)!= parentEquivalenceSet && eqId(u,w,nodesSize,eq,l) != parentEquivalenceSet){
                        if(reverse) {
                            mergeTwoEqs(w,u, uparent, vparent, nodesSize, eq, l, levelparent);
                            parentEquivalenceSet = eq.disjointSets.getRepresentative(eq.pairId[levelparent][localIndex(eq,uparent, vparent, levelparent)]); 
                        } else {
                            mergeTwoEqs(u,w, uparent, vparent, nodesSize, eq, l, levelparent);
                            parentEquivalenceSet = eq.disjointSets.getRepresentative(eq.pairId[levelparent][localIndex(eq,uparent, vparent, levelparent)]); 
                            /*
                               for(auto [key,value] : eq.pairId ){
                               std::cout << "******** AFTER merge: **********************" << std::endl; 
                               std::cout << "(" << key <<")u = " << key/nodesSize  << ", v = " <<  key%nodesSize << std::endl;
                               }
                               */
                        }
                    }
                } 
                if(!reverse){
                    visited.push_back(w); 
                }
            }
        }
        l++;
    }
}
