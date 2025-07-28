#include <ogdf/basic/Graph_d.h>
#include <ogdf/basic/GraphAttributes.h>
#include <ogdf/basic/simple_graph_alg.h>
#include <ogdf/basic/Graph.h>
#include <ogdf/fileformats/GraphIO.h>
#include <ogdf/basic/graph_generators/randomized.h>
#include <ogdf/basic/GraphCopy.h>
#include <random>

#include <utility>

#include "NodePartition.h"
class GraphBuilder{
    public:
        ogdf::Graph G; 
        ogdf::GraphAttributes GA; 
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
        void pruneEdges(ogdf::Graph& G, int max_edges, int min_deg) {
            std::vector<ogdf::edge> edges;
            for (ogdf::edge e : G.edges) {
                edges.push_back(e);
            }
            std::mt19937 mt(ogdf::randomSeed());
            shuffle(edges.begin(), edges.end(), mt);
            for (ogdf::edge e : edges) {
                if (e->source()->degree() > min_deg && e->target()->degree() > min_deg) {
                    G.delEdge(e);
                }
                if(!ogdf::isConnected(G)){
                    G.newEdge(e->source(), e->target());
                }
                if (G.numberOfEdges() <= max_edges) {
                    break;
                }
            }
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
                assert(c->children.size() == 0);
                if (i >= this->LVL.size()) {
                    this->LVL.newCell(); 
                }
                for(auto n : c->nodes){
                    LVL.moveToCell(n,i);
                }
                i++;
            }
        }

        void buildLevelGraphFromGML(std::string fileName){

            ogdf::ClusterGraphAttributes CGA(this->CG, ogdf::ClusterGraphAttributes::all);
            ogdf::GraphIO::read(CGA, this->CG, this->G, fileName); 
            this->GA = CGA;

            from_cluster();


            this->emb = LVL.cells();
            drawLevelGraph(50, 100);
            postTraitement();
        }
        void buildRandomLevelGraph(int maxNodes, int maxLevels){
            ogdf::randomProperMaximalLevelPlaneGraph(this->G, this->emb, maxNodes, maxLevels, false); 
            for(auto& level : this->emb){
                //std::cout << "new level" << std::endl;
                for(auto& node: level) {
                    //std::cout << "node : " << node->index() << ", "; 
                }
                //std::cout << std::endl;
            }
            float reduction = 1.00;
            this->pruneEdges(this->G, this->G.numberOfEdges() * 0.75, 0); 

            //ogdf::Graph copyG;
            /*
            do {
                std::map<int, ogdf::node> nodesmap; 
                reduction -= 0.25;
                copyG.clear(); 
                for(auto& e : this->G.edges){
                    ogdf::node src = e->source();
                    ogdf::node trg = e->target();
                    if(nodesmap.find(src->index()) == nodesmap.end()){
                        nodesmap[src->index()] = copyG.newNode(src->index());
                    }
                    if(nodesmap.find(trg->index()) == nodesmap.end()){
                        nodesmap[trg->index()] = copyG.newNode(trg->index());
                    }
                    copyG.newEdge(nodesmap[src->index()], nodesmap[trg->index()]);
                }
                ogdf::pruneEdges(copyG, copyG.numberOfEdges() * reduction, 0); 
            }while(!(ogdf::isConnected(copyG)) && reduction > 0);
           */ 
            // I can't just copy the graph with triggering an assert test.
            /*
            if(reduction > 0){
                //this->GA = ogdf::GraphAttributes(copyG, ogdf::GraphAttributes::all);
                //this->G.clear(); 
                //this->G.insert(copyG);
                this->G = copyG;
                std::map<int, ogdf::edge> edgesToDel;
                std::map<int, ogdf::node> nodesToDel;
                for(auto& e: this->G.edges){
                    edgesToDel[e->index()] = e; 
                }
                for(auto& v: this->G.nodes){
                    nodesToDel[v->index()] = v; 
                }

                for(auto& e: copyG.edges){
                    edgesToDel.erase(e->index()); 
                }
                for(auto& v: copyG.nodes){
                    nodesToDel.erase(v->index()); 
                }

                for(auto& [index, e]: edgesToDel){
                    this->G.delEdge(e); 
                }
                for(auto& [index, v]: nodesToDel){
                    this->G.delNode(v); 
                }

                for(auto& v : this->G.nodes){
                    if(this->G.numberOfNodes() > 1 && v->degree() == 0){
                        this->G.delNode(v);
                    }
                }

            }
        */

            
            /*
            if(reduction > 0){
                std::cout << "we successfully created a non max connected planar graph" << std::endl;
                ogdf::pruneEdges(this->G, this->G.numberOfEdges() * reduction, 0); 
            }
            */
           
            drawLevelGraph(50, 100);
            this->toCluster(); 
            ogdf::ClusterGraphAttributes CGA(this->CG, ogdf::ClusterGraphAttributes::all);
            //ogdf::randomClusterPlanarGraph(this->G, CG, 4, 10 , 4);

            //from_cluster(&CG);

            postTraitement();
        }
};
