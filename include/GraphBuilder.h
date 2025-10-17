#pragma once
#include <ogdf/basic/basic.h>
#include <ogdf/basic/Graph_d.h>
#include <ogdf/basic/GraphAttributes.h>
#include <ogdf/basic/simple_graph_alg.h>
#include <ogdf/basic/extended_graph_alg.h>
#include <ogdf/basic/Graph.h>
#include <ogdf/fileformats/GraphIO.h>
#include <ogdf/basic/graph_generators/randomized.h>
#include <ogdf/basic/GraphCopy.h>
#include <random>
#include <unordered_set>

#ifdef BUILD_PROFILING
#include <tracy/Tracy.hpp>
#endif

#include <utility>
#include <functional>

#include "NodePartition.h"
#include "GraphWithPairNode.hpp"

int randomNumberNormalDistribution(int low, int high);
class BEdge {
    public:
        int head, tail, id, pos;
        BEdge* next;

        BEdge(int t, int h, int c) : head(h), tail(t), id(c), pos(-1), next(nullptr) { }

        OGDF_NEW_DELETE
};

using bEdge = BEdge*;

namespace ogdf{
    OGDF_DECLARE_COMPARER(CmpTail, bEdge, int, x->tail);
    OGDF_DECLARE_COMPARER(CmpHead, bEdge, int, x->head);
}

class GraphBuilder{
    public:
        ogdf::Graph G; 
        ogdf::GraphAttributes GA; 
        ogdf::GraphAttributes GANew; 
        ogdf::NodePartition LVL; 
        std::vector<std::vector<ogdf::node>> emb;
        ogdf::ClusterGraph CG; 
        bool validGraph; 
        GraphBuilder() : LVL(G), GA(G, ogdf::GraphAttributes::all), CG(G), validGraph(true){

        }

        void randomHierarchy(ogdf::Graph& G, int numberOfNodes, int numberOfEdges, bool planar, bool singleSource, bool longEdges, ogdf::Array<ogdf::node>& vertexArray, ogdf::Array<int>& levelsArray, int& numberOfLayers);
        void randomPlanarLevelProperGraphFromOGDFRandomHiearchy(int N);
        void randomLevelGraph(int N, int K, std::function<int(int,int)> randomazier);

