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
        compute2SATClasses(graphBuild, eq);
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
        bool test = testEmbedding(graphBuild, eq, title, canGenerate);
        auto stop = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);
        std::cout << "Elapsed Time for test: " << duration.count() << " ms\n";

    }
    int size = graphBuild.emb.size();
    for(size_t key = 0; key < size; key++){
        free(eq.pairId[key]);
        free(eq.pairIdLocalIndex[key]);
        free(eq.pairIdLocalIndexInverse[key]);
    }

    free(eq.pairIdArraySize); 
    free(eq.pairIdLocalIndex);
    free(eq.pairIdLocalIndexInverse); 
    free(eq.pairIdOffset);
    free(eq.pairId);
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

    std::unique_ptr<Contribution> contrib; 

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
        } else if((arg == "-g" || arg == "--generate-svg")){
            canGenerate = true; 
        } else if((arg == "-a" || arg == "--algorithm" && i + 1 < argc)){
            std::string alg = argv[i+1]; 
            if(alg == "greedy-arbitrary" || alg == "ga"){
                contrib = std::make_unique<ContributionSimpleGreedyApproach>(PROFILING::DISABLE, GENERATING_OUTPUT::ENABLE, DEBUGING::DISABLE);
            } else if(alg == "greedy-with-sort" || alg == "gws") {
                contrib = std::make_unique<ContributionSimpleGreedyWithSortApproach>(PROFILING::DISABLE, GENERATING_OUTPUT::ENABLE, DEBUGING::DISABLE);
            } else {
                std::cerr << "you chose an unkown algorithm. Check the help (-h|--help) for available algorithms" << std::endl;; 
                return 1;
            }
        } else if((arg == "-h" || arg == "--help")){
            std::cout << " ./2SATEquivalenceReduction [-f|--file (GML_FILE) | [-r|--random (hiearchy|custom|maximal)] [-n|--nodes NUMBER_NODES] [-l|--levels NUMBER_LEVELS]] [-a | --algorithm (ALGORITHM)] [-p | --profiling] [-g | --generate-svg] " << std::endl;
            std::cout << "OPTIONS:" << std::endl;
            std::cout << " - ALGORITHM: the algorithms to choose from are:  "<< std::endl;
            std::cout << "    - greedy-arbitrary (ga)" << std::endl;
            std::cout << "    - greedy-with-sort (gws)" << std::endl;
            return 0;
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
    
    ContributionSimpleGreedyApproach algv1(PROFILING::DISABLE, GENERATING_OUTPUT::ENABLE, DEBUGING::DISABLE); 
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
        {
#ifdef BUILD_PROFILING 
            ZoneScopedN("graphBuilder"); 
#endif
            graphBuild.buildRandomLevelGraph(max_nodes, max_levels);
        }
        logTimeFile << ""<< max_levels << " " << max_nodes << " " ;
        logResult << max_levels <<  " " << max_nodes << " " ; 
        contrib->process(mode + "/" + "v_" + std::to_string(max_nodes) + "_l_" + std::to_string(max_levels), graphBuild, counter);
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
        contrib->process(p.stem(), graphBuild, counter);
    }

    logTimeFile.close();
    return 0;
}

