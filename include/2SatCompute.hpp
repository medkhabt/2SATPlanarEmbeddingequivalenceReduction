#pragma once
#include <stdio.h>
#include <ogdf/basic/Graph.h>
#include <ogdf/basic/Graph_d.h>
#include <cassert>
#include <string>
#include <malloc.h>
#include "GraphBuilder.h"
#include "type.hpp"
#include <tracy/Tracy.hpp>

inline void compute2SATClasses(GraphBuilder& builder, equivalenceClasses& eqDs){
    ZoneScoped;
    // sync 
    auto& emb = builder.emb;
    int nodesSize = builder.G.numberOfNodes();
    // each vertex can have up to |V(G)| - 1 relations, reversing the relation result in an other |V(G)| - 1 relation per vertex.  
    // TODO be careful of the number of vertices. the constructor only accepts int.
    int maxNumberOfDijointSets = 0; 
    for(const auto& nodes: emb){
        int n = nodes.size();
        OGDF_ASSERT(1 < (INT_MAX - maxNumberOfDijointSets)/ (2 * n * (n))); 
        maxNumberOfDijointSets += 2 * n * n; 
    } 

    // Build the disjoint set union of the equivalence classes 
    eqDs.disjointSets = ogdf::DisjointSets(maxNumberOfDijointSets); 
    eqDs.pairIdSize = builder.G.maxNodeIndex() * ( nodesSize + 1) ;
    eqDs.pairId = (int*) calloc(eqDs.pairIdSize, sizeof(int));  
    // TODO I need a graph registery, probably inheriting from ogdf::Graph
    // TODO It would be nice if i can implement pairnodearray
    //ogdf::PairNodeArray<int> pairNodes(builder.G);

    // Initializing the 2D array with -1 for all the pairs in the graph
    std::vector<ogdf::edge>E;
    // Initializing the 2D array with each pair in a level with a disjointed set for all levels.
    // O(l)

    for(size_t key = 0 ; key < eqDs.pairIdSize ; key++){
        eqDs.pairId[key] = -1;
    }

    for(const auto& nodes : emb){
        ZoneScopedN("makeset-loop");
        // O(|V|/l)
        for(const auto& u : nodes){
            //O(|V|/l) -> O(n^2)
            for(const auto& v : nodes){
                if((u->index()) < (v->index())){
                    int id1, id2;
                    {
                        ZoneScopedN("makeset");
                        id1 = eqDs.disjointSets.makeSet(); 
                        id2 = eqDs.disjointSets.makeSet();
                    }
                    {
                        ZoneScopedN("save makeset");
                        eqDs.pairId[u->index() * nodesSize + v->index()] = id1;
                        eqDs.pairId[v->index() * nodesSize + u->index()] = id2;
                    }
                }
            }
        }
    }

    // o(l) with children O(|E|^2/l) -> O(n^2)
    for(const auto& nodes : emb){
        ZoneScopedN("merge sets");
        E.clear();
        // o(|V|/l)
        for(const auto& n : nodes){
            // o(\delta(G))
            for(const auto& adj : n->adjEntries) {
                if(adj->isSource()) {
                    E.push_back(adj->theEdge());
                }
            }
        }
        ogdf::edge f, s; 
        ogdf::node a,b,c,d ; 
        // o(|E|/l)
        for(size_t i = 0 ; i < E.size() ; i++){
            //o(|E|/l) 
            for(size_t j = i + 1 ; j < E.size() ; j++){
                f = E[i];  
                s = E[j];
                a = f->source(); b = s->source();
                c = f->target(); d = s->target(); 
                if(!(a->index() == b->index() || c->index() == d->index()) ) {
                    // O(1)
                    eqDs.disjointSets.quickUnion(eqDs.pairId[a->index() * nodesSize + b->index()], eqDs.pairId[c->index() * nodesSize + d->index()]); 
                    eqDs.disjointSets.quickUnion(eqDs.pairId[b->index() * nodesSize + a->index()], eqDs.pairId[d->index() * nodesSize + c->index()]); 
                }
            }
        }
    }
    return;
}
