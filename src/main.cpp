#include <stdio.h>
#include <ogdf/basic/Graph.h>
#include <ogdf/basic/graph_generators.h>
#include <ogdf/fileformats/GraphIO.h>
#include <ogdf/layered/DfsAcyclicSubgraph.h>
#include <ogdf/basic/GraphAttributes.h>
#include <ogdf/basic/geometry.h>
#include <ogdf/basic/simple_graph_alg.h>
#include <cassert>
#include <string>
#include <algorithm>
#include <memory>
#include <cmath>
#include "NodePartition.h"
#include "GraphBuilder.h"

using namespace ogdf;

using nodePair = std::pair<int, int>;
using nodePairSet = std::set<nodePair>; 
using sharedNodePairSet = std::shared_ptr<nodePairSet>;
using equivalentClasses = std::map<nodePair, sharedNodePairSet>;
using equivalentClassesAssignement = std::map<nodePair, int>;

void print_assignment(std::string_view comment, const equivalentClassesAssignement& assignement){
    std::cout << comment;
    for (const auto& [key, value] : assignement){
        std::cout << '[' << key.first << "," << key.second << "] = " << value << std::endl;
    }

    std::cout << '\n';

}
void print_mapint(std::string_view comment, const std::map<int,int>& m){
    std::cout << comment;
    for (const auto& [key, value] : m){
        std::cout << '[' << key << "] = " << value ; 
    }

    std::cout << '\n';
}
template<class Key>
void print_vector(std::string_view comment, const std::vector<Key>& s){

    std::cout << comment;
    for (const auto&  value : s){
        std::cout << ", " << value; 
    }
    std::cout << std::endl;

}

void print_set(std::string_view comment, const std::set<nodePair>& s){

    std::cout << comment;
    for (const auto&  value : s){
        std::cout << ", " << value.first <<  ", " << value.second << "; ";
    }

}
void print_map(std::string_view comment, const std::map<nodePair, sharedNodePairSet>& m)
{
    std::cout << comment;
    for (const auto& [key, value] : m){

        std::cout << '[' << key.first << ", " << key.second  << "] = "; 
        print_set("set: ", *value); 
        std::cout << '\n';
    }

    std::cout << '\n';
}
void print_map(std::string_view comment, const std::map<nodePair, nodePairSet>& m)
{
    std::cout << comment;
    // Iterate using C++17 facilities
    for (const auto& [key, value] : m){

        std::cout << '[' << key.first << ", " << key.second  << "] = "; 
        print_set("set: ", value); 
        std::cout << '\n';
    }

    std::cout << '\n';
}



