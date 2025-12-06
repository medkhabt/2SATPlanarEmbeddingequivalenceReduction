#pragma once
#include <ogdf/basic/Graph_d.h>
#include <ogdf/basic/GraphAttributes.h>
#include <ogdf/basic/simple_graph_alg.h>
#include <ogdf/basic/Graph.h>
#include <ogdf/fileformats/GraphIO.h>
#include <ogdf/basic/graph_generators/randomized.h>
#include <ogdf/basic/GraphCopy.h>
#include <random>
#include <unordered_set>

#ifdef BUILD_PROFILING 
#include <tracy/Tracy.hpp>
#endif

#include "type.hpp"
#include <utility>

#include "NodePartition.h"


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
        std::unique_ptr<ogdf::Graph> G; 
        std::unique_ptr<ogdf::GraphAttributes> GA; 
        std::unique_ptr<ogdf::GraphAttributes> GANew; 
        std::unique_ptr<ogdf::NodePartition> LVL; 
        std::vector<std::vector<ogdf::node>> emb;
        std::unique_ptr<ogdf::ClusterGraph> CG; 
        bool enableNodePartition; 
        bool validGraph; 
        GraphBuilder() :
            G(std::make_unique<ogdf::Graph>()),
            LVL(std::make_unique<ogdf::NodePartition>(*G)), 
            GA(std::make_unique<ogdf::GraphAttributes>(*G, ogdf::GraphAttributes::all)),
            CG(std::make_unique<ogdf::ClusterGraph>(*G)), validGraph(true), enableNodePartition(true){}
        GraphBuilder(bool enableNodePartition) :
            G(std::make_unique<ogdf::Graph>()),
            GA(std::make_unique<ogdf::GraphAttributes>(*G, ogdf::GraphAttributes::all)),
            CG(std::make_unique<ogdf::ClusterGraph>(*G)), validGraph(true), enableNodePartition(enableNodePartition){
                if(enableNodePartition){
                    this->LVL = std::make_unique<ogdf::NodePartition>(*G); 
                } else {
                    this->LVL = std::make_unique<ogdf::NodePartition>();
                }
            }
        ~GraphBuilder() = default;
        GraphBuilder(GraphBuilder&&) noexcept = default;
        GraphBuilder& operator=(GraphBuilder&&) noexcept = default;

        GraphBuilder(const GraphBuilder&) = delete; 
        GraphBuilder& operator = (const GraphBuilder&) = delete;

        //GraphBuilder(ogdf::Graph& g) : LVL(G), GA(G, ogdf::GraphAttributes::all), CG(G), validGraph(true){this->G = g;}
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
                    this->GA->x(node) =  x * scaleX + offs;
                    this->GA->y(node) =  y * scaleY;
                    x++;
                }
                y++;
            }

        }
        void drawLevelGraph2(int scaleX=50, int scaleY=100){
            int counter = 0;
            int t = 1;
            for(ogdf::node v : this->G->nodes){
                if(counter%2==0)
                    t = t * -1;
                //this->GA.x(v) = t * counter * scaleX ; 
                this->GA->x(v) =  counter * scaleX ; 
                //this->GA.y(v) = (counter%2 ? 1 : -1) * counter * scaleY ; 
                this->GA->y(v) =  scaleY ; 
                std::cout << "index of v is : " << v->index() << std::endl;
                this->GA->label(v) = std::to_string(v->index()); 
                counter++;
            } 
            for(ogdf::edge e : this->G->edges){
                ogdf::DPolyline &p = this->GA->bends(e);
                //double xCoor = (this->GA.x(e->source()) + this->GA.x(e->target())) / 4;
                //double yCoor = (this->GA.y(e->source()) + this->GA.y(e->target())) / 4;
                int rand1=  randomNumberNormalDistribution(-400, -100);
                int rand2 =  randomNumberNormalDistribution(100, 400);
                int randHeight = randomNumberNormalDistribution(0, 1) ? rand1: rand2;
                bool srcBiggerthenTarget = (this->GA->x(e->source()) > this->GA->x(e->target())); 
                if(srcBiggerthenTarget){
                    double xCoor = this->GA->x(e->source()) -  (this->GA->x(e->source()) - this->GA->x(e->target())) / 4;
                    double yCoor = randHeight / 2; 
                    p.pushBack(ogdf::DPoint(xCoor, yCoor));
                    xCoor = this->GA->x(e->source()) - 2*(this->GA->x(e->source()) - this->GA->x(e->target())) / 4;
                    yCoor = randHeight; 
                    p.pushBack(ogdf::DPoint(xCoor, yCoor));
                    xCoor = this->GA->x(e->source()) - 3 * (this->GA->x(e->source()) - this->GA->x(e->target())) / 4;
                    yCoor = randHeight / 2; 
                    p.pushBack(ogdf::DPoint(xCoor, yCoor));
                } else {
                    double xCoor = this->GA->x(e->source()) + (this->GA->x(e->target()) - this->GA->x(e->source())) / 4;
                    double yCoor = randHeight / 2; 
                    p.pushBack(ogdf::DPoint(xCoor, yCoor));
                    xCoor = this->GA->x(e->source()) + 2*(this->GA->x(e->target()) - this->GA->x(e->source())) / 4;
                    yCoor = randHeight; 
                    p.pushBack(ogdf::DPoint(xCoor, yCoor));
                    xCoor = this->GA->x(e->source()) + 3*(this->GA->x(e->target()) - this->GA->x(e->source())) / 4;
                    yCoor = randHeight / 2; 
                    p.pushBack(ogdf::DPoint(xCoor, yCoor));
                }
                p.normalize();
                //this->GA.strokeColor(e) = ogdf::Color((ogdf::Color::Name) randomNumberNormalDistribution(0, 144)); 
                this->GA->strokeColor(e) = ogdf::Color(randomNumberNormalDistribution(0,255), randomNumberNormalDistribution(0,255), randomNumberNormalDistribution(0,255), 255); 
            }
        }
        void drawLevelGraph(int** const ordering, int scaleX=50, int scaleY=50){
            size_t maxlvl = 0; 
            for(const auto& level: this->emb) {
                if(level.size() > maxlvl){
                    maxlvl = level.size(); 
                } 
            }
            size_t y = 0, x = 0;
            boost::container::flat_map<int, ogdf::node> level_ordering;
            int level_index = 0; 
            for(const auto& level: this->emb){
                level_ordering.clear();
                int min = this->G->numberOfNodes(); 
                bool singleNode = false; 
                int counter = 0;
                for(const auto& node: level){
                    if(ordering[level_index][node->index()] > -1){
                        counter++;
                        int order = ordering[level_index][node->index()];
                        level_ordering[order] = node; 
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
                        this->GA->x(node) =  x * scaleX + offs;
                        this->GA->y(node) =  y * scaleY;
                        x++;
                    }
                } else {
                    for(int i = min; i < min + level.size(); i++){
                        this->GA->x(level_ordering[i]) = x * scaleX + offs; 
                        this->GA->y(level_ordering[i]) =  y * scaleY;
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
        void pruneEdges(ogdf::Graph& G, int max_edges, int min_deg, int iteration_max = -1) {
            int counter = 0;
            std::cout << "GraphBuilder.h::pruneEdges ::: Start num of edges: " << G.numberOfEdges() << std::endl;
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
            std::cout << "GraphBuilder.h::pruneEdges ::: Target num of edges: " << max_edges << ", Result num of edges: " << G.numberOfEdges() << ", All edges exausthed : "  << exausted_str << std::endl; 
        }
        void postTraitement(){
            const ogdf::NodeArray<int> lvl = this->LVL->cellAssignment();
            for(const auto& e: this->G->edges){
                if(lvl[e->source()] > lvl[e->target()]){
                    this->G->reverseEdge(e);
                }
            }

            for(const auto& n : this->G->nodes){
                this->GA->label(n) = std::to_string(n->index()); 
            }

        }
        void toCluster(){
            for (auto& level : this->emb){
                auto c = this->CG->newCluster(this->CG->rootCluster()); 
                for(auto& n : level){
                    this->CG->reassignNode(n,c);
                }
            }
        } 
        ogdf::ClusterGraphAttributes toClusterAttrs(){
            auto CGA = ogdf::ClusterGraphAttributes(*this->CG, this->GA->attributes());
            return CGA;
        }
        void from_cluster(){

            assert(this->CG->rootCluster()->nodes.size() == 0);
            int i = 0;
            this->LVL->clear();
            for(auto c :this->CG->rootCluster()->children){
#ifdef BUILD_PROFILING 
                ZoneScopedN("loop");  
#endif
                assert(c->children.size() == 0);
                //std::cout << "**************" << std::endl;
                //std::cout << "cluster number : "  << i << std::endl;
                //std::cout << "lvl size: "  << LVL.size() << std::endl;
                if (i >= this->LVL->size()) {
                    //std::cout << "create new cell " << std::endl; 
                    this->LVL->newCell(); 
                }
                for(auto n : c->nodes){
                    //std::cout << "add node : " << n->index() << std::endl; 
#ifdef BUILD_PROFILING 
                    ZoneScopedN("oop second");  
#endif
                    this->LVL->addToCell(n,i);
                }
                i++;
            }
        }

        void buildLevelGraphFromGML(std::string fileName){
            ogdf::ClusterGraphAttributes CGA(*this->CG, ogdf::ClusterGraphAttributes::all);
            {
#ifdef BUILD_PROFILING 
                ZoneScopedN("read");  
#endif
                //std::cout << "read" << std::endl;
                ogdf::GraphIO::read(CGA, *this->CG, *this->G, fileName); 
            }
            *this->GA = CGA;

            {
#ifdef BUILD_PROFILING 
                ZoneScopedN("from cluster");  
#endif
                //std::cout << "from cluster" << std::endl;
                from_cluster();
            }

            {
#ifdef BUILD_PROFILING 
                ZoneScopedN("cells"); 
#endif
                //std::cout << "cells" << std::endl;
                this->emb = this->LVL->cells();
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
                ogdf::randomProperMaximalLevelPlaneGraph(*this->G, this->emb, maxNodes, maxLevels, false); 
                std::cout << "ogdf:: randomProperMaximal .. size of the new emb : " << this->emb.size() << std::endl;
            }
            {
#ifdef BUILD_PROFILING 
                ZoneScopedN("pruneEdge"); 
#endif
                float reduction = 1.00;
                int iteration_max = (this->G->numberOfNodes() > 1000) ? 1000 : this->G->numberOfNodes(); 
                this->pruneEdges(*this->G, this->G->numberOfEdges() * 0.70, 2, iteration_max); 
                std::cout << "after prune .. size of the new emb : " << this->emb.size() << std::endl;
            }
            {
#ifdef BUILD_PROFILING 
                ZoneScopedN("drawLevelGraph"); 
#endif
                drawLevelGraph(50, 100);
                std::cout << "after drawing .. size of the new emb: " << this->emb.size() << std::endl;
            }
            {
#ifdef BUILD_PROFILING 
                ZoneScopedN("tocluster"); 
#endif
                this->toCluster(); 
                std::cout << "after to Cluster.. size of the new emb: " << this->emb.size() << std::endl;
            }
            {
#ifdef BUILD_PROFILING 
                ZoneScopedN("clustergraphatt"); 
#endif
                ogdf::ClusterGraphAttributes CGA(*this->CG, ogdf::ClusterGraphAttributes::all);
            }
            postTraitement();
            std::cout << "after postTraitment.. size of the new emb: " << this->emb.size() << std::endl;
        }
        static inline uint64_t key(int a, int b) {
            if (a > b) std::swap(a,b);              
            return (uint64_t)(uint32_t)a << 32 | (uint32_t)b;
        }
        void buildRandomLevelGraph(int nNodes, int nLevels){
            randomLevelGraph(nNodes, nLevels, randomNumberNormalDistribution);
        } 
};
