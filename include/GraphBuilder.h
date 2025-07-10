#include <ogdf/basic/Graph_d.h>
#include <ogdf/basic/GraphAttributes.h>
#include <ogdf/basic/Graph.h>
#include <ogdf/fileformats/GraphIO.h>
#include <utility>

#include "NodePartition.h"
class GraphBuilder{
    public:
        ogdf::Graph G; 
        ogdf::GraphAttributes GA; 
        ogdf::NodePartition LVL; 
        GraphBuilder() : LVL(G), GA(G, ogdf::GraphAttributes::all){
        }

        static void drawLevelGraph(ogdf::GraphAttributes* GA, std::vector<std::vector<ogdf::NodeElement*>> emb, int scaleX=50, int scaleY=50 ){
            size_t maxlvl = 0; 
            for(const auto& level: emb) {
                if(level.size() > maxlvl){
                    maxlvl = level.size(); 
                } 
            }
            size_t y = 0, x = 0;

            for(const auto& level: emb){
                float offs = (maxlvl - level.size())* scaleX / 2;  
                x = 0;
                for(const auto& node: level){
                    GA->x(node) =  x * scaleX + offs;
                    GA->y(node) =  y * scaleY;
                    x++;
                }
                y++;
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
        void from_cluster(ogdf::ClusterGraph* CG){
            assert(CG->rootCluster()->nodes.size() == 0);
            int i = 0;
            for(auto c :CG->rootCluster()->children){
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

        std::vector<std::vector<ogdf::NodeElement*>> buildLevelGraphFromGML(std::string fileName){

            ogdf::ClusterGraph CG(this->G); 
            ogdf::ClusterGraphAttributes CGA(CG, ogdf::ClusterGraphAttributes::all);
            ogdf::GraphIO::read(CGA, CG, this->G, fileName); 
            this->GA = CGA;

            from_cluster(&CG);


            std::vector<std::vector<ogdf::NodeElement*>> emb = LVL.cells();
            drawLevelGraph(&(this->GA), emb, 50, 100);
            postTraitement();
            return std::move(emb);  
        }
};