        void drawLevelGraph(int scaleX=50, int scaleY=50){
            size_t maxlvl = 0; 
            for(const auto& level: this->emb) {
                if(level.size() > maxlvl){
                    maxlvl = level.size(); 
                } 
            }
            size_t y = 0, x = 0;

            for(const auto& level: this->emb){
                float offs = (maxlvl - level.size())* scaleX / 2;  
                x = 0;
                for(const auto& node: level){
                    this->GA.x(node) =  x * scaleX + offs;
                    this->GA.y(node) =  y * scaleY;
                    x++;
                }
                y++;
            }
            /*
               for(const auto& e : G.edges){
               this->GA.strokeColor(e) = ogdf::Color(ogdf::Color::Name::Green); 
               }
               */

        }
        void drawLevelGraph(ogdf::NodeArray<int>& ordering, std::map<int,ogdf::node>& id_nodes,  int scaleX=50, int scaleY=50){
            size_t maxlvl = 0; 
            for(const auto& level: this->emb) {
                if(level.size() > maxlvl){
                    maxlvl = level.size(); 
                } 
            }
            size_t y = 0, x = 0;
            boost::container::flat_map<int, ogdf::node> level_ordering;
            for(const auto& level: this->emb){
                level_ordering.clear();
                int min = this->G.numberOfNodes(); 
                //std::cout <<  "number of nodes are : " << min << std::endl; 
                bool singleNode = false; 
                int counter = 0;
                for(const auto& node: level){
                    if(ordering[level_index][node->index()] > -1){
                        counter++;
                        int order = ordering[level_index][node->index()];
                        level_ordering[order] = node; 
                        //std::cout << "found " << node->index() << " gave it the order " << order << std::endl; 
                        if(order < min ) {
                            min = order;
                        }

                    } else {
                        singleNode = true; 
                    }
                } 
                if(counter==1){
                    singleNode = true;  
                }
                level_index++;
                float offs = (maxlvl - level.size())* scaleX / 2;  
                x = 0;
                if(singleNode){
                    for(const auto& node: level){
                        this->GA.x(node) =  x * scaleX + offs;
                        this->GA.y(node) =  y * scaleY;
                        x++;
                    }
                } else {
                    for(int i = min; i < min + level.size(); i++){
                        //std::cout << "level ordering i : " << i << ": vertex index " << level_ordering[i]->index() << std::endl; 
                        this->GA.x(level_ordering[i]) = x * scaleX + offs; 
                        this->GA.y(level_ordering[i]) =  y * scaleY;
                        x++;  
                    }
                }
                /*
                   for(const auto& node: level){
                   this->GA.x(node) =  x * scaleX + offs;
                   this->GA.y(node) =  y * scaleY;
                   x++;
                   }
                   */
                y++;
            }
            postTraitement();
        }
        void pruneEdges(ogdf::Graph& G, int max_edges, int min_deg) {
            std::vector<ogdf::edge> edges;
            {
#ifdef BUILD_PROFILING 
                ZoneScopedN("save edges"); 
#endif 
                for (ogdf::edge e : G.edges) {
                    edges.push_back(e);
                }
            }
            {
#ifdef BUILD_PROFILING 
                ZoneScopedN("shuffle"); 
#endif 
                std::mt19937 mt(ogdf::randomSeed());
                shuffle(edges.begin(), edges.end(), mt);
            }
            bool exausted = true;
            for (ogdf::edge e : edges) {
#ifdef BUILD_PROFILING 
                ZoneScopedN("loopdel"); 
#endif 
                if (e->source()->degree() > min_deg && e->target()->degree() > min_deg) {
#ifdef BUILD_PROFILING 
                    ZoneScopedN("del_edge"); 
#endif 
                    G.delEdge(e);
                    {
#ifdef BUILD_PROFILING 
                        ZoneScopedN("test_connected "); 
#endif 
                        if(!ogdf::isConnected(G)){
                            G.newEdge(e->source(), e->target());
                        }
                        if (G.numberOfEdges() <= max_edges || (iteration_max > -1 && counter > iteration_max) ) {
                            exausted = false; 
                            break;
                        }
                        //std::cout << "counter: " << counter <<  ", iteration_max : " << iteration_max << std::endl; 
                        counter++;
                    }
                }
            }
            std::string exausted_str;
            if(exausted){
                exausted_str = "YES"; 
            } else {
                exausted_str = "NO"; 
            }
            //std::cout << "GraphBuilder.h::pruneEdges ::: Target num of edges: " << max_edges << ", Result num of edges: " << G.numberOfEdges() << ", All edges exausthed : "  << exausted_str << std::endl; 
        }
        void postTraitement(){
            const ogdf::NodeArray<int> lvl = this->LVL.cellAssignment();
            for(const auto& e: this->G.edges){
                if(lvl[e->source()] > lvl[e->target()]){
                    G.reverseEdge(e);
                }
            }

            for(const auto& n : G.nodes){
                GA.label(n) = std::to_string(n->index()); 
            }

        }
        /*
        def to_cluster(G, LVL):
    CG = ogdf.ClusterGraph(G)
    for ns in LVL.cells():
        c = CG.newCluster(CG.rootCluster())
        for n in ns:
            CG.reassignNode(n, c)
    return CG

def to_cluster_attrs(GA, LVL):
    CG = to_cluster(GA.constGraph(), LVL)
    CGA = ogdf.ClusterGraphAttributes(CG, GA.attributes())
    ogdf.GraphAttributes.__assign__(CGA, GA)
    return CG, CGA
    */
        void toCluster(){
            //std::cout << "*************** START OF GENERATING CLUSTERS ***************** "<< std::endl;
            for (auto& level : this->emb){
                auto c = this->CG.newCluster(this->CG.rootCluster()); 
                //std::cout << ">>>>> create new cluster: "  << c->index() << std::endl;
                for(auto& n : level){
                    this->CG.reassignNode(n,c);
                    //std::cout << ">>>>>>>>>> reasign " << n->index() << " to cluster " << c->index() << std::endl;
                }
            }
            //std::cout << "*************** END OF GENERATING CLUSTERS ***************** "<< std::endl;
        } 
        ogdf::ClusterGraphAttributes toClusterAttrs(){
            auto CGA = ogdf::ClusterGraphAttributes(this->CG, this->GA.attributes());
            return CGA;
        }
        void from_cluster(){

            assert(this->CG.rootCluster()->nodes.size() == 0);
            int i = 0;
            for(auto c :this->CG.rootCluster()->children){
#ifdef BUILD_PROFILING 
                ZoneScopedN("loop");  
#endif 
                assert(c->children.size() == 0);
                if (i >= this->LVL.size()) {
                    this->LVL.newCell(); 
                }
                for(auto n : c->nodes){
                    //std::cout << "add node : " << n->index() << std::endl; 
#ifdef BUILD_PROFILING 
                    ZoneScopedN("oop second");  
#endif 
                    LVL.addToCell(n,i);
                }
                i++;
            }
        }

