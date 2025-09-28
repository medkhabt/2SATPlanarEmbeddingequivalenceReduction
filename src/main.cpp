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
#include <tracy/Tracy.hpp>
#include "2SatCompute.hpp"
#include "algorithm.hpp"
#include "algorithmSimplev1.hpp"

// the pair is a counter of passed and failed instances for each of planarity and acyclic relation check.
void process(std::string title, GraphBuilder& graphBuild, std::pair<std::pair<int, int>, std::pair<int, int>>& counter, bool debug, std::ofstream& logTimeFile, bool profiling){
    ZoneScopedN("process");
    {
        ZoneScopedN("image and gml creation");
        if(profiling){
            //ogdf::GraphIO::write(graphBuild.GA, "../graphs/inputs/svg/"+ title + ".svg", ogdf::GraphIO::drawSVG);
            //ogdf::GraphIO::write(graphBuild.CG, "../graphs/inputs/gml/"+ title + ".gml", ogdf::GraphIO::writeGML);
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

    equivalenceClasses eq ;
    {
        ZoneScopedN("compute 2 sat");
        compute2SATClasses(graphBuild, eq);
    }
    int nodesSize = graphBuild.G.numberOfNodes();
    {

        ZoneScopedN("contribution");
        auto start = std::chrono::high_resolution_clock::now();
        Contribution1::enforceTransitivity(graphBuild, eq);
        auto stop = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);

        std::cout << "Elapsed Time for contribution: " << duration.count() << " ms\n";
    }

    {
        ZoneScopedN("get emb and test");
        auto start = std::chrono::high_resolution_clock::now();
        bool test = testEmbedding(graphBuild, eq, title);
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

    ZoneScopedN("main");
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
        GraphBuilder graphBuild; 
        std::cout << "Graph with nodes: " << max_nodes << " and levels: "  << max_levels << std::endl; 
        {
        ZoneScopedN("graphBuilder");
        graphBuild.buildRandomLevelGraph(max_nodes, max_levels);
        }
        logTimeFile << ""<< max_levels << " " << max_nodes << " " ;
        logResult << max_levels <<  " " << max_nodes << " " ; 
        process("randomProperLevelGraph_v_" + std::to_string(max_nodes) + "_l_" + std::to_string(max_levels), graphBuild, counter, false, logTimeFile, profiling);
        malloc_trim(0);
    } else {
        GraphBuilder graphBuild; 
        {
        ZoneScopedN("graphbuilderfromgml"); 
        graphBuild.buildLevelGraphFromGML(graphFile);
        }
        process("customGraph", graphBuild, counter, false, logTimeFile, profiling);
    }

    logTimeFile.close();
    return 0;
}

