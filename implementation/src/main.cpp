#include <stdio.h>
#include <ogdf/basic/Graph.h>
#include <ogdf/basic/Graph_d.h>
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
#include <chrono>
#include <malloc.h>
#include "NodePartition.h"
#include "GraphBuilder.h"
//#include "algorithm.hpp"
#include "type.hpp"
#include "utils.hpp"
#include "Tracy.hpp"

equivalenceClass* compute2SATClasses(GraphBuilder& builder, equivalenceClasses& eqDs){
    ZoneScoped;
    // sync 
    auto& emb = builder.emb;
    int nodesSize = builder.G.numberOfNodes();
    // each vertex can have up to |V(G)| - 1 relations, reversing the relation result in an other |V(G)| - 1 relation per vertex.  
    // TODO be careful of the number of vertices. the constructor only accepts int.
    
    OGDF_ASSERT(nodesSize < INT_MAX / 2); 
    OGDF_ASSERT(2 * nodesSize < INT_MAX / (nodesSize - 1)); 

    int maxNumberOfDijointSets = 2 * nodesSize * (nodesSize-1); 
    // Build the disjoint set union of the equivalence classes 
    eqDs = equivalenceClasses(maxNumberOfDijointSets); 

    // TODO I need a graph registery, probably inheriting from ogdf::Graph
    // TODO It would be nice if i can implement pairnodearray
    //ogdf::PairNodeArray<int> pairNodes(builder.G);
    //TODO free it
    equivalenceClass* pairsEq = (equivalenceClass*) malloc(maxNumberOfDijointSets * sizeof(equivalenceClass));  
    for(const auto& v : builder.G.nodes){
        for(const auto& w: builder.G.nodes){
            pairsEq[v->index() * nodesSize + w->index()].value = -1; 
            pairsEq[v->index() * nodesSize + w->index()].reverseValue = -1; 
        }
    }
    // Initializing the 2D array with -1 for all the pairs in the graph
    std::vector<ogdf::edge>E;
    // Initializing the 2D array with each pair in a level with a disjointed set for all levels.
    // O(l)
    for(const auto& nodes : emb){
        // O(|V|/l)
        for(const auto& u : nodes){
            //O(|V|/l) -> O(n^2)
            for(const auto& v : nodes){
                if((u->index()) < (v->index())){
                    pairsEq[u->index() * nodesSize + v->index()].value = eqDs.makeSet();
                    pairsEq[v->index() * nodesSize + u->index()].value = eqDs.makeSet();            
                    pairsEq[u->index() * nodesSize + v->index()].reverseValue = pairsEq[v->index() * nodesSize + u->index()].value ;
                    pairsEq[v->index() * nodesSize + u->index()].reverseValue = pairsEq[u->index() * nodesSize + v->index()].value ;
                }
            }
        }
    }
/*
    std::cout << "FULL list" << std::endl;
    for(const auto& v : builder.G.nodes){
        for(const auto& w: builder.G.nodes){
            std::cout << " " << pairsEq[v->index() * nodesSize + w->index()] << " ";  
        }
        std::cout<<std::endl;
    }
    */
    // o(l) with children O(|E|^2/l) -> O(n^2)
    for(const auto& nodes : emb){
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
                    eqDs.quickUnion(pairsEq[a->index() * nodesSize + b->index()].value, pairsEq[c->index() * nodesSize + d->index()].value); 
                    pairsEq[a->index() * nodesSize + b->index()].value = eqDs.getRepresentative(pairsEq[a->index() * nodesSize + b->index()].value);   
                    pairsEq[b->index() * nodesSize + a->index()].reverseValue = eqDs.getRepresentative(pairsEq[a->index() * nodesSize + b->index()].value);   

                    pairsEq[c->index() * nodesSize + d->index()].value = eqDs.getRepresentative(pairsEq[c->index() * nodesSize + d->index()].value);   
                    pairsEq[d->index() * nodesSize + c->index()].reverseValue = eqDs.getRepresentative(pairsEq[c->index() * nodesSize + d->index()].value);   

                    eqDs.quickUnion(pairsEq[b->index() * nodesSize + a->index()].value, pairsEq[d->index() * nodesSize + c->index()].value); 
                    pairsEq[b->index() * nodesSize + a->index()].value = eqDs.getRepresentative(pairsEq[b->index() * nodesSize + a->index()].value);   
                    pairsEq[a->index() * nodesSize + b->index()].reverseValue= eqDs.getRepresentative(pairsEq[b->index() * nodesSize + a->index()].value);   

                    pairsEq[d->index() * nodesSize + c->index()].value = eqDs.getRepresentative(pairsEq[d->index() * nodesSize + c->index()].value);   
                    pairsEq[c->index() * nodesSize + d->index()].reverseValue = eqDs.getRepresentative(pairsEq[d->index() * nodesSize + c->index()].value);   
                }
            }
        }
    }

    // O(l) with children O(|V|^2/l)->O(n^2)
    for(const auto& nodes: emb){
        for(const auto& v : nodes) {
            for(const auto& w: nodes){
                if(v->index() < w->index()){
                    pairsEq[v->index() * nodesSize + w->index()].value = eqDs.getRepresentative(pairsEq[v->index() * nodesSize + w->index()].value);
                    pairsEq[w->index() * nodesSize + v->index()].reverseValue = eqDs.getRepresentative(pairsEq[v->index() * nodesSize + w->index()].value);
                    pairsEq[w->index() * nodesSize + v->index()].value = eqDs.getRepresentative(pairsEq[w->index() * nodesSize + v->index()].value);
                    pairsEq[v->index() * nodesSize + w->index()].reverseValue = eqDs.getRepresentative(pairsEq[w->index() * nodesSize + v->index()].value);
                }
            } 
        }
    }

    std::cout << "Merging" << std::endl;
    for(const auto& nodes: emb){
        for(const auto& v : nodes) {
            for(const auto& w: nodes){
                std::cout << "("<< v->index() << ","  << w->index() << ") = "  << pairsEq[v->index() * nodesSize + w->index()].value << std::endl; 
                if(v->index() != w->index()){
                    std::cout << "("<< w->index() << ","  << v->index() << ") = "  << pairsEq[w->index() * nodesSize + v->index()].value << std::endl; 
                }
            } 
        }
    }

    return pairsEq;
}

