//def drawLevelGraph(GA, emb, scaleX=50, scaleY=50):
//    maxlvl = max(map(len, emb))
//    for y, lvl in enumerate(emb):
//        offs = (maxlvl - len(lvl)) * scaleX / 2
//        for x, n in enumerate(lvl):
//            GA.x[n] = x * scaleX + offs
//            GA.y[n] = y * scaleY
//drawLevelGraph(GA, emb, 50, 100)

#include <ogdf/basic/Graph_d.h>
#include <ogdf/basic/GraphAttributes.h>

void drawLevelGraph(ogdf::GraphAttributes* GA, std::vector<std::vector<ogdf::NodeElement*>> emb, int scaleX=50, int scaleY=50 ){
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
