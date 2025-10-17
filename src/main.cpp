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
#include <filesystem>
#include <algorithm>
#include <memory>
#include <cmath>
#include <chrono>
#include <malloc.h>
#include "NodePartition.h"
#include "GraphBuilder.h"
#include "type.hpp"
#include "utils.hpp"

#ifdef BUILD_PROFILING
#include <tracy/Tracy.hpp>
#endif 

#include "2SatCompute.hpp"
#include "algorithms/algorithmSimplev1.hpp"
#include "algorithms/algorithmSimplev2.hpp"

// the pair is a counter of passed and failed instances for each of planarity and acyclic relation check.
void process(std::string title, GraphBuilder& graphBuild, std::pair<std::pair<int, int>, std::pair<int, int>>& counter, bool debug, std::ofstream& logTimeFile, bool profiling, bool canGenerate){
#ifdef BUILD_PROFILING
    ZoneScopedN("process");
#endif
    {
#ifdef BUILD_PROFILING
        ZoneScopedN("image and gml creation");
#endif
        if(!graphBuild.validGraph){
            std::cout << "The Graph is not valid." << std::endl;
            return; 
        }
        if(canGenerate){
            if(profiling){
                //ogdf::GraphIO::write(graphBuild.GA, "../graphs/inputs/svg/"+ title + ".svg", ogdf::GraphIO::drawSVG);
                //ogdf::GraphIO::write(graphBuild.CG, "../graphs/inputs/gml/"+ title + ".gml", ogdf::GraphIO::writeGML);
            }else {
                ogdf::GraphIO::write(graphBuild.GA, "graphs/inputs/svg/"+ title + ".svg", ogdf::GraphIO::drawSVG);
                ogdf::GraphIO::write(graphBuild.CG, "graphs/inputs/gml/"+ title + ".gml", ogdf::GraphIO::writeGML);
            }

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
    {
#ifdef BUILD_PROFILING
        ZoneScopedN("compute 2 sat");
#endif
        bool isPlanar = compute2SATClasses(graphBuild, eq);
        if(!isPlanar){
            std::cout << "NOT Planar graph, can't processed with the algorithm" << std::endl;  
            return;
        }
    }
    int nodesSize = graphBuild.G.numberOfNodes();
    {

#ifdef BUILD_PROFILING
        ZoneScopedN("contribution");
#endif
        auto start = std::chrono::high_resolution_clock::now();
        //Contribution1::enforceTransitivity(graphBuild, eq);
        auto stop = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);

        std::cout << "Elapsed Time for contribution: " << duration.count() << " ms\n";
    }

    {
#ifdef BUILD_PROFILING
        ZoneScopedN("get emb and test");
#endif
        auto start = std::chrono::high_resolution_clock::now();
        bool test = testEmbedding(graphBuild, eq, title);
        auto stop = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);
        std::cout << "Elapsed Time for test: " << duration.count() << " ms\n";

    }
}


int main(int argc, char* argv[]){

#ifdef BUILD_PROFILING
    ZoneScopedN("main");
#endif
    std::string graphFile;
    graphFile = "graphs/inputs/gml/counterexample.gml";
    bool randomInput = false;
    bool profiling = false;
    int max_nodes = 40;
    int max_levels = 10; 
    bool canGenerate = false;
    enum randomMethod {
        OGDF_RANDOM_PROPER_MAXIMAL_PLANAR, 
        RANDOM,
        OGDF_RANDOM_HIERACHY_PLANAR
    };
    randomMethod randMethod;
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];

        if ((arg == "-f" || arg == "--file") && i + 1 < argc) {
            graphFile = argv[++i];
        } else if ((arg == "-r" || arg == "--random") && i + 1 < argc) {
            randomInput = true;
            std::string method = argv[i+1];
            if(method == "maximal"){
                randMethod = OGDF_RANDOM_PROPER_MAXIMAL_PLANAR;        
            } else if(method == "hiearchy") {
                randMethod = OGDF_RANDOM_HIERACHY_PLANAR;
            } else if(method == "custom"){
                randMethod = RANDOM;
            } else {
                std::cerr << "Wrong argument for --random" << std::endl; 
                return 1;
            }
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
    ContributionSimpleGreedyApproach alg(PROFILING::DISABLE, GENERATING_OUTPUT::ENABLE, DEBUGING::DISABLE); 
    ContributionSimpleGreedyWithSortApproach algv2(PROFILING::DISABLE, GENERATING_OUTPUT::ENABLE, DEBUGING::DISABLE);
    if(randomInput){
        GraphBuilder graphBuild; 
        std::string mode; 
#ifdef BUILD_PROFILING
        ZoneScopedN("graphBuilder");
#endif
        switch(randMethod){
            case OGDF_RANDOM_PROPER_MAXIMAL_PLANAR:
                mode = "PROPER_MAXIMAL_PLANAR"; 
                graphBuild.buildRandomLevelGraphFromRandomProperMaximalLevelPlanarGraph(max_nodes, max_levels);
                break;
            case RANDOM:
                mode = "BLINDLY_RANDOM"; 
                graphBuild.buildRandomLevelGraph(max_nodes, max_levels); 
                break;
            case OGDF_RANDOM_HIERACHY_PLANAR: 
                mode ="RANDOM_HIERACHY";
                graphBuild.randomPlanarLevelProperGraphFromOGDFRandomHiearchy(max_nodes);
                break;
        }
        std::cout << "Graph with nodes: " << max_nodes << " and levels: "  << max_levels << std::endl; 
        logTimeFile << ""<< max_levels << " " << max_nodes << " " ;
        logResult << max_levels <<  " " << max_nodes << " " ; 
        algv2.process(mode + "/" + "v_" + std::to_string(max_nodes) + "_l_" + std::to_string(max_levels), graphBuild, counter);
        malloc_trim(0);
    } else {
        GraphBuilder graphBuild; 
        {
#ifdef BUILD_PROFILING
            ZoneScopedN("graphbuilderfromgml"); 
#endif
            graphBuild.buildLevelGraphFromGML(graphFile);
        }
        std::filesystem::path p = graphFile;
        std::cout << " stem is : " << p.stem() << std::endl;
        algv2.process(p.stem(), graphBuild, counter);
        //process("customGraph", graphBuild, counter, false, logTimeFile, profiling, canGenerate);
    }

    logTimeFile.close();
    return 0;
}

