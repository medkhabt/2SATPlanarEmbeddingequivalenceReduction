#include "algorithm.hpp"
#include "utils.hpp"
#include "Tracy.hpp"

/*equivalentClasses*/ void Contribution::addAdjacentEdgesRestrition(const std::vector<ogdf::NodeElement*>& level, equivalentClasses& eq, const ogdf::node& v, const std::vector<int>& adjOut, const std::vector<int>& adjIn, std::map<int, int>& orderOut, std::map<int,int>& orderIn){
    ZoneScopedN("addAjdacentEdges function"); 
    int counter = 0;
    for(const auto i : adjIn){
        orderIn[i] = counter++;
    }
    counter = 0;
    for(const auto i : adjOut){
        orderOut[i] = counter++;
    }
    for(const auto u:adjOut){
        ZoneScopedN("outside ajdOut function"); 
        for(const auto w:adjOut){
            if(u < w && eq.find(std::make_pair(u ,w ))!= eq.end()){
                // it means we already processed an equivalent class 
                // that has the inverse of this order.
                if(orderOut[u] > orderOut[w]){
                    continue;
                }
                for(const auto pair : *eq[std::make_pair(u,w)]){
                    auto u1 = pair.first;
                    auto w1 = pair.second;
                    if(u1 > w1 && orderOut.find(u1)!= orderOut.end() && orderOut.find(w1) != orderOut.end()){
                        if(orderOut[u1] > orderOut[w1]){
                            int temp = orderOut[u1];
                            orderOut[u1] = orderOut[w1];
                            orderOut[w1] = temp; 
                        }
                    }
                }
            } 
        }
    }
    for(const auto u:adjIn){
        ZoneScopedN("outside inadj function"); 
        for(const auto w:adjIn){
            if(u < w){
                if(eq.find(std::make_pair(u, w)) == eq.end()){
                    std::pair pair(u,w);
                    std::pair pair_inverse(w,u);
                    eq[pair] = std::make_shared<nodePairSet>(); 
                    eq[pair]->insert(pair);
                    eq[pair_inverse] = std::make_shared<nodePairSet>(); 
                    eq[pair_inverse]->insert(pair_inverse);
                }
                // it means we already processed an equivalent class 
                // that has the inverse of this order.
                if(orderIn[u] > orderIn[w]){
                    continue;
                }
                for(const auto pair : *eq[std::make_pair(u,w)]){
                    auto u1 = pair.first;
                    auto w1 = pair.second;
                    if(u1 > w1 && orderIn.find(u1)!= orderIn.end() && orderIn.find(w1) != orderIn.end()){
                        if(orderIn[u1] > orderIn[w1]){
                            int temp = orderIn[u1];
                            orderIn[u1] = orderIn[w1];
                            orderIn[w1] = temp; 
                        }
                    }
                }
            } 
        }
    }

    std::shared_ptr<nodePairSet> e = nullptr, e_inverse = nullptr; 
    nodePair paar, paar_inverse; 
    for(const auto u: adjOut ){
        ZoneScopedN("outside outdj eq sync "); 
        for(const auto  w : adjOut){
            if(u < w){
                paar = std::make_pair(u,w); 
                paar_inverse = std::make_pair(w,u);
                if(orderOut[u] > orderOut[w]){
                    std::swap(paar, paar_inverse);
                }
                // if the equivalent class doesn't exist yet due to vertex not 
                // having an edge that is important (non-adjacent critical edges) edge.
                if(eq.find(paar) == eq.end()){
                    eq[paar] = std::make_shared<nodePairSet>(); 
                    eq[paar]->insert(std::make_pair(u,w));
                    eq[paar_inverse] = std::make_shared<nodePairSet>(); 
                    eq[paar_inverse]->insert(std::make_pair(w,u));
                }

                if(e == nullptr){
                    e = eq[paar]; 
                    e_inverse = eq[paar_inverse];
                }
                if(e->find(paar) == e->end() && e->find(paar_inverse) == e->end()){

                    e->insert(eq[paar]->begin(), eq[paar]->end());
                    e_inverse->insert(eq[paar_inverse]->begin(), eq[paar_inverse]->end());

                    eq[paar] = e;
                    eq[paar_inverse] = e_inverse;
                    for(auto& [u1, w1]: *eq[paar]){
                        std::pair<int, int> eqPaar(u1,w1);
                        std::pair<int, int> eqPaar_inverse(w1,u1);
                        eq[eqPaar] = e;  
                        eq[eqPaar_inverse] = e_inverse;
                    }
                }
            }
        }
    }

    e = nullptr;
    e_inverse = nullptr;

    for(const auto u: adjIn ){
        ZoneScopedN("outside in eq sync "); 
        for(const auto  w : adjIn){
            if(u < w){
                paar = std::make_pair(u,w); 
                paar_inverse = std::make_pair(w,u);
                if(orderIn[u] > orderIn[w]){
                    ZoneScopedN("outside in eq sync "); 
                    std::swap(paar, paar_inverse);
                }
                // if the equivalent class doesn't exist yet due to vertex not 
                // having an edge that is important (non-adjacent critical edges) edge.
                if(eq.find(paar) == eq.end()){
                    ZoneScopedN("if we don't find the pair "); 
                    eq[paar] = std::make_shared<nodePairSet>(); 
                    eq[paar]->insert(std::make_pair(u,w));
                    eq[paar_inverse] = std::make_shared<nodePairSet>(); 
                    eq[paar_inverse]->insert(std::make_pair(w,u));
                }

                if(e == nullptr){
                    ZoneScopedN("nullptr case "); 
                    e = eq[paar]; 
                    e_inverse = eq[paar_inverse];
                }
                if(e->find(paar)== e->end() && e->find(paar_inverse)== e->end() ){
                    ZoneScopedN("the syncing  "); 
                    e->insert(eq[paar]->begin(), eq[paar]->end());
                    e_inverse->insert(eq[paar_inverse]->begin(), eq[paar_inverse]->end());

                    eq[paar] = e;
                    eq[paar_inverse] = e_inverse;
                    for(auto& [u1, w1]: *eq[paar]){
                        ZoneScopedN("sync the other eq"); 
                        std::pair<int, int> eqPaar(u1,w1);
                        std::pair<int, int> eqPaar_inverse(w1,u1);
                        eq[eqPaar] = e;  
                        eq[eqPaar_inverse] = e_inverse;
                    }
                }
            }
        }
    }

}