std::map<nodePair, sharedNodePairSet> compute2SATClasses(std::vector<std::vector<NodeElement*>>emb){
    // sync 
    std::map<nodePair, nodePairSet> sync; 

    std::vector<ogdf::edge> E; 
    for(const auto& nodes : emb){
        E.clear();
        for(const auto& n : nodes){
            for(const auto& adj : n->adjEntries) {
                if(adj->isSource()) {
                    E.push_back(adj->theEdge());
                }
            }
        }
        ogdf::edge f, s; 
        ogdf::node a,b,c,d ; 
        for(size_t i = 0; i < E.size(); i++){
            for(size_t j = i + 1; j < E.size(); j++){
                f = E[i];  
                s = E[j];
                a = f->source(); b = s->source();
                c = f->target(); d = s->target(); 
                if(!(a->index() == b->index() || c->index() == d->index())){
                    sync[std::make_pair(a->index(), b->index())].insert(std::make_pair(c->index(),d->index())); 
                    sync[std::make_pair(b->index(), a->index())].insert(std::make_pair(d->index(),c->index())); 
                    sync[std::make_pair(c->index(), d->index())].insert(std::make_pair(a->index(),b->index())); 
                    sync[std::make_pair(d->index(), c->index())].insert(std::make_pair(b->index(),a->index())); 
                }
            } 
        }
    }

    equivalentClasses eq; 
    std::vector<nodePair> todo;

    for(const auto& [key, value] : sync){
        int a = key.first;
        int b = key.second;
        if(a  > b ){
            int temp = a ; 
            a  = b ; 
            b  = temp;
        }
        if(eq.find(std::make_pair(a ,b ))!= eq.end()){
            continue;
        }
        nodePair mainPair(a,b);
        nodePair mainPairInversed(b,a);
        eq[mainPair] = std::make_shared<nodePairSet>(); 
        eq[mainPairInversed] = std::make_shared<nodePairSet>(); 
        todo.clear(); 
        todo.push_back(mainPair);

        while(!todo.empty()){
            int c = todo.back().first; 
            int d = todo.back().second; 
            nodePair pair(c,d), pairInversed(d,c);
            todo.pop_back();
            if(mainPair != pair){
                // TODO add the assert.
            }
            eq[pair] = eq[mainPair];
            eq[pairInversed] = eq[mainPairInversed];

            eq[mainPair]->insert(sync[pair].begin(), sync[pair].end());
            eq[mainPairInversed]->insert(sync[pairInversed].begin(), sync[pairInversed].end());

            for(auto& p : sync[pair]) {
                // if we don't find the pair yet in the eq class. 
                if( eq.find(p) == eq.end())
                    todo.push_back(std::make_pair(p.first, p.second));    
            }

        }
    }
    return eq; 
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


/*equivalentClasses*/ std::vector<int> addAdjacentEdgesRestrition(const std::vector<ogdf::NodeElement*>& level, const equivalentClasses& eqOrg, equivalentClasses& eq, const node& v, const std::vector<int>& adjOut, const std::vector<int>& adjIn ){
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

std::pair<std::map<int, std::set<int>>, std::map<int, int>> connetectedCompsVerticesMap(const ogdf::NodeArray<int>& connectedcomps, const ogdf::Graph& G, const std::vector<ogdf::NodeElement*>& level){
    std::pair<std::map<int,std::set<int>>, std::map<int,int>> result; 
    std::map<int, std::set<int>> compToVertices;
    std::map<int, int> vertexToComp;
    ogdf::Array<node> nodes; 
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
void addWeakHananiTutteSpecialCase(const std::vector<ogdf::NodeElement*>& level, const std::vector<ogdf::NodeElement*>& previousLevel, const equivalentClasses& eqOrg, equivalentClasses& eq, const node& v, ogdf::Graph& G, ogdf::NodeArray<int>& connectedComps, const std::vector<int>& adjIn, std::map<int, node>& gVertices){
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
    // TODO take in consideration the case where C2 < C2 from the hanani-tutte paper.
    int u,w;
    for(int cc1: connectedCompsMerged){
        for(int cc2: connectedCompsMerged){
            if(cc1 < cc2){
                for(int vIndex: idConComps.first[cc1]) {
                    for(int wIndex: idConComps.first[cc2]){
                        if(eq.find(std::pair(vIndex, wIndex)) == eq.end()){
                            eq[std::pair(vIndex, wIndex)] = std::make_shared<nodePairSet>(); 
                            eq[std::pair(wIndex, vIndex)] = std::make_shared<nodePairSet>(); 
                        }
                        if(npair_set == nullptr){
                            u = vIndex; 
                            w = wIndex;
                            npair_set = eq[std::pair(vIndex, wIndex)];
                            npair_inverse_set = eq[std::pair(wIndex, vIndex)];
                        }
                        npair_set->insert(eq[std::pair(vIndex, wIndex)]->begin(),eq[std::pair(vIndex, wIndex)]->end() );
                        eq[std::pair(vIndex, wIndex)] = npair_set; 
                        npair_inverse_set->insert(eq[std::pair(wIndex, vIndex)]->begin(),eq[std::pair(wIndex, vIndex)]->end());
                        eq[std::pair(wIndex, vIndex)] = npair_inverse_set; 

                        std::cout << "DEBUG ::: "<< vIndex << "," << wIndex << " merged with " << u << "," << w <<std::endl; 
                    }
                }

            }
        }
    }
    for(auto& adj_v_index: adjIn){
        G.newEdge(gVertices[adj_v_index], gVertices[v->index()]);
    }

}
void createLayout(std::string nameFile, ogdf::Graph& G){
    GraphAttributes GA(G,
            GraphAttributes::all );
    NodeArray<int> mappings(G); 
    ogdf::Array<node> nodes; 
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
    GraphIO::write(GA, "relation.gml", GraphIO::writeGML);
    GraphIO::write(GA, "relation.svg", GraphIO::drawSVG);

}
bool AcyclicRelation(equivalentClassesAssignement assignement){
    std::map<int, node> nodes;
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

equivalentClasses reduceEquivalentClasses(std::vector<std::vector<ogdf::NodeElement*>>& emb, const equivalentClasses& eqOrg){
    equivalentClasses eq = eqOrg; 
    /*
       for(auto& [pair,sharedset]: eqOrg){
       eq[pair] = std::make_shared<nodePairSet>(); 
       for(auto& element: *sharedset){
       eq[pair]->insert(element);
       }
       }
       */
    ogdf::Graph G; 
    ogdf::NodeArray<int> connectedComps(G);
    std::vector<int> adjIn, adjOut;
    //TODO started caring less about the structure, this needs refactoring
    std::map<int, node> gVertices; 
    // the instantiation doesn't change much here, I just don't want it to be null.
    std::vector<node> previousLevel = emb[0];
    for(const auto& level : emb){
        for(const auto& v : level){
            std::vector<int> adjOut, adjIn;
            for(const auto& adj : v->adjEntries){
                edge e = adj->theEdge(); 
                if(v->index() == e->source()->index()){
                    //TODO refactor so i work with vertices as keys for the containers that i am using.
                    adjOut.push_back(e->target()->index());  
                } else {
                    adjIn.push_back(e->source()->index());
                }
            }
            sort(adjIn.begin(), adjIn.end());
            sort(adjOut.begin(), adjOut.end());
            addAdjacentEdgesRestrition(level, eqOrg, eq, v, adjOut, adjIn);
            addWeakHananiTutteSpecialCase(level, previousLevel, eqOrg, eq, v, G, connectedComps, adjIn, gVertices);
        }
        previousLevel = level;
    }
    return eq;
}
int main(int argc, char* argv[]){


    std::string graphFile;
    if(argc < 2){
        graphFile = "counterexample.gml";
    } else {
        graphFile = argv[1]; 
    }
    GraphBuilder graphBuild; 

    auto emb = graphBuild.buildLevelGraphFromGML(graphFile);

    equivalentClasses eq = compute2SATClasses(emb);


    GraphIO::write(graphBuild.GA, "input_graph.svg", GraphIO::drawSVG);

    equivalentClasses eqReduced = reduceEquivalentClasses(emb, eq);
    equivalentClassesAssignement assignement = fillEquivalentClasses(eqReduced);

    bool result = planarityCheck(assignement, eq);
    //print_map("reduced eq", eqReduced);


    std::cout << "the planarity check : " << result << std::endl; 


    bool acyclic = AcyclicRelation(assignement); 
    if(acyclic == true){
        std::cout << "No cyclic relation." << std::endl ;
    } else {

        std::cout << "exists a cyclic relation." << std::endl;;
    }

    return 0;
}
