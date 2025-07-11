#include <stdio.h>
#include <ogdf/basic/Graph.h>
#include <ogdf/basic/graph_generators.h>
#include <ogdf/fileformats/GraphIO.h>
#include <ogdf/layered/DfsAcyclicSubgraph.h>
#include <ogdf/basic/GraphAttributes.h>
#include <ogdf/basic/geometry.h>
#include <cassert>
#include <string>
#include <algorithm>
#include <memory>
#include "NodePartition.h"
#include "GraphBuilder.h"

using namespace ogdf;

using nodePair = std::pair<int, int>;
using nodePairSet = std::set<nodePair>; 
using sharedNodePairSet = std::shared_ptr<nodePairSet>;
using equivalentClasses = std::map<nodePair, sharedNodePairSet>;
using equivalentClassesAssignement = std::map<nodePair, int>;

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

    print_map("sync: ", sync); 
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

    for(auto [key, value] : eq){
        if(eqAs[key] == -1){
            const auto u = key.first; 
            const auto w = key.second; 
            std::pair key_inverse(w,u);
            eqAs[key] = 1; 
            eqAs[key_inverse]=0;
        }
    }

    return eqAs; 

}
bool planarityCheck(equivalentClassesAssignement eqAs, equivalentClasses eq){
    auto eqTrue = fillEquivalentClasses(eq);
    for(auto [key, value] : eq){
        if(eqAs[key] != eqTrue[key] ){
            return false;
        }
    }
    return true;
}


//TODO time difference enabling recuceEq.
equivalentClasses reduceEquivalentClasses(std::vector<std::vector<ogdf::NodeElement*>>& emb, const equivalentClasses& eqOrg){
    equivalentClasses eq = eqOrg; 
    for(const auto& level : emb){
        for(const auto& v : level){
            std::cout << "*****************We are in vertex : " << v->index() << std::endl;
            std::vector<int> adjIn, adjOut;
            std::map<int,int> orderIn, orderOut; 
            for(const auto& adj : v->adjEntries){
                edge e = adj->theEdge(); 
                if(v->index() == e->source()->index()){
                    adjOut.push_back(e->target()->index());  
                } else {
                    adjIn.push_back(e->source()->index());
                }
            }
            int counter = 0;
            sort(adjIn.begin(), adjIn.end());
            sort(adjOut.begin(), adjOut.end());
            for(const auto i : adjIn){
                orderIn[i] = counter++;
            }
            counter = 0;
            for(const auto i : adjOut){
                orderOut[i] = counter++;
            }
            std::cout << ">> before reorder" << std::endl;
            print_mapint("orderOut", orderOut); 
            print_mapint("orderIn", orderIn); 
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
            std::cout << ">> after reorder" << std::endl;
            print_mapint("orderOut", orderOut); 
            print_mapint("orderIn", orderIn); 

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
                            eq[paar_inverse]->insert(std::make_pair(u,w));
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
                            eq[paar_inverse]->insert(std::make_pair(u,w));
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
        }
    }
    return eq;
}
int main(){


    GraphBuilder graphBuild; 
    auto emb = graphBuild.buildLevelGraphFromGML("counterexample.gml");

    equivalentClasses eq = compute2SATClasses(emb);

    print_map("equivalence classes : ", eq );

    ogdf::GraphIO::write(graphBuild.GA, "output-acyclic-graph.gml", GraphIO::writeGML);
    GraphIO::write(graphBuild.GA, "output-acyclic-graph.svg", GraphIO::drawSVG);

    equivalentClasses eqReduced = reduceEquivalentClasses(emb, eq);
    equivalentClassesAssignement assignement = fillEquivalentClasses(eqReduced);
    bool result = planarityCheck(assignement, eq);
    print_map("equivalence classes reduced : ", eqReduced );

    std::cout << "the planarity check : " << result << std::endl; 


    return 0;
}