        void buildLevelGraphFromGML(std::string fileName){

            ogdf::ClusterGraphAttributes CGA(this->CG, ogdf::ClusterGraphAttributes::all);
            {
#ifdef BUILD_PROFILING 
                ZoneScopedN("read");  
#endif 
                //std::cout << "read" << std::endl;
                ogdf::GraphIO::read(CGA, this->CG, this->G, fileName); 
            }
            this->GA = CGA;

            {
#ifdef BUILD_PROFILING 
                ZoneScopedN("from cluster");  
#endif 
                //std::cout << "from cluster" << std::endl;
                from_cluster();
            }
            this->validGraph = ogdf::isConnected(this->G);
            {
#ifdef BUILD_PROFILING 
                ZoneScopedN("cells"); 
#endif 
                //std::cout << "cells" << std::endl;
                this->emb = LVL.cells();
            }
            {

#ifdef BUILD_PROFILING 
                ZoneScopedN("draw");  
#endif 
                //std::cout << "draw" << std::endl;
                drawLevelGraph(50, 100);
            }
            {

#ifdef BUILD_PROFILING 
                ZoneScopedN("post");  
#endif 
                //std::cout << "post" << std::endl;
                postTraitement();
            }
        }
        void buildRandomLevelGraphFromRandomProperMaximalLevelPlanarGraph(int maxNodes, int maxLevels){
            {
#ifdef BUILD_PROFILING 
                ZoneScopedN("randomProperMaximal"); 
#endif 
                ogdf::randomProperMaximalLevelPlaneGraph(this->G, this->emb, maxNodes, maxLevels, false); 
            }
            {
#ifdef BUILD_PROFILING 
                ZoneScopedN("pruneEdge"); 
#endif 
                float reduction = 1.00;
                int iteration_max = (this->G.numberOfNodes() > 1000) ? 1000 : this->G.numberOfNodes(); 
                this->pruneEdges(this->G, this->G.numberOfEdges() * 0.70, 2, iteration_max); 
            }
            {
#ifdef BUILD_PROFILING 
                ZoneScopedN("drawLevelGraph"); 
#endif 
                drawLevelGraph(50, 100);
            }
            {
#ifdef BUILD_PROFILING 
                ZoneScopedN("tocluster"); 
#endif 
                this->toCluster(); 
            }
            {
#ifdef BUILD_PROFILING 
                ZoneScopedN("clustergraphatt"); 
#endif 
                ogdf::ClusterGraphAttributes CGA(this->CG, ogdf::ClusterGraphAttributes::all);
            }
            postTraitement();
        }
        static inline uint64_t key(int a, int b) {
            if (a > b) std::swap(a,b);              
            return (uint64_t)(uint32_t)a << 32 | (uint32_t)b;
        }
        void buildRandomLevelGraph(int nNodes, int nLevels){
            randomLevelGraph(nNodes, nLevels, randomNumberNormalDistribution);
        } 
};

