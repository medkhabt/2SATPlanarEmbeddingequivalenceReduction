#include "algorithms/algorithmSimplev1.hpp"
#include "GraphBuilder.h"
#include "utils.hpp"
#include "type.hpp"
void ContributionSimpleGreedyApproach::enforceTransitivity(GraphBuilder& builder, equivalenceClasses& eq){
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

    for(const auto& level: emb){
        visited.clear();
           for(const auto& v: level) {
               reverse = false;
               int w = v->index();
               if(visited.empty()) {
                   visited.push_back(w); 
                   // Didn't set the parent equivalence class yet.
                   // this stays fixed for the entire graph
               } else if(parentEquivalenceSet == -1){
                   int u = visited.front();  
                   parentEquivalenceKey = u * nodesSize + w;
                   parentEquivalenceSet = eq.disjointSets.getRepresentative(eq.pairId[parentEquivalenceKey]); 
                   visited.push_back(w);
               } else {
                   for(std::list<int>::iterator it = visited.begin(); it != visited.end() ; it++ ){
                       int u = *it;
                       if(eqId(w,u,nodesSize, eq) == parentEquivalenceSet){
                           reverse = true; 
                           // insert w before u in visited.
                           visited.insert(it, w); 
                       }
                       if(eqId(w,u,nodesSize, eq) != parentEquivalenceSet && eqId(u,w,nodesSize,eq) != parentEquivalenceSet){
                           if(reverse) {
                               mergeTwoEqs(w,u, parentEquivalenceKey, nodesSize, eq);
                               parentEquivalenceSet = eq.disjointSets.getRepresentative(eq.pairId[parentEquivalenceKey]); 
                           } else {
                               mergeTwoEqs(u,w, parentEquivalenceKey, nodesSize, eq);
                               parentEquivalenceSet = eq.disjointSets.getRepresentative(eq.pairId[parentEquivalenceKey]); 
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
    }
}
