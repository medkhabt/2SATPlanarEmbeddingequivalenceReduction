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
#include <chrono>
#include "NodePartition.h"
#include "GraphBuilder.h"
#include "algorithm.hpp"
#include "type.hpp"
#include "utils.hpp"
#include "Tracy.hpp"

void compute2SATClasses(std::vector<std::vector<ogdf::NodeElement*>>& emb, std::map<nodePair, sharedNodePairSet>& eq){
    ZoneScoped;
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
}

// the pair is a counter of passed and failed instances for each of planarity and acyclic relation check.
void process(std::string title, GraphBuilder& graphBuild, std::pair<std::pair<int, int>, std::pair<int, int>>& counter, bool debug, std::ofstream& logTimeFile, bool profiling){
    ZoneScopedN("process");
    {
        ZoneScopedN("image and gml creation");
        if(profiling){
            ogdf::GraphIO::write(graphBuild.GA, "../graphs/inputs/svg/"+ title + ".svg", ogdf::GraphIO::drawSVG);
            ogdf::GraphIO::write(graphBuild.CG, "../graphs/inputs/gml/"+ title + ".gml", ogdf::GraphIO::writeGML);
        }else {
            ogdf::GraphIO::write(graphBuild.GA, "graphs/inputs/svg/"+ title + ".svg", ogdf::GraphIO::drawSVG);
            ogdf::GraphIO::write(graphBuild.CG, "graphs/inputs/gml/"+ title + ".gml", ogdf::GraphIO::writeGML);
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

    equivalentClasses eq ;
    compute2SATClasses(graphBuild.emb, eq);

    std::cout << "original eq class" << std::endl;
    print_eq(eq);

    //std::cout << "***************** after" << std::endl;

    /*
    if(debug){
        logFile << std::endl; 
        logFile << "> Reducing the equivalent classes"  << std::endl;
        logFile << std::endl;
    }
    */

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
    auto start = std::chrono::steady_clock::now();
    Contribution::reduceEquivalentClasses(graphBuild.emb, eq);
    auto end = std::chrono::steady_clock::now();

    std::cout << "merged eq class" << std::endl;
    print_eq(eq);

    logTimeFile << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << std::endl;

    //logFile << std::endl;
    if(debug){
        //logFile << "> Assigning the equivalent classes" << std::endl; 
        //logFile << std::endl;
    }
    std::vector<equivalentClassesAssignement> allAssignements = fillEquivalentClasses(eq);



    //TODO make it planarity check for the entire set of possible truth assignements.
    if(planarityCheck(allAssignements, oldEq)){
        /*
        if(debug){
            logFile << std::endl;
            logFile << "> PLANARITY CHECK: PASSED" << std::endl;
        }
        */
        counter.first.first ++;
    } else {
        /*
        if(debug){
            logFile << std::endl;
            logFile << "> PLANARITY CHECK: FAILED" << std::endl;
        }
        */
        counter.first.second ++;
    }
    /*
    if(debug){
        logFile << std::endl;
    }

    */
    std::ofstream wrongAssignementsFile = std::ofstream("graphs/outputs/log/wrong_assignement_" + title + ".log");
    if(profiling){
        wrongAssignementsFile = std::ofstream("../graphs/outputs/log/wrong_assignement_" + title + ".log");
    } 
    if(!wrongAssignementsFile){
        std::cerr << "Unable to open wrong assigment file" << std::endl;
    }

    bool acyclic = AcyclicRelation(title, allAssignements, wrongAssignementsFile); 
    wrongAssignementsFile.close();
    if(acyclic == true){
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
    auto logTimeFile = std::ofstream("time.dat"); 
    if(!logTimeFile){
        std::cerr << "Unable to open log time file" << std::endl;
    }

    std::cout << std::endl; 

    std::cout << std::endl; 
    std::cout << " ******** START of the program ********** " << std::endl;
    std::vector<std::vector<ogdf::node>> emb;
    std::pair<std::pair<int, int>, std::pair<int,int>> counter; 
    if(randomInput){
        for(size_t levels = 1; levels < max_levels; levels ++){
            for(size_t nodes = 1 ; nodes < max_nodes; nodes++){
                GraphBuilder graphBuild; 
                std::cout << "Graph with nodes: " << nodes << " and levels: "  << levels << std::endl; 
                graphBuild.buildRandomLevelGraph(nodes, levels);
                logTimeFile << ""<< levels << " " << nodes << " " ;
                process("randomProperLevelGraph_v_" + std::to_string(nodes) + "_l_" + std::to_string(levels), graphBuild, counter, false, logTimeFile, profiling);
            }
        }
    } else {
        GraphBuilder graphBuild; 
        graphBuild.buildLevelGraphFromGML(graphFile);
        process("customGraph", graphBuild, counter, false, logTimeFile, profiling);
    }

    logTimeFile.close();
    std::cout << " Planarity check : " << counter.first.first << " PASSED, " << counter.first.second << " FAILED." <<std::endl;
    std::cout << " Acyclic relations check check : " << counter.second.first << " PASSED, " << counter.second.second << " FAILED." <<std::endl;



    return 0;
}

