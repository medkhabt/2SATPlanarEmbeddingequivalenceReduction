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
#include "algorithm.hpp"
#include "type.hpp"
#include "utils.hpp"


std::map<nodePair, sharedNodePairSet> compute2SATClasses(std::vector<std::vector<ogdf::NodeElement*>>emb){
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



int main(int argc, char* argv[]){

    std::string graphFile;
    if(argc < 2){
        graphFile = "graphs/inputs/gml/counterexample.gml";
    } else {
        graphFile = argv[1]; 
    }
    GraphBuilder graphBuild; 

    std::cout << std::endl; 

    std::cout << std::endl; 
    std::cout << " ******** START of the program ********** " << std::endl;
    auto emb = graphBuild.buildLevelGraphFromGML(graphFile);

    ogdf::GraphIO::write(graphBuild.GA, "graphs/inputs/svg/input_graph.svg", ogdf::GraphIO::drawSVG);

    std::cout << std::endl; 
    std::cout << "> computing the equivalent classes"  << std::endl;
    std::cout << std::endl;

    equivalentClasses eq = compute2SATClasses(emb);




    std::cout << std::endl; 
    std::cout << "> Reducing the equivalent classes"  << std::endl;
    std::cout << std::endl;

    std::cout << "******************** equivalence class: " << std::endl; 
    print_eq(eq);
    equivalentClasses oldEq; 
    oldEq = Contribution::reduceEquivalentClasses(emb, eq);
    std::cout << std::endl;

    std::cout << "******************** equivalence class: " << std::endl; 
    print_eq(oldEq);
    std::cout << "******************** reduced equivalence class: " << std::endl; 
    print_eq(eq);
    std::cout << "> Assigning the equivalent classes" << std::endl; 
    std::cout << std::endl;
    std::vector<equivalentClassesAssignement> allAssignements = fillEquivalentClasses(eq);



    //TODO make it planarity check for the entire set of possible truth assignements.
    if(planarityCheck(allAssignements, oldEq)){
        std::cout << std::endl;
        std::cout << "> PLANARITY CHECK: PASSED" << std::endl;
    } else {

        std::cout << std::endl;
        std::cout << "> PLANARITY CHECK: FAILED" << std::endl;
    }
    std::cout << std::endl;
    bool acyclic = AcyclicRelation(allAssignements); 
    if(acyclic == true){
        std::cout << std::endl;
        std::cout << "> TRANSITIVITY CHECK: PASSED (No cyclic relation)" << std::endl ;
    } else {

        std::cout << std::endl;
        std::cout << "> TRANSITIVITY CHECK: FAILED (Exists a cyclic relation)" << std::endl;;
    }
    std::cout << std::endl;

    return 0;
}

