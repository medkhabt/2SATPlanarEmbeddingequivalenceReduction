#pragma once
#include <stdio.h>
#include <ogdf/basic/Graph.h>
#include <ogdf/basic/Graph_d.h>
#include <cassert>
#include <string>
#include <malloc.h>
#include "GraphBuilder.h"
#include "type.hpp"
#include <fstream>
#include <iostream>
#include <tracy/Tracy.hpp>

inline int localIndex(equivalenceClasses& eq, int u, int v, int level){
    int offset = eq.pairIdOffset[level];
    int size = eq.pairIdArraySize[level];
    int ulocal= eq.pairIdLocalIndex[level][u - offset];
    int vlocal= eq.pairIdLocalIndex[level][v - offset];
    return ulocal * sqrt(size/2) + vlocal; 

}
inline std::pair<int,int> localIndexInverse(equivalenceClasses& eq, int key,  int level){
    int offset = eq.pairIdOffset[level];
    int size = eq.pairIdArraySize[level];
    int ulocal = key / int(sqrt(size/2)); 
    int vlocal = key % int(sqrt(size/2)); 
    int u = eq.pairIdLocalIndexInverse[level][ulocal] + offset; 
    int v = eq.pairIdLocalIndexInverse[level][vlocal] + offset; 
    return std::pair<int,int>(u,v); 

}
inline size_t currentRSS() {
    std::ifstream f("/proc/self/statm");
    size_t size, resident;
    f >> size >> resident;
    return resident * sysconf(_SC_PAGESIZE);
}
struct MemCheckpoint {
    size_t start;
    MemCheckpoint() : start(currentRSS()) {}
    void report(const char* msg = "", bool zeroomit = false) {
        size_t now = currentRSS();
        int size = (now - start) / (1024*1024);
        if(!zeroomit || size > 0)
        std::cout << msg << " delta: " << size  << " MB\n";
    }
};
inline void compute2SATClasses(GraphBuilder& builder, equivalenceClasses& eqDs){
    ZoneScoped;
    // sync 
    auto& emb = builder.emb;
    int nodesSize = builder.G.numberOfNodes();
    // each vertex can have up to |V(G)| - 1 relations, reversing the relation result in an other |V(G)| - 1 relation per vertex.  
    // TODO be careful of the number of vertices. the constructor only accepts int.
    std::cout << "start 2sat" << std::endl;
    int maxNumberOfDijointSets = 0; 
    //MemCheckpoint c; 
    //int mem_size = 0 ;
    eqDs.pairId = (int**) calloc(emb.size(), sizeof(int*));  
    eqDs.pairIdArraySize = (int*) calloc(emb.size(), sizeof(int));
    eqDs.pairIdLocalIndex = (int**) calloc(emb.size(), sizeof(int*));
    eqDs.pairIdLocalIndexInverse = (int**) calloc(emb.size(), sizeof(int*)); 
    eqDs.pairIdOffset = (int*) calloc(emb.size(), sizeof(int));
    //mem_size += emb.size() * 5 * 4 / 1024 ;
    //std::cout << "mallocs allocates : " << mem_size << "MB"  << std::endl;
    //c.report("mallocs");
    int i = 0 ;
    //MemCheckpoint c1; 
    for(const auto& nodes: emb){
        int n = nodes.size();
        int size = 2 * n * n;
        eqDs.pairIdArraySize[i] = size;
        int max = 0;
        int min = INT_MAX;
        for(const auto& n: nodes){
            if(n->index() > max){max = n->index();}
            if(n->index() < min){min = n->index();}
        }
        eqDs.pairIdLocalIndex[i]=(int*)calloc(max - min + 1, sizeof(int)); 
        //int mem_size_1 = (max-min+1) * 4 / 1024 ; 
        //std::cout << "PairIdLocalIndex : " << mem_size_1 << " MB" << std::endl;
        //mem_size += mem_size_1;  
        eqDs.pairIdLocalIndexInverse[i]=(int*)calloc(n, sizeof(int));
        //int mem_size_2 = (n) * 4 / 1024 ;
        //std::cout << "PairIdLocalIndexInverse : " << mem_size_2 << " MB" << std::endl;
        //mem_size += mem_size_2;  
        eqDs.pairIdOffset[i]= min;
        int j = 0;
        for(int l = 0 ; l < max - min + 1; l++){
            eqDs.pairIdLocalIndex[i][l] = -1;  
        }
        for(const auto& n : nodes){
            eqDs.pairIdLocalIndex[i][n->index() - min] = j;  
            eqDs.pairIdLocalIndexInverse[i][j] = n->index() - min; 
            j++;
        }
        eqDs.pairId[i++]=(int*)calloc(size, sizeof(int));
        //int mem_size_3 = size * 4 / 1024 ;
        //std::cout << "size is : " << size << std::endl;
        //std::cout << "pairId: " << mem_size_3 << " MB" << std::endl;
        //mem_size += mem_size_3;  
        OGDF_ASSERT(1 < (INT_MAX - maxNumberOfDijointSets)/ (size)); 
        maxNumberOfDijointSets += size; 
    } 
    //c1.report("all mallocs");
    //std::cout << "all mallocs allcoates in total : " << mem_size  << "MB" << std::endl;
    // Build the disjoint set union of the equivalence classes 
    //eqDs.disjointSets = ogdf::DisjointSets(maxNumberOfDijointSets); 
    eqDs.disjointSets = ogdf::DisjointSets(maxNumberOfDijointSets / 2); 
    std::cout << "create disjointsets with " << maxNumberOfDijointSets << std::endl;
    eqDs.pairIdSize = emb.size(); 
    // TODO I need a graph registery, probably inheriting from ogdf::Graph
    // TODO It would be nice if i can implement pairnodearray
    //ogdf::PairNodeArray<int> pairNodes(builder.G);

    // Initializing the 2D array with -1 for all the pairs in the graph
    std::vector<ogdf::edge>E;
    // Initializing the 2D array with each pair in a level with a disjointed set for all levels.
    // O(l)

    for(size_t key = 0 ; key < eqDs.pairIdSize ; key++){
        for(size_t i = 0 ; i < eqDs.pairIdArraySize[key]; i++){
            eqDs.pairId[key][i]= -1;
        }
    }

    //std::cout << "filled with -1 " << std::endl;
    int l = 0 ;
    int makesets_created = 0;
    MemCheckpoint c2; 
    for(const auto& nodes : emb){
        ZoneScopedN("makeset-loop");
        //MemCheckpoint ci; 
        // O(|V|/l)
        for(const auto& u : nodes){
            //O(|V|/l) -> O(n^2)
            for(const auto& v : nodes){
                ZoneScopedN("last-loop");
                if((u->index()) < (v->index())){
                ZoneScopedN("IF");
                    int id1, id2;
                    {
                        ZoneScopedN("makeset"); 
                        id1 = eqDs.disjointSets.makeSet(); 
                        makesets_created++;
                        id2 = eqDs.disjointSets.makeSet();
                        makesets_created++;
                    }
                    {
                        ZoneScopedN("save makeset");
                        eqDs.pairId[l][localIndex(eqDs, u->index(), v->index(), l)] = id1;
                        eqDs.pairId[l][localIndex(eqDs, v->index(), u->index(), l)] = id2;
                    }
                }
            }
        }
        //std::cout << "level : " << l << std::endl;
        //ci.report("level : ");
        l++;
    }
    c2.report("makesets"); 

    //std::cout << "end creating makessets" << std::endl;
    // o(l) with children O(|E|^2/l) -> O(n^2)
    l = 0;
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
                    eqDs.disjointSets.quickUnion(eqDs.pairId[l][localIndex(eqDs, a->index(), b->index(), l)], eqDs.pairId[l + 1][localIndex(eqDs, c->index(), d->index(), l + 1)]); 
                    eqDs.disjointSets.quickUnion(eqDs.pairId[l][localIndex(eqDs, b->index(), a->index(), l)], eqDs.pairId[l + 1][localIndex(eqDs, d->index(), c->index(), l + 1)]); 
                }
            }
        }
        l++;
    }
    //std::cout << "end merging and the 2sat calc" << std::endl;
    return;
}