// the pair is a counter of passed and failed instances for each of planarity and acyclic relation check.
void process(std::string title, GraphBuilder& graphBuild, std::pair<std::pair<int, int>, std::pair<int, int>>& counter, bool debug, std::ofstream& logTimeFile, bool profiling){
    ZoneScopedN("process");
    {
        ZoneScopedN("image and gml creation");
        if(profiling){
            ogdf::GraphIO::write(graphBuild.GA, "../graphs/inputs/svg/"+ title + ".svg", ogdf::GraphIO::drawSVG);
            //ogdf::GraphIO::write(graphBuild.CG, "../graphs/inputs/gml/"+ title + ".gml", ogdf::GraphIO::writeGML);
        }else {
            ogdf::GraphIO::write(graphBuild.GA, "graphs/inputs/svg/"+ title + ".svg", ogdf::GraphIO::drawSVG);
            //ogdf::GraphIO::write(graphBuild.CG, "graphs/inputs/gml/"+ title + ".gml", ogdf::GraphIO::writeGML);
        }
    }

    //std::ofstream logFile; 
    if(debug){
        if(profiling){
            //logFile = std::ofstream("../graphs/outputs/log/" + title + ".log"); 
        }else {
            //logFile = std::ofstream("graphs/outputs/log/" + title + ".log"); 
        }

        /*
        if(!logFile){
            std::cerr << "Unable to open log file" << std::endl;
        }
        */
    }
/*
    if(debug){
        logFile << "*********************** Graph : " << title << std::endl;
        logFile << std::endl; 
        logFile << "> computing the equivalent classes"  << std::endl;
        logFile << std::endl;
    }
    */

    equivalenceClasses eq ;
    int* eqPairs = compute2SATClasses(graphBuild, eq);

    //Contribution::reduceEquivalentClasses(graphBuild.emb, eq);
    free(eqPairs);
    //std::cout << "original eq class" << std::endl;
    //print_eq(eq);

    //std::cout << "***************** after" << std::endl;

    /*
    if(debug){
        logFile << std::endl; 
        logFile << "> Reducing the equivalent classes"  << std::endl;
        logFile << std::endl;
    }
    */

    /*
    equivalentClasses oldEq; 
    {
        ZoneScopedN("deep copy for planarity test");
        for(const auto& [pair,sharedset]: eq){
            oldEq[pair] = std::make_shared<nodePairSet>(); 
            for(const auto& eqPair : *sharedset){
                oldEq[pair]->insert(std::pair(eqPair.first, eqPair.second));
            }
        }
    }
    */
    /*
    auto start = std::chrono::steady_clock::now();
    Contribution::reduceEquivalentClasses(graphBuild.emb, eq);
    auto end = std::chrono::steady_clock::now();
    */

    //std::cout << "merged eq class" << std::endl;
    //print_eq(eq);

    //logTimeFile << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << std::endl;

    //logFile << std::endl;
    if(debug){
        //logFile << "> Assigning the equivalent classes" << std::endl; 
        //logFile << std::endl;
    }
    //std::vector<equivalentClassesAssignement> allAssignements = fillEquivalentClasses(eq);
    //eq.clear();
    //equivalentClasses().swap(eq);


    //TODO make it planarity check for the entire set of possible truth assignements.
    //if(planarityCheck(allAssignements, oldEq)){
        /*
        if(debug){
            logFile << std::endl;
            logFile << "> PLANARITY CHECK: PASSED" << std::endl;
        }
        */
        //counter.first.first ++;
    //} else {
        /*
        if(debug){
            logFile << std::endl;
            logFile << "> PLANARITY CHECK: FAILED" << std::endl;
        }
        */
        //counter.first.second ++;
    //}
    /*
    if(debug){
        logFile << std::endl;
    }

    */
    //bool acyclic = AcyclicRelation(title, allAssignements); 
    //allAssignements.clear();
    //std::vector<equivalentClassesAssignement>().swap(allAssignements);
    /*if(acyclic == true){
        if(debug){
            //logFile << std::endl;
            //logFile << "> TRANSITIVITY CHECK: PASSED (No cyclic relation)" << std::endl ;
        }
        counter.second.first ++;
    } else {

        if(debug){
            //logFile << std::endl;
            //logFile << "> TRANSITIVITY CHECK: FAILED (Exists a cyclic relation)" << std::endl;;
        }
        counter.second.second ++;
    }
    if(debug){
        //logFile << std::endl;
        //logFile.close();
    }
    */
}


