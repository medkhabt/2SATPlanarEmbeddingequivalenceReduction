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
        eq[std::make_pair(a ,b )] = std::make_shared<nodePairSet>(); 
        eq[std::make_pair(b ,a )] = std::make_shared<nodePairSet>(); 
        todo.clear(); 
        todo.push_back(std::make_pair(a ,b ));

        while(!todo.empty()){
            int c = todo.back().first; 
            int d = todo.back().second; 
            todo.pop_back();
            if(std::make_pair(a,b) != std::make_pair(c,d)){
                // TODO add the assert.
            }
            eq[std::make_pair(c, d)] = eq[std::make_pair(a,b)];
            eq[std::make_pair(d, c)] = eq[std::make_pair(b,a)];

            eq[std::make_pair(a, b)]->insert(sync[std::make_pair(c, d)].begin(), sync[std::make_pair(c, d)].end());
            eq[std::make_pair(b, a)]->insert(sync[std::make_pair(d, c)].begin(), sync[std::make_pair(d, c)].end());

            for(auto& pair : sync[std::make_pair(c,d)]) {
                if( eq.find(pair) == eq.end())
                    todo.push_back(std::make_pair(pair.first, pair.second));    
            }

        }
    }
    return eq; 
}

//TODO time difference enabling recuceEq.
void reduceEquivalentClasses(std::vector<std::vector<ogdf::NodeElement*>>& emb, equivalentClasses eq){
    for(const auto& level : emb){
        for(const auto& v : level){
            std::cout << "*****************8We are in vertex : " << v->index() << std::endl;
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
                        for(const auto pair : *eq[std::make_pair(u,w)]){
                            auto u1 = pair.first;
                            auto w1 = pair.second;
                            if(u1 > w1 && orderOut.find(u1)!= orderOut.end() && orderOut.find(w1) != orderOut.end()){
                                int temp = orderOut[u1];
                                orderOut[u1] = orderOut[w1];
                                orderOut[w1] = temp; 
                            }
                        }
                    } 
                }
            }
            for(const auto u:adjIn){
                for(const auto w:adjIn){
                    if(u < w && eq.find(std::make_pair(u ,w ))!= eq.end()){
                        for(const auto pair : *eq[std::make_pair(u,w)]){
                            auto u1 = pair.first;
                            auto w1 = pair.second;
                            if(u1 > w1 && orderIn.find(u1)!= orderIn.end() && orderIn.find(w1) != orderIn.end()){
                                int temp = orderIn[u1];
                                orderIn[u1] = orderIn[w1];
                                orderIn[w1] = temp; 
                            }
                        }
                    } 
                }
            }
            std::cout << ">> after reorder" << std::endl;
            print_mapint("orderOut", orderOut); 
            print_mapint("orderIn", orderIn); 
        }
    }

}
int main(){


    GraphBuilder graphBuild; 
    auto emb = graphBuild.buildLevelGraphFromGML("counterexample.gml");

    equivalentClasses eq = compute2SATClasses(emb);

    reduceEquivalentClasses(emb, eq);

    print_map("equivalence classes : ", eq );
    ogdf::GraphIO::write(graphBuild.GA, "output-acyclic-graph.gml", GraphIO::writeGML);
    GraphIO::write(graphBuild.GA, "output-acyclic-graph.svg", GraphIO::drawSVG);


    return 0;
}
