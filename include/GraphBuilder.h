#pragma once
#include <ogdf/basic/Graph_d.h>
#include <ogdf/basic/GraphAttributes.h>
#include <ogdf/basic/simple_graph_alg.h>
#include <ogdf/basic/Graph.h>
#include <ogdf/fileformats/GraphIO.h>
#include <ogdf/basic/graph_generators/randomized.h>
#include <ogdf/basic/GraphCopy.h>
#include <random>
#include <tracy/Tracy.hpp>

#include <utility>

#include "NodePartition.h"
#include "GraphWithPairNode.hpp"

class GraphBuilder{
    public:
        ogdf::Graph G; 
        ogdf::GraphAttributes GA; 
        ogdf::GraphAttributes GANew; 
        ogdf::NodePartition LVL; 
        std::vector<std::vector<ogdf::node>> emb;
        ogdf::ClusterGraph CG; 
        GraphBuilder() : LVL(G), GA(G, ogdf::GraphAttributes::all), CG(G){
        }

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

        }
        void drawLevelGraph(ogdf::NodeArray<int>& ordering, boost::container::flat_map<int, ogdf::node>& id_nodes,  int scaleX=50, int scaleY=50){
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
                bool singleNode = false; 
                for(const auto& node: level){
                    if(id_nodes.find(node->index()) != id_nodes.end()){
                        int order = ordering[id_nodes[node->index()]];
                        level_ordering[order] = node; 
                        if(order < min ) {
                            min = order;
                        }

                    } else {
                        singleNode = true; 
                    }
                } 
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
        void pruneEdges(ogdf::Graph& G, int max_edges, int min_deg, int iteration_max = -1) {
            std::cout << "GraphBuilder.h::pruneEdges ::: Start num of edges: " << G.numberOfEdges() << std::endl;
            std::vector<ogdf::edge> edges;
            {
                ZoneScopedN("save edges");
                for (ogdf::edge e : G.edges) {
                    edges.push_back(e);
                }
            }
            {
                ZoneScopedN("shuffle");
                std::mt19937 mt(ogdf::randomSeed());
                shuffle(edges.begin(), edges.end(), mt);
            }
            bool exausted = true;
            for (ogdf::edge e : edges) {
                ZoneScopedN("loopdel");
                if (e->source()->degree() > min_deg && e->target()->degree() > min_deg) {
                    ZoneScopedN("del_edge");
                    G.delEdge(e);
                    {
                        ZoneScopedN("test_connected ");
                        if(!ogdf::isConnected(G)){
                            G.newEdge(e->source(), e->target());
                        }
                        if (G.numberOfEdges() <= max_edges ) {
                            exausted = false; 
                            break;
                        }
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
            this->LVL.clear();
            for(auto c :this->CG.rootCluster()->children){
                ZoneScopedN("loop"); 
                assert(c->children.size() == 0);
                std::cout << "**************" << std::endl;
                std::cout << "cluster number : "  << i << std::endl;
                std::cout << "lvl size: "  << LVL.size() << std::endl;
                if (i >= this->LVL.size()) {
                    std::cout << "create new cell " << std::endl; 
                    this->LVL.newCell(); 
                }
                for(auto n : c->nodes){
                    std::cout << "add node : " << n->index() << std::endl; 
                    ZoneScopedN("oop second"); 
                    //LVL.moveToCell(n,i);
                    LVL.addToCell(n,i);
                }
                i++;
            }
        }

        void buildLevelGraphFromGML(std::string fileName){
            ogdf::ClusterGraphAttributes CGA(this->CG, ogdf::ClusterGraphAttributes::all);
            {
                ZoneScopedN("read"); 
                std::cout << "read" << std::endl;
                ogdf::GraphIO::read(CGA, this->CG, this->G, fileName); 
            }
            this->GA = CGA;

            {
                ZoneScopedN("from cluster"); 
                std::cout << "from cluster" << std::endl;
                from_cluster();
            }

            {
                ZoneScopedN("cells");
                std::cout << "cells" << std::endl;
                this->emb = LVL.cells();
            }
            {

                ZoneScopedN("draw"); 
                std::cout << "draw" << std::endl;
                drawLevelGraph(50, 100);
            }
            {

                ZoneScopedN("post"); 
                std::cout << "post" << std::endl;
                postTraitement();
            }
        }
        void buildRandomLevelGraph(int maxNodes, int maxLevels){
            {
            ZoneScopedN("randomProperMaximal");
            ogdf::randomProperMaximalLevelPlaneGraph(this->G, this->emb, maxNodes, maxLevels, false); 
            }
            {
            ZoneScopedN("pruneEdge");
            float reduction = 1.00;
            int iteration_max = (this->G.numberOfNodes() > 10000) ? this->G.numberOfNodes() * 10: this->G.numberOfNodes() * 10; 
            this->pruneEdges(this->G, this->G.numberOfEdges() * 0.70, 2, iteration_max); 
            }
            {
            ZoneScopedN("drawLevelGraph");
            drawLevelGraph(50, 100);
            }
            {
            ZoneScopedN("tocluster");
            this->toCluster(); 
            }
            {
            ZoneScopedN("clustergraphatt");
            ogdf::ClusterGraphAttributes CGA(this->CG, ogdf::ClusterGraphAttributes::all);
            }
            //ogdf::randomClusterPlanarGraph(this->G, CG, 4, 10 , 4);

            //from_cluster(&CG);

            postTraitement();
        }
};
