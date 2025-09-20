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
#include "type.hpp"
#include "utils.hpp"
#include "Tracy.hpp"
#include "2SatCompute.hpp"
#include "algorithm.hpp"

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
    compute2SATClasses(graphBuild, eq);

    Contribution::enforceTransitivity(graphBuild, eq);
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

