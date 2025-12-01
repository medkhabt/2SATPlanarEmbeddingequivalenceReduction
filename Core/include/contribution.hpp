#pragma once
#include "GraphBuilder.h"
#include "type.hpp"
#include "string"
#ifdef BUILD_PROFILING 
#include <tracy/Tracy.hpp>
#endif

#include "2SatCompute.hpp"
#include "utils.hpp"

enum class PROFILING {
    ENABLE,
    DISABLE
};
enum class GENERATING_OUTPUT {
    ENABLE,
    DISABLE
};
enum class DEBUGING {
    ENABLE,
    DISABLE
};
class Contribution {
    protected:
        std::string algName;
        int** finalOrdering = nullptr; 
        bool isTotalOrder = false;
    public : 
        std::ofstream logTimeFile; 
        PROFILING profiling; 
        GENERATING_OUTPUT canGenerate; 
        DEBUGING debug;
        Contribution(PROFILING profiling = PROFILING::DISABLE, GENERATING_OUTPUT canGenerate = GENERATING_OUTPUT::DISABLE, DEBUGING debug = DEBUGING::DISABLE): profiling(profiling), canGenerate(canGenerate), debug(debug){}
        //TODO free in the deconstructor
        virtual ~Contribution() = default;
        virtual int enforceTransitivity(GraphBuilder& builder, equivalenceClasses& eq) = 0;
        int process(std::string title, GraphBuilder& graphBuild, std::pair<std::pair<int, int>, std::pair<int, int>>& counter){
            int s = -1; 
            if(canGenerate == GENERATING_OUTPUT::ENABLE){
                std::cout << "enabled !! " << std::endl;
            } else if (canGenerate == GENERATING_OUTPUT::DISABLE){
                std::cout << "disabled!! " << std::endl;
            } else {
                std::cout << "wrong state" << std::endl; 
            }
#ifdef BUILD_PROFILING 
            ZoneScopedN("process"); 
#endif 
            {
#ifdef BUILD_PROFILING 
                ZoneScopedN("image and gml creation"); 
#endif 
                if(!graphBuild.validGraph){
                    std::cout << "The Graph is not valid." << std::endl;
                    return 1; 
                }
                if(canGenerate == GENERATING_OUTPUT::ENABLE){
                    if(profiling == PROFILING::ENABLE){
                        //ogdf::GraphIO::write(graphBuild.GA, "../graphs/inputs/svg/"+ title + ".svg", ogdf::GraphIO::drawSVG);
                        //ogdf::GraphIO::write(graphBuild.CG, "../graphs/inputs/gml/"+ title + ".gml", ogdf::GraphIO::writeGML);
                    }else {
                        std::cout << "Are we here ? with title: " << title << std::endl;
                        ogdf::GraphIO::write(*graphBuild.GA, "graphs/inputs/svg/"+ title + ".svg", ogdf::GraphIO::drawSVG);
                        ogdf::GraphIO::write(*graphBuild.CG, "graphs/inputs/gml/"+ title + ".gml", ogdf::GraphIO::writeGML);
                    }

                }
            }
            //std::ofstream logFile; 
            if(debug == DEBUGING::ENABLE){
                if(profiling == PROFILING::ENABLE){
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
                    return 1;
                }
            }
            int nodesSize = graphBuild.G->numberOfNodes();
            {

#ifdef BUILD_PROFILING 
                ZoneScopedN("contribution"); 
#endif 
                auto start = std::chrono::high_resolution_clock::now();
                s = this->enforceTransitivity(graphBuild, eq);
                auto stop = std::chrono::high_resolution_clock::now();
                auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);

                std::cout << "Elapsed Time for contribution: " << duration.count() << " ms\n";
            }

            {
#ifdef BUILD_PROFILING 
                ZoneScopedN("get emb and test"); 
#endif 
                auto start = std::chrono::high_resolution_clock::now();
                bool test = testEmbedding(graphBuild, eq, title + "_" + this->algName, (canGenerate == GENERATING_OUTPUT::ENABLE), this->finalOrdering, this->isTotalOrder);
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
            return s;
        }
};