int main(int argc, char* argv[]){

    std::string graphFile;
    graphFile = "graphs/inputs/gml/counterexample.gml";
    bool randomInput = false;
    bool profiling = false;
    int max_nodes = 40;
    int max_levels = 10; 
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];

        if ((arg == "-f" || arg == "--file") && i + 1 < argc) {
            graphFile = argv[++i];
        } else if ((arg == "-r" || arg == "--random")) {
            randomInput = true;
        } else if((arg == "-n" || arg == "--nodes" && i + 1 < argc)){
            max_nodes = std::stoi(argv[++i]);  
        } else if((arg == "-l" || arg == "--levels" && i + 1 < argc)){
            max_levels = std::stoi(argv[++i]);  
        } else if((arg == "-p" || arg == "--profiling")){
            profiling = true; 
        }
    }
    auto logResult = std::ofstream("result.log" , std::ios_base::app);
    auto logTimeFile = std::ofstream("time.dat"); 

    if(!logResult){
        std::cerr << "Unable to open log result file" << std::endl;
    }
    if(!logTimeFile){
        std::cerr << "Unable to open log time file" << std::endl;
    }

    std::cout << std::endl; 

    std::cout << std::endl; 
    std::cout << " ******** START of the program ********** " << std::endl;
    std::vector<std::vector<ogdf::node>> emb;
    std::pair<std::pair<int, int>, std::pair<int,int>> counter; 
    if(randomInput){
        //for(size_t levels = 10; levels < max_levels; levels ++){
            //for(size_t nodes = 20 ; nodes < max_nodes; nodes++){
                GraphBuilder graphBuild; 
                std::cout << "Graph with nodes: " << max_nodes << " and levels: "  << max_levels << std::endl; 
                graphBuild.buildRandomLevelGraph(max_nodes, max_levels);
                logTimeFile << ""<< max_levels << " " << max_nodes << " " ;
                logResult << max_levels <<  " " << max_nodes << " " ; 
                process("randomProperLevelGraph_v_" + std::to_string(max_nodes) + "_l_" + std::to_string(max_levels), graphBuild, counter, false, logTimeFile, profiling);
                malloc_trim(0);
            //}
        //}
    } else {
        GraphBuilder graphBuild; 
        graphBuild.buildLevelGraphFromGML(graphFile);
        process("customGraph", graphBuild, counter, false, logTimeFile, profiling);
    }

    logTimeFile.close();
    std::cout << " Planarity check : " << counter.first.first << " PASSED, " << counter.first.second << " FAILED." <<std::endl;
    std::cout << " Acyclic relations check check : " << counter.second.first << " PASSED, " << counter.second.second << " FAILED." <<std::endl;
    logResult << ((counter.second.first) ? "1" : "0") << std::endl; 
    return 0;
}

