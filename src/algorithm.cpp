#include "algorithm.hpp"
#include "utils.hpp"

/*equivalentClasses*/ std::vector<int> Contribution::addAdjacentEdgesRestrition(const std::vector<ogdf::NodeElement*>& level, equivalentClasses& eq, const ogdf::node& v, const std::vector<int>& adjOut, const std::vector<int>& adjIn ){
    std::map<int,int> orderIn, orderOut; 
    int counter = 0;
    for(const auto i : adjIn){
        orderIn[i] = counter++;
    }
    counter = 0;
    for(const auto i : adjOut){
        orderOut[i] = counter++;
    }
    for(const auto u:adjOut){
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
        for(const auto w:adjIn){
            if(u < w && eq.find(std::make_pair(u ,w ))!= eq.end()){
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
        for(const auto  w : adjOut){
            if(u < w){
                paar = std::make_pair(u,w); 
                paar_inverse = std::make_pair(w,u);
                if(orderIn[u] > orderIn[w]){
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
                e->insert(eq[paar]->begin(), eq[paar]->end());
                e_inverse->insert(eq[paar_inverse]->begin(), eq[paar_inverse]->end());

                eq[paar] = e;
                eq[paar_inverse] = e_inverse;
            }
        }
    }

    e = nullptr;
    e_inverse = nullptr;

    for(const auto u: adjIn ){
        for(const auto  w : adjIn){
            if(u < w){
                paar = std::make_pair(u,w); 
                paar_inverse = std::make_pair(w,u);
                if(orderIn[u] > orderIn[w]){
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
                e->insert(eq[paar]->begin(), eq[paar]->end());
                e_inverse->insert(eq[paar_inverse]->begin(), eq[paar_inverse]->end());

                eq[paar] = e;
                eq[paar_inverse] = e_inverse;
            }
        }
    }

    return std::move(adjIn);
}

std::pair<std::map<int, std::set<int>>, std::map<int, int>> Contribution::connetectedCompsVerticesMap(const ogdf::NodeArray<int>& connectedcomps, const ogdf::Graph& G, const std::vector<ogdf::NodeElement*>& level){
    std::pair<std::map<int,std::set<int>>, std::map<int,int>> result; 
    std::map<int, std::set<int>> compToVertices;
    std::map<int, int> vertexToComp;
    ogdf::Array<ogdf::node> nodes; 
    G.allNodes(nodes);
    for(const auto& n : nodes){
        for(const auto & levelNode : level){
            if(n->index() == levelNode->index()){
                compToVertices[connectedcomps[n]].insert(n->index()); 
                vertexToComp[n->index()] = connectedcomps[n]; 
            }
        }
    } 
    return std::pair(compToVertices, vertexToComp);
}
void Contribution::addWeakHananiTutteSpecialCase(const std::vector<ogdf::NodeElement*>& level, 
        const std::vector<ogdf::NodeElement*>& previousLevel,
        equivalentClasses& eq, 
        const ogdf::node& v,
        ogdf::Graph& G, 
        ogdf::NodeArray<int>& connectedComps,
        const std::vector<int>& adjIn, 
        std::map<int, ogdf::node>& gVertices){
    // from GraphRegistery<Key> static inline int keyToIndex(Key* key) { return key->index(); }
    // so i can just create a new node 

    /*
       value_ref_type operator[](key_type key) {
       OGDF_ASSERT(getRegistry().isKeyAssociated(key));
       return m_data[registeredAt()->keyToIndex(key)];
       }
       hmm but the issue is that there is a check to seee if the vertex is part of the graph.., so i can't just sneak in a new vertex just for the mapping (either way i can't use the ElementNode constructor..)
       */


    // TODO gotta work on the datastructure for saving the connected comps.
    ogdf::connectedComponents(G, connectedComps); 
    // map idNode -> concomp
    auto idConComps = connetectedCompsVerticesMap(connectedComps, G, previousLevel); 
    bool v_connets_2_comps = false;
    std::set<int> connectedCompsMerged; 
    for(auto adj_v: adjIn){
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
    sharedNodePairSet npair_set = nullptr; 
    sharedNodePairSet npair_inverse_set = nullptr; 
    // TODO URGENT take in consideration the case where C2 < C2 from the hanani-tutte paper.
    int u,w;
    for(int cc1: connectedCompsMerged){

        for(int cc2: connectedCompsMerged){
            if(cc1 < cc2){
                //TODO do i really need to consider C2 < C1, is it not possible to just push the smaller 
                //encapsulated connected comp outside of the encapsulating one?
                for(int vIndex: idConComps.first[cc1]) {
                    for(int wIndex: idConComps.first[cc2]){
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
                        npair_set->insert(eq[vwPair]->begin(),eq[vwPair]->end() );
                        eq[vwPair] = npair_set; 
                        npair_inverse_set->insert(eq[vwPair_inverse]->begin(),eq[vwPair_inverse]->end());
                        eq[vwPair_inverse] = npair_inverse_set; 

                        //std::cout << "DEBUG ::: "<< vIndex << "," << wIndex << " merged with " << u << "," << w <<std::endl; 
                    }
                }

            }
        }
    }
    for(auto& adj_v_index: adjIn){
        std::cout << "DEBUG:::: creating a new edge between " << adj_v_index << " and " << v->index() << std::endl;
        G.newEdge(gVertices[adj_v_index], gVertices[v->index()]);
    }

}
equivalentClasses Contribution::reduceEquivalentClasses(std::vector<std::vector<ogdf::NodeElement*>>& emb, equivalentClasses& eq){
    equivalentClasses eq_old_save; 
    for(const auto& [pair,sharedset]: eq){
        eq_old_save[pair] = std::make_shared<nodePairSet>(); 
        for(const auto& eqPair : *sharedset){
            eq_old_save[pair]->insert(std::pair(eqPair.first, eqPair.second));
        }
    }
    ogdf::Graph G; 
    ogdf::NodeArray<int> connectedComps(G);
    std::vector<int> adjIn, adjOut;
    //TODO started caring less about the structure, this needs refactoring
    std::map<int, ogdf::node> gVertices; 
    // the instantiation doesn't change much here, I just don't want it to be null.
    std::vector<ogdf::node> previousLevel = emb[0];
    for(const auto& level : emb){
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
            addAdjacentEdgesRestrition(level, eq, v, adjOut, adjIn);
        }
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
        previousLevel = level;
    }
    return eq_old_save;
}