std::pair<std::map<int, std::set<int>>, std::map<int, int>> Contribution::connetectedCompsVerticesMap(const ogdf::NodeArray<int>& connectedcomps, const ogdf::Graph& G, const std::vector<ogdf::NodeElement*>& level){
    ZoneScoped;
    std::pair<std::map<int,std::set<int>>, std::map<int,int>> result; 
    std::map<int, std::set<int>> compToVertices;
    std::map<int, int> vertexToComp;
    ogdf::Array<ogdf::node> nodes; 
    G.allNodes(nodes);
    
    for(const auto& n : nodes){
        compToVertices[connectedcomps[n]].insert(n->index()); 
        vertexToComp[n->index()] = connectedcomps[n]; 
    } 
    return std::pair(compToVertices, vertexToComp);
}
void Contribution::addWeakHananiTutteSpecialCase(const std::vector<ogdf::NodeElement*>& level, 
        const std::vector<ogdf::NodeElement*>& previousLevel,
        equivalentClasses& eq, 
        const ogdf::node& v,
        ogdf::Graph& G, 
        const std::vector<int>& adjIn, 
        std::map<int, ogdf::node>& gVertices,
        ogdf::NodeArray<int>& vertexLevel,
        int levelIndex, 
        const std::map<int,int>& orderIn
        ){
    ZoneScopedN("addWeakHanani function"); 
    // from GraphRegistery<Key> static inline int keyToIndex(Key* key) { return key->index(); }
    // so i can just create a new node 

    /*
       value_ref_type operator[](key_type key) {
       OGDF_ASSERT(getRegistry().isKeyAssociated(key));
       return m_data[registeredAt()->keyToIndex(key)];
       }
       hmm but the issue is that there is a check to seee if the vertex is part of the graph.., so i can't just sneak in a new vertex just for the mapping (either way i can't use the ElementNode constructor..)
       */
    
    // I need to map the vertices to the levels  
    // this is done in the function that calls hanani-tutte weak    
    //
    ogdf::NodeArray<int> connectedComps(G);
    std::map<int, int> minLevelPerComp;
    int n = ogdf::connectedComponents(G, connectedComps); 
    std::vector<int> componentsOrder;
    // Order the connected components based on their vertices with the minimum level.
    


    // After ordering the new connected component. Merge the two connected components. And do the process again.
    // map idNode -> concomp
    auto idConComps = connetectedCompsVerticesMap(connectedComps, G, previousLevel); 
    bool v_connets_2_comps = false;
    std::set<int> connectedCompsMerged; 
    for(auto adj_v: adjIn){
    ZoneScopedN("cut-vertex to which comps?"); 
        for(auto adj_w: adjIn){
            if(idConComps.second[adj_v] != idConComps.second[adj_w]){
                connectedCompsMerged.insert(idConComps.second[adj_v]);
                connectedCompsMerged.insert(idConComps.second[adj_w]);
                v_connets_2_comps = true; 
            }
        }
    }
    /*
       for(int cc: connectedCompsMerged){
       std::cout << cc << ": "; 
       for(int v: idConComps.first[cc]){
       std::cout << " " << v ; 
       }
       std::cout << std::endl;
       }
       std::cout << std::endl;
       */

    gVertices[v->index()] = G.newNode(v->index()); 
    vertexLevel[gVertices[v->index()]] = levelIndex;
    sharedNodePairSet npair_set = nullptr; 
    sharedNodePairSet npair_inverse_set = nullptr; 
    bool rootSet = false;


    // TODO URGENT take in consideration the case where C2 < C2 from the hanani-tutte paper.
    int u,w;
    if(v_connets_2_comps){
        ZoneScopedN("cut-vertex v");
        for(auto it = connectedComps.begin(); it != connectedComps.end(); it++ ){
            ZoneScopedN("finding min level per connected comp");
            auto v = it.key();
            int conComp = *it; 
            if((connectedCompsMerged.find(conComp) != connectedCompsMerged.end()) && (minLevelPerComp.find(conComp)== minLevelPerComp.end() || vertexLevel[v] <  minLevelPerComp[conComp])){
                minLevelPerComp[conComp]  = vertexLevel[v];
            }
        }

        for(auto& [c,k]: minLevelPerComp){
            componentsOrder.push_back(c); 
        }

        {
        ZoneScopedN("quicksort");
        quickSort<int>(componentsOrder, 0, componentsOrder.size()-1, minLevelPerComp); 
        }
        // order the adjacent vertices (sloppy..)
        int orderAdjacentVertices[orderIn.size()]; 
        for(auto [v,order] : orderIn){
            orderAdjacentVertices[order] = v;   
        }
        // First component doesn't need any traitement

        //TODO I need map the level to vertices of the connected comp.
        // The updated version 
        std::unordered_set<int> oneLevelVertices; 
        for(size_t i = 1; i < componentsOrder.size(); i++){
            ZoneScopedN("component i"); 
            // find all vertices on the level minH of componentsOrder[1]. 
            int minLevelCompI = minLevelPerComp[componentsOrder[i]];
            oneLevelVertices.clear();
            for(auto vertexIndex: idConComps.first[componentsOrder[0]]){
                ZoneScopedN("find the vertices to work on in comp 0"); 
                if(vertexLevel[vertexIndex] == minLevelCompI) {
                    oneLevelVertices.insert(vertexIndex);         
                }
            }           
            for(auto vertexIndex: idConComps.first[componentsOrder[i]]){
                ZoneScopedN("find the vertices to work on in comp i"); 
                if(vertexLevel[vertexIndex] == minLevelCompI) {
                    oneLevelVertices.insert(vertexIndex);         
                }
                // merge the two comps for the next iteration of the connected comps.
                idConComps.first[componentsOrder[0]].insert(vertexIndex);
            }           
            // we have all the vertices of th level minlevelComp_i and also we
            // merged the comp 0 with comp i in comp 0
            std::map<int, ogdf::node> nodes;
            std::map<std::pair<int,int>, ogdf::edge> edges;
            ogdf::Graph G_rel; 
            // we just go throught all the vertices of the two comps in the
            // min level of comp i and create graph based on the equivalence classes.

            for(int z : oneLevelVertices){
                ZoneScopedN("pick a z from the vetices"); 
                for(int y : oneLevelVertices){
                ZoneScopedN("pick a y from the vertices"); 
                    if(z < y){
                        std::pair<int,int> keyPair(z,y);
                        std::pair<int,int> keyPairInverse(y,z);

                        if(eq.find(keyPair) == eq.end()){
                            eq[keyPair] = std::make_shared<nodePairSet>();
                            eq[keyPair]->insert(keyPair);
                            eq[keyPairInverse] = std::make_shared<nodePairSet>();
                            eq[keyPairInverse]->insert(keyPairInverse);
                        }
                        sharedNodePairSet pKeyPairSet = eq[keyPair];
                        sharedNodePairSet pKeyPairSetInverse = eq[keyPairInverse];  


                        auto& pairSet = *eq[keyPair];
                        for(auto pPair: pairSet){
                            ZoneScopedN("create the graph"); 
                            int u = pPair.first; 
                            int a = pPair.second; 
                            if(oneLevelVertices.find(u) != oneLevelVertices.end() && oneLevelVertices.find(a) != oneLevelVertices.end()){
                                auto itU = nodes.find(u);
                                auto ita = nodes.find(a);
                                if(itU == nodes.end()) {
                                    nodes[u] = G_rel.newNode(u);
                                }
                                if(ita == nodes.end()){
                                    nodes[a] = G_rel.newNode(a);
                                }
                                if(edges.find(std::pair(a,u)) == edges.end() && edges.find(std::pair(u,a)) == edges.end()){
                                    edges[std::pair(u,a)] = G_rel.newEdge(nodes[u], nodes[a]); 
                                } 
                            }
                           
                        }

                    }
                }
            }
            ogdf::NodeArray<int> ordering(G_rel); 
            // calculate a topological ordering
            {
                ZoneScopedN("topological numbering");
            ogdf::topologicalNumbering(G_rel, ordering);	
            }
            sharedNodePairSet pRootPairSet = nullptr;
            sharedNodePairSet pRootPairSetInverse = nullptr;

            for(auto it = ordering.begin(); it != ordering.end(); it++){
                ZoneScopedN("go throught the ordering");
                auto a = it.key(); 
                for(auto it1 = ordering.begin(); it1 != ordering.end(); it1++ ){
                    auto w = it1.key();
                    auto vOrder = *it;
                    auto wOrder = *it1;
                    if(a->index() < w->index()){
                        if(eq.find(std::pair(a->index(), w->index())) == eq.end()){
                            eq[std::pair<int,int>(a->index(), w->index())] = std::make_shared<nodePairSet>();
                            eq[std::pair<int,int>(a->index(), w->index())]->insert(std::pair<int,int>(a->index(), w->index()));
                            eq[std::pair<int,int>(w->index(), a->index())] = std::make_shared<nodePairSet>();
                            eq[std::pair<int,int>(w->index(), a->index())]->insert(std::pair<int,int>(w->index(), a->index()));
                        }   
                        auto& pPairSet = eq[std::pair<int,int>(a->index(), w->index())];  
                        auto& pPairSetInverse = eq[std::pair<int,int>(w->index(), a->index())];  
                        if(pRootPairSet == nullptr){
                            if(vOrder < wOrder){
                                pRootPairSet = pPairSet;                             
                                pRootPairSetInverse = pPairSetInverse; 
                            } else {
                                pRootPairSet = pPairSetInverse;                             
                                pRootPairSetInverse = pPairSet; 
                            }
                        }
                        if(pRootPairSet != pPairSet && pRootPairSet != pPairSetInverse){
                            if(vOrder < wOrder){
                                pRootPairSet->insert(pPairSet->begin(), pPairSet->end());
                                pPairSet = pRootPairSet; 
                                pRootPairSetInverse->insert(pPairSetInverse->begin(), pPairSetInverse->end());
                                pPairSetInverse = pRootPairSetInverse; 
                            
                            } else {
                                pRootPairSet->insert(pPairSetInverse->begin(), pPairSetInverse->end());
                                pPairSetInverse = pRootPairSet; 
                                pRootPairSetInverse->insert(pPairSet->begin(), pPairSet->end());
                                pPairSet = pRootPairSetInverse; 
                            }
                            for(auto& p: *pPairSet){
                                ZoneScopedN("more sync inside ordering");
                               eq[p] = pRootPairSet; 
                            }
                        }
                    }
                }
            }
            // create relation graph 
            // topological sorting 
            // fix the equivalence classes of the absolute order that we get.  
        }
        // Old version
        /*
        for(size_t i = 1; i < componentsOrder.size(); i++){
            ZoneScopedN("merging connected comps");
            bool foundTargetCompVertex = false;
            int lastVFromC1 = -1; 

            for(int vIndex: orderAdjacentVertices){
                ZoneScopedN("finding closest vertex from comp 0 in vertices Neighbors to v in h - 1");
                // If we found the first vertex from the connected component that we want to draw, we stop
                if(idConComps.second[vIndex] == componentsOrder[i]){
                    break;
                }
                // look for the last vertex from the adjacent vertices to v while still not finding a vertex from the connected comp to draw.
                if(idConComps.second[vIndex] == componentsOrder[0]){
                    lastVFromC1 = vIndex;
                } 
            }
            // TODO gotta solve the v = 14, l=3 with vertex 7 situation
            if(lastVFromC1 == -1){
                for(int vIndex: orderAdjacentVertices){
                    ZoneScopedN("finding closest vertex from comp 0 in vertices Neighbors to v in h - 1");
                    // If we found the first vertex from the connected component that we want to draw, we stop
                    if(idConComps.second[vIndex] == componentsOrder[i]){
                        foundTargetCompVertex = true; 
                    }
                    // look for the nearest vertex from the adjacent vertices to v while still after finding a vertex from the connected comp to draw.
                    if(idConComps.second[vIndex] == componentsOrder[0] && foundTargetCompVertex){
                        lastVFromC1 = vIndex;
                        break;
                    } 
                }
            }
            if(lastVFromC1 != -1){
                ZoneScopedN("in case we found the closet v from c0 before comp in question");
                npair_set = nullptr;
                for(int wIndex: idConComps.first[componentsOrder[i]]){
                    // If the vindex \in C0 is after the first vertex in the Ci, we switch the pair order 
                    std::pair mainPair(lastVFromC1, wIndex);
                    std::pair mainPair_inverse(wIndex,lastVFromC1);
                    
                    if(foundTargetCompVertex){
                        auto temp = mainPair; 
                        mainPair = mainPair_inverse; 
                         mainPair_inverse = temp; 
                    }
                    
                    auto& set_mainPair = eq[mainPair]; 
                    auto& set_mainPair_inverse = eq[mainPair_inverse]; 
                    if(!set_mainPair){
                        set_mainPair = std::make_shared<nodePairSet>();
                        set_mainPair->insert(mainPair);
                        set_mainPair_inverse = std::make_shared<nodePairSet>();
                        set_mainPair_inverse->insert(mainPair_inverse);
                    }
                    

                    if(npair_set == nullptr){
                           npair_set = eq[mainPair]; 
                           npair_inverse_set = eq[mainPair_inverse];
                    }

                    if(npair_set->find(mainPair_inverse) == npair_set->end()){
                        if(npair_set != eq[mainPair]){
                            npair_set->insert(eq[mainPair]->begin(), eq[mainPair]->end()); // TODO check if they have the same pointer.
                            eq[mainPair] = npair_set;
                            npair_inverse_set->insert(eq[mainPair_inverse]->begin(), eq[mainPair_inverse]->end());
                            eq[mainPair_inverse] = npair_inverse_set;
                            for(auto& [u1, w1]: *eq[mainPair]){
                                ZoneScopedN("propagate the merge else");
                                eq[std::pair(u1,w1)] = eq[mainPair]; 
                                eq[std::pair(w1,u1)] = eq[mainPair_inverse]; 
                            }

                        
                        }
                    
                    }else {
                        npair_set->insert(eq[mainPair_inverse]->begin(), eq[mainPair_inverse]->end());
                        eq[mainPair_inverse] = npair_set;
                        npair_inverse_set->insert(eq[mainPair]->begin(), eq[mainPair]->end());
                        eq[mainPair] = npair_inverse_set;
                    }
                    
                    for(int vIndex: idConComps.first[componentsOrder[0]]){
                        ZoneScopedN("work with all vertices of c0");
                        std::cout << "vIndex: " << vIndex << std::endl; 
                        if(vIndex != lastVFromC1){
                            // 30 -> 34
                            std::pair pair(lastVFromC1, vIndex); 
                            // 10 -> 34
                            std::pair copierPair(wIndex, vIndex);
                            std::pair pair_inverse(vIndex, lastVFromC1); 
                            std::pair copierPair_inverse(vIndex, wIndex);
                            auto& set_pair = eq[pair];
                            auto& set_pair_inverse = eq[pair_inverse];
                            auto& set_copierPair = eq[copierPair];
                            auto& set_copierPair_inverse = eq[copierPair_inverse];  
                            if(!set_pair){
                                set_pair = std::make_shared<nodePairSet>();
                                set_pair->insert(pair);
                                set_pair_inverse = std::make_shared<nodePairSet>();
                                set_pair_inverse->insert(pair_inverse);
                            }

                            if(!set_copierPair){
                                set_copierPair = std::make_shared<nodePairSet>();
                                set_copierPair ->insert(copierPair);
                                set_copierPair_inverse = std::make_shared<nodePairSet>();
                                set_copierPair_inverse->insert(copierPair_inverse);
                                 
                            }

                            // 34 -> 10 is not in 30 -> 34 true 
                            if(set_pair->find(copierPair_inverse) == set_pair->end()){
                                // eq(10,34) != eq(30,34) -> true;
                                if(set_pair != set_copierPair){
                                    // add 10->34 (just created i think, so eq have only its key) to eq(30->34) 
                                    set_pair->insert(set_copierPair->begin(), set_copierPair->end());
                                    set_copierPair = set_pair; 
                                    set_pair_inverse->insert(set_copierPair_inverse->begin(), set_copierPair_inverse->end()); 
                                    set_copierPair_inverse = set_pair_inverse;
                                    // irrelavent in this case (empty equiv) 
                                    for(auto& [u1, w1]: *eq[copierPair]){
                                        ZoneScopedN("propagate the merge if");
                                        eq[std::pair(u1,w1)] = set_copierPair; 
                                        eq[std::pair(w1,u1)] = set_copierPair_inverse; 
                                    }
                                }
                            } else {
                                if(set_pair != set_copierPair_inverse){
                                    set_pair->insert(set_copierPair_inverse->begin(), set_copierPair_inverse->end());
                                    set_copierPair_inverse = set_pair; 
                                    set_pair_inverse->insert(set_copierPair->begin(), set_copierPair->end()); 
                                    set_copierPair = set_pair_inverse;
                                    for(auto& [u1, w1]: *eq[copierPair]){
                                        ZoneScopedN("propagate the merge else");
                                        eq[std::pair(u1,w1)] = set_copierPair; 
                                        eq[std::pair(w1,u1)] = set_copierPair_inverse; 
                                    }
                                }
                            }

                        }
                    }
                }
                // Put wIndex in first connected comp
                for(int wIndex: idConComps.first[componentsOrder[i]]){
                    idConComps.first[componentsOrder[0]].insert(wIndex);
                    idConComps.second[wIndex] = componentsOrder[0];
                }
                

            } else {
                ZoneScopedN("in case we didn't find the closest v in c0 before c in question");
                npair_set = nullptr;
                for(int wIndex: idConComps.first[componentsOrder[i]]){
                    for(int vIndex: idConComps.first[componentsOrder[0]]){
                        std::pair pair(wIndex, vIndex); 
                        std::pair pair_inverse(vIndex, wIndex); 

                        if(eq.find(pair) == eq.end()){
                            eq[pair] = std::make_shared<nodePairSet>(); 
                            eq[pair]->insert(pair);
                            eq[pair_inverse] = std::make_shared<nodePairSet>(); 
                            eq[pair_inverse]->insert(pair_inverse);
                        }
                        if(npair_set == nullptr){
                            npair_set = eq[pair]; 
                            npair_inverse_set = eq[pair_inverse];
                        }

                        if(npair_set->find(pair_inverse) == npair_set->end()){
                            npair_set->insert(eq[pair]->begin(), eq[pair]->end());
                            eq[pair] = npair_set;
                            npair_inverse_set->insert(eq[pair_inverse]->begin(), eq[pair_inverse]->end());
                            eq[pair_inverse] = npair_inverse_set;

                        } else {
                            npair_set->insert(eq[pair_inverse]->begin(), eq[pair_inverse]->end());
                            eq[pair_inverse] = npair_set;
                            npair_inverse_set->insert(eq[pair]->begin(), eq[pair]->end());
                            eq[pair] = npair_inverse_set;
                        }


                        for(auto& [u1, w1]: *eq[pair]){
                            std::pair<int, int> eqPaar(u1,w1);
                            std::pair<int, int> eqPaar_inverse(w1,u1);
                            eq[eqPaar] = eq[pair]; 
                            eq[eqPaar_inverse] = eq[pair_inverse]; 
                        }
                    }
                    // Put wIndex in first connected comp
                }
                for(int wIndex: idConComps.first[componentsOrder[i]]){
                    idConComps.first[componentsOrder[0]].insert(wIndex);
                    idConComps.second[wIndex] = componentsOrder[0];
                }

                std::cout << "ALERT? SHOULD THIS HAPPEN?" << std::endl;
                //  ?
            }
        }
        */

        // TODO TO REMOVE
        /*for(int cc1: connectedCompsMerged){
            for(int cc2: connectedCompsMerged){
                npair_set = nullptr;
                if(cc1 < cc2){
                    //TODO do i really need to consider C2 < C1, is it not possible to just push the smaller 
                    //encapsulated connected comp outside of the encapsulating one?
                    for(int vIndex: idConComps.first[cc1]) {
                        for(int wIndex: idConComps.first[cc2]){
                            if(vIndex == 6 && wIndex == 5 || vIndex == 5 && wIndex == 12){
                                std::cout << "vIndex: " << vIndex << ", wIndex: " << wIndex <<std::endl; 
                                std::cout << "debug" <<std::endl; 
                            }
                            std::pair vwPair(vIndex, wIndex);
                            std::pair vwPair_inverse(wIndex, vIndex);
                            if(eq.find(vwPair) == eq.end()){
                                eq[vwPair] = std::make_shared<nodePairSet>(); 
                                eq[vwPair]->insert(std::pair(vIndex, wIndex));
                                eq[vwPair_inverse] = std::make_shared<nodePairSet>(); 
                                eq[vwPair_inverse]->insert(std::pair(wIndex, vIndex));
                            }
                            if(npair_set == nullptr){
                                npair_set = eq[vwPair];
                                npair_inverse_set = eq[vwPair_inverse];
                            }
                            if(npair_set->find(vwPair_inverse) != npair_set->end()){
                                {
                                    ZoneScopedN("syncing in weak ht2");
                                    npair_set->insert(eq[vwPair_inverse]->begin(),eq[vwPair_inverse]->end() );
                                    eq[vwPair_inverse] = npair_set; 
                                    npair_inverse_set->insert(eq[vwPair]->begin(),eq[vwPair]->end());
                                    eq[vwPair] = npair_inverse_set; 
                                }
                                for(auto& [u1, w1]: *eq[vwPair]){
                                    std::pair<int, int> eqPaar(u1,w1);
                                    std::pair<int, int> eqPaar_inverse(w1,u1);
                                    eq[eqPaar] = npair_inverse_set;  
                                    eq[eqPaar_inverse] = npair_set;
                                }
                            } else {
                                {
                                    ZoneScopedN("syncing in weak ht1");
                                    npair_set->insert(eq[vwPair]->begin(),eq[vwPair]->end() );
                                    eq[vwPair] = npair_set; 
                                    npair_inverse_set->insert(eq[vwPair_inverse]->begin(),eq[vwPair_inverse]->end());
                                    eq[vwPair_inverse] = npair_inverse_set; 
                                }
                                for(auto& [u1, w1]: *eq[vwPair]){
                                    std::pair<int, int> eqPaar(u1,w1);
                                    std::pair<int, int> eqPaar_inverse(w1,u1);
                                    eq[eqPaar] = npair_set;  
                                    eq[eqPaar_inverse] = npair_inverse_set;
                                }

                            }

                            //std::cout << "DEBUG ::: "<< vIndex << "," << wIndex << " merged with " << u << "," << w <<std::endl; 
                        }
                    }

                }
            }
        }*/
    }
    for(auto& adj_v_index: adjIn){
        G.newEdge(gVertices[adj_v_index], gVertices[v->index()]);
    }

}
void Contribution::reduceEquivalentClasses(std::vector<std::vector<ogdf::NodeElement*>>& emb, equivalentClasses& eq){
    ZoneScopedN("the reduce function");
    
    ogdf::Graph G; 
    ogdf::NodeArray<int> vertexlevel(G);
    std::vector<int> adjIn, adjOut;
    //TODO started caring less about the structure, this needs refactoring
    std::map<int, ogdf::node> gVertices; 
    // the instantiation doesn't change much here, I just don't want it to be null.
    std::vector<ogdf::node> previousLevel = emb[0];
    int levelIndex = 0;
    for(const auto& level : emb){
        for(const auto& v : level){
            std::vector<int> adjOut, adjIn;
            std::map<int, int> orderIn, orderOut;
            for(const auto& adj : v->adjEntries){
                ogdf::edge e = adj->theEdge(); 
                if(v->index() == e->source()->index()){
                    //TODO refactor so i work with vertices as keys for the containers that i am using.
                    adjOut.push_back(e->target()->index());  
                } else {
                    adjIn.push_back(e->source()->index());
                }
            }
            {
                ZoneScopedN("sort");
            sort(adjIn.begin(), adjIn.end());
            sort(adjOut.begin(), adjOut.end());
            }
            addAdjacentEdgesRestrition(level, eq, v, adjOut, adjIn, orderOut, orderIn);
            addWeakHananiTutteSpecialCase(level, previousLevel, eq, v, G, adjIn, gVertices, vertexlevel, levelIndex, orderIn);
        }
        /*
           for(const auto& v : level){
           std::vector<int> adjOut, adjIn;
           for(const auto& adj : v->adjEntries){
           ogdf::edge e = adj->theEdge(); 
           if(v->index() == e->source()->index()){
        //TODO refactor so i work with vertices as keys for the containers that i am using.
        adjOut.push_back(e->target()->index());  
        } else {
        adjIn.push_back(e->source()->index());
        }
        }
        sort(adjIn.begin(), adjIn.end());
        sort(adjOut.begin(), adjOut.end());

        addWeakHananiTutteSpecialCase(level, previousLevel, eq, v, G, connectedComps, adjIn, gVertices);
        }
        */
        previousLevel = level;
        levelIndex++;
    }
}
