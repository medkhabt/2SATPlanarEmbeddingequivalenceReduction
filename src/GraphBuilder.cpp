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
#include <algorithm>
#ifdef BUILD_PROFILING
#include <tracy/Tracy.hpp>
#endif

#include <utility>
#include <functional>

#include "NodePartition.h"
#include "GraphBuilder.h"
#include "2SatCompute.hpp"

#include <ogdf/basic/Array.h>
#include <ogdf/basic/Graph.h>
#include <ogdf/basic/GraphList.h>
#include <ogdf/basic/List.h>
#include <ogdf/basic/SList.h>
#include <ogdf/basic/basic.h>
#include <ogdf/basic/comparer.h>
#include <ogdf/basic/graph_generators/deterministic.h>
#include <ogdf/basic/graph_generators/randomized.h>
#include <ogdf/basic/memory.h>
#include <ogdf/graphalg/PlanarSeparatorModule.h>

#include <algorithm>
#include <random>
#include <vector>

using std::minstd_rand;
using std::mt19937;
using std::uniform_int_distribution;
using std::uniform_real_distribution;
int randomNumberNormalDistribution(int low, int high){

    std::random_device rd{};
    std::mt19937 gen{rd()};

    float mean = ogdf::randomNumber(low, high);

    float derivation = ogdf::randomNumber((low > 0) ? low : 1, (mean > 2) ? mean : 2);
    //std::cout << "mean is : " << mean << "and derivation is " << derivation << std::endl;
    std::normal_distribution d = std::normal_distribution<float>(mean, sqrt(derivation/(high-low)));

    auto random_int = [&d, &gen]{ return std::lround(d(gen)); };
    int result;  
    do {
        result = random_int();
    }while(result > high || result < low);

    return result; 
}
void GraphBuilder::randomHierarchy(ogdf::Graph& G, int numberOfNodes, int numberOfEdges, bool planar, bool singleSource,
        bool longEdges, ogdf::Array<ogdf::node>& vertexArray, ogdf::Array<int>& levelsArray, int& numberOfLayers) {
    ogdf::Array<ogdf::node> nnr(3 * numberOfNodes);
    ogdf::Array<int> vrt(3 * numberOfNodes);
    ogdf::Array<int> fst(numberOfNodes + 1);

    /** Place nodes **/

    emptyGraph(G, numberOfNodes);

    minstd_rand rng(ogdf::randomSeed());
    uniform_real_distribution<> dist_0_1(0.0, 1.0);

    numberOfLayers = 0;
    int totNumber = 0, realCount = 0;
    fst[0] = 0;
    for (ogdf::node v : G.nodes) {
        if (longEdges && numberOfLayers) {
            vrt[totNumber++] = 1;
        }

        nnr[totNumber] = v;
        vrt[totNumber++] = 0;
        realCount++;
        double r = dist_0_1(rng);
        if ((totNumber == 1 && singleSource) || realCount == numberOfNodes
                || r * r * numberOfNodes < 1) {
            if (longEdges && numberOfLayers) {
                vrt[totNumber++] = 1;
            }
            fst[++numberOfLayers] = totNumber;
        }
    }

    /** Determine allowed neighbours **/

    ogdf::Array<int> leftN(totNumber);
    ogdf::Array<int> rightN(totNumber);
    for (int layer = 1; layer < numberOfLayers; layer++) {
        if (planar) {
            int n1 = fst[layer - 1];
            int n2 = fst[layer];
            leftN[n2] = n1;
            while (n1 < fst[layer] && n2 < fst[layer + 1]) {
                double r = dist_0_1(rng);
                if (n1 != fst[layer] - 1
                        && (n2 == fst[layer + 1] - 1
                            || r < (double)(fst[layer] - fst[layer - 1])
                            / (double)(fst[layer + 1] - fst[layer - 1]))) {
                    n1++;
                } else {
                    rightN[n2] = n1;
                    if (++n2 < fst[layer + 1]) {
                        leftN[n2] = n1;
                    }
                }
            }
        } else {
            for (int n2 = fst[layer]; n2 < fst[layer + 1]; n2++) {
                leftN[n2] = fst[layer - 1];
                rightN[n2] = fst[layer] - 1;
            }
        }
    }

    /** Insert edges **/

    ogdf::List<bEdge> startEdges;
    ogdf::Array<ogdf::SList<bEdge>> edgeIn(totNumber);
    ogdf::Array<ogdf::SList<bEdge>> edgeOut(totNumber);

    if (numberOfLayers) {
        double x1 = numberOfEdges;
        double x2 = 0;
        for (int n2 = fst[1]; n2 < totNumber; n2++) {
            if (!vrt[n2]) {
                x2 += rightN[n2] - leftN[n2] + 1;
            }
        }

        int idc = 0;
        for (int n2 = fst[1]; n2 < totNumber; n2++) {
            if (!vrt[n2]) {
                bool connected = !singleSource;
                for (int n1 = leftN[n2]; n1 <= rightN[n2] || !connected; n1++) {
                    double r = dist_0_1(rng);
                    if (r < x1 / x2 || n1 > rightN[n2]) {
                        int next = (n1 <= rightN[n2]
                                ? n1
                                : uniform_int_distribution<>(leftN[n2], rightN[n2])(rng));
                        int act = n2;
                        bEdge nextEdge = new BEdge(next, act, idc++);
                        while (vrt[next]) {
                            act = next;
                            next = uniform_int_distribution<>(leftN[act], rightN[act])(rng);
                            edgeOut[act].pushBack(nextEdge);
                            nextEdge = new BEdge(next, act, idc++);
                            edgeIn[act].pushBack(nextEdge);
                        }
                        startEdges.pushBack(nextEdge);
                        connected = true;
                        x1 -= 1;
                    }
                    if (n1 <= rightN[n2]) {
                        x2 -= 1;
                    }
                }
            }
        }
    }

    if (planar) {
        for (int act = 0; act < totNumber; act++) {
            ogdf::CmpTail cmpTail;
            edgeIn[act].quicksort(cmpTail);
            ogdf::CmpHead cmpHead;
            edgeOut[act].quicksort(cmpHead);
        }
    }

    for (int act = 0; act < totNumber; act++) {
        for (bEdge nextEdge : edgeIn[act]) {
            nextEdge->next = edgeOut[act].popFrontRet();
        }
    }

    for (bEdge actEdge : startEdges) {
        bEdge nextEdge = actEdge;
        while (vrt[nextEdge->head]) {
            nextEdge = nextEdge->next;
        }
        G.newEdge(nnr[actEdge->tail], nnr[nextEdge->head]);
    }

    /** Clean up **/
    for (bEdge nextEdge : startEdges) {
        bEdge toDelete = nextEdge;
        while (vrt[nextEdge->head]) {
            nextEdge = nextEdge->next;
            delete toDelete;
            toDelete = nextEdge;
        }
        delete toDelete;
    }
    vertexArray = nnr;     
    levelsArray = fst;
    std::cout << "The size of nodes of the graph are : " << this->G->nodes.size() << std::endl;
}
void GraphBuilder::randomPlanarLevelProperGraphFromOGDFRandomHiearchy(int N){
    ogdf::Array<int> levelArray;  
    ogdf::Array<ogdf::node> verticesArray; 
    int numberOfLevels;
    randomHierarchy(*this->G, N, ogdf::randomNumber(N,3*N-6) , true, false, false, verticesArray, levelArray, numberOfLevels); 
    // TODO I need to build the embedding 
    int firstThisLevel = 0, firstNextLevel;
    for(int i=1; i < levelArray.size(); i++){
        //std::cout << " level " << i << " levelArray - firstthisLevel " << levelArray[i]-firstThisLevel  << std::endl;
        if(levelArray[i]-firstThisLevel <= 0){
            break;
        }
        firstNextLevel = levelArray[i];
        this->emb.emplace_back();    
        for(int j=firstThisLevel; j<firstNextLevel; j++ ){
            this->emb.back().push_back(verticesArray[j]); 
        }
        firstThisLevel = firstNextLevel;
        //std::cout << "size of the level " << i << "is : " << this->emb.back().size() << std::endl;
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
        ogdf::ClusterGraphAttributes CGA(*this->CG, ogdf::ClusterGraphAttributes::all);
    }
    postTraitement();
}
int connectedComponents(const ogdf::Graph& G, ogdf::NodeArray<int>& component,
        std::map<int, int>& compSizes) {
    int nComponent = 0;
    component.fill(-1);

    ogdf::ArrayBuffer<ogdf::node> S;

    for (ogdf::node v : G.nodes) {
        if (component[v] != -1) {
            continue;
        }

        S.push(v);
        component[v] = nComponent;
        compSizes[nComponent] = 1;

        while (!S.empty()) {
            ogdf::node w = S.popRet();
            for (ogdf::adjEntry adj : w->adjEntries) {
                ogdf::node x = adj->twinNode();
                if (component[x] == -1) {
                    component[x] = nComponent;
                    compSizes[nComponent]++;
                    S.push(x);
                }
            }
        }

        ++nComponent;
    }

    return nComponent;
}
void GraphBuilder::randomLevelGraph(int N, int K, std::function<int(int,int)> randomizer){
    OGDF_ASSERT(N > 0);
    OGDF_ASSERT(K > 0);

    // init arrays
    this->G->clear();
    emb.clear();
    emb.resize(K);

    std::unordered_set<uint64_t> seen; 
    ogdf::NodeArray<int> levelNode(*this->G); 
    //std::cout << "::: CREATION VERTEX PER LEVEL: " << std::endl;
    for (int i = 0; i < K; ++i) {
        emb[i].push_back(this->G->newNode());
        levelNode[emb[i].back()] = i;
        //std::cout << "::: ::: level " << i << " vertex created : " << emb[i][0]->index() << std::endl; 
    }
    //std::cout << "::: CREATION OF WHAT'S LEFT OF THE VERTICES" << std::endl;
    int max = 1; 
    for (int i = K; i < N; ++i) {
        //int l = ogdf::randomNumber(0, K - 1);
        int l = randomizer(0,K-2);
        emb[l].push_back(this->G->newNode());
        levelNode[emb[l].back()] = l;
        if(emb[l].size() > max ) {
            max = emb[l].size(); 
        }
        //std::cout << "::: ::: level " << l << " vertex created : " << emb[l].back()->index() << std::endl; 
    }

    //std::cout << "::: CREATION OF EDGE PER LEVEL:" << std::endl;
    int n_init_edges = 0;
    /*
       for(int i = 0; i < K; ++i){
       for(int j = 0 ; j < emb[i].size(); j++){
       if((i == 0 || !ogdf::randomNumber(0,1)) && (i < K-1)){
       int v = randomizer(0, emb[i+1].size() - 1);
       G.newEdge(emb[i][j], emb[i+1][v]); 
       seen.insert(key(emb[i][j]->index(), emb[i+1][v]->index()));
    //std::cout << "::: ::: level " << i << " edge created : (" << emb[i][j]->index() << "," << emb[i+1][v]->index() << ")" << std::endl; 
    n_init_edges++;
    } else {
    int v = randomizer(0, emb[i-1].size() - 1);
    G.newEdge(emb[i-1][v], emb[i][j]); 
    seen.insert(key(emb[i-1][v]->index(), emb[i][j]->index()));
    //std::cout << "::: ::: level " << i << " edge created : (" << emb[i][j]->index() << "," << emb[i+1][v]->index() << ")" << std::endl; 
    n_init_edges++;
    }
    //int v = ogdf::randomNumber(0, emb[i+1].size() - 1);
    }
    }
    */
    /*
       for(int i = 1; i < K; ++i){
       for(int j = 0 ; j < emb[i].size(); j++){
    //int v = ogdf::randomNumber(0, emb[i+1].size() - 1);
    if(!ogdf::randomNumber(0, emb[i].size()*2)){
    int v = randomizer(0, emb[i-1].size() - 1);
    G.newEdge(emb[i-1][v], emb[i][j]); 
    seen.insert(key(emb[i-1][v]->index(), emb[i][j]->index()));
    //std::cout << "::: ::: level " << i << " edge created : (" << emb[i][j]->index() << "," << emb[i+1][v]->index() << ")" << std::endl; 
    n_init_edges++;

    }
    }
    }
    */
    int n_edges = 3 * (K - 1 + max) - 6 - (n_init_edges); 
    //std::cout << "::: Number of edges left :" << n_edges  << std::endl;
    //std::cout << "::: CREATION OF WHAT'S LEFT OF THE EDGES" << std::endl;
    int n_edges_total = 0; 
    std::vector<ogdf::edge> addedChuckOfEdges; 
    bool planar = false, prev_planar = false;; 
    //std::cout <<  "number of edges total : " << n_edges << std::endl;;
    int n_tries = 0; 

    equivalenceClasses eqDs;
    //planar = compute2SATClasses(*this, eqDs); 
    bool abort = false;
    /*
       if(!planar){
       this->validGraph = false; 
       abort = true;
       }
       */
    do{
        n_edges_total = 0 ;
        do {
            addedChuckOfEdges.clear();
            int n_edges_chunck = n_edges / 100; 

            n_edges_total += (n_edges_chunck > 0) ? n_edges_chunck : 1; 
            //std::cout << "number of edges: " << n_edges_total << " :: " << n_edges << std::endl;
            for(int i = 0; i < n_edges_chunck; i++){
                //int l = ogdf::randomNumber(0, K - 2); 
                int l = randomizer(0, K - 1); 
                //int u = ogdf::randomNumber(0, emb[l].size() - 1 );
                //int v = ogdf::randomNumber(0, emb[l + 1].size() - 1 );
                int v, u; 
                for(int j = 0 ; j < emb[l].size(); j++)
                    u = randomizer(0, emb[l].size() - 1 );
                bool inv = false;
                if(l == 0) {
                    u = randomizer(0, emb[l].size() - 1 );
                    v = randomizer(0, emb[l + 1].size() - 1 );
                    //std::cout << "u : " << emb[l][u]->index() << " v : " << emb[l+1][v]->index() << std::endl; 
                } else if(l == K - 1){
                    v = randomizer(0, emb[l].size() - 1 );
                    u = randomizer(0, emb[l - 1].size() - 1 );
                    inv = true;
                    //std::cout << "u : " << emb[l-1][u]->index() << " v : " << emb[l][v]->index() << std::endl; 
                } else {
                    //if(ogdf::randomNumber(0,1)) {
                    v = randomizer(0, emb[l].size() - 1 );
                    u = randomizer(0, emb[l - 1].size() - 1 );
                    inv = true;
                    //std::cout << "u : " << emb[l-1][u]->index() << " v : " << emb[l][v]->index() << std::endl; 
                    //} else {
                    /*
                       u = randomizer(0, emb[l].size() - 1 );
                       v = randomizer(0, emb[l + 1].size() - 1 );
                       std::cout << "u : " << emb[l][u]->index() << " v : " << emb[l+1][v]->index() << std::endl; 
                       */
                    //}                      
                }
                // TODO should i not consider in the counter the case where we already have the edge in G 
                //std::cout << "is it seen ? " << std::endl;
                if(inv){
                    if(!seen.count(key(emb[l-1][u]->index(), emb[l][v]->index()))){
                        addedChuckOfEdges.push_back(this->G->newEdge(emb[l-1][u], emb[l][v])); 
                        seen.insert(key(emb[l-1][u]->index(), emb[l][v]->index()));
                    }
                } else {
                    if(!seen.count(key(emb[l][u]->index(), emb[l+1][v]->index()))){
                        addedChuckOfEdges.push_back(this->G->newEdge(emb[l][u], emb[l+1][v])); 
                        seen.insert(key(emb[l][u]->index(), emb[l+1][v]->index()));
                    }

                }
            }
            prev_planar = planar; 
            //std::cout << "Prev_planar value " << ((prev_planar) ? 1 : 0) << std::endl;  
            planar = compute2SATClasses(*this, eqDs); 
        }while(planar && n_edges_total < n_edges && !abort);
        n_tries++; 
    }while(!prev_planar && n_tries < 300 && !abort);

    if(!prev_planar){
        std::cout << "couldn't find a planar Graph with this setting" << std::endl; 
        this->validGraph = false;
    } else {
        std::cout << "Found a level proper planar Graph" << std::endl;
        for(ogdf::edge e : addedChuckOfEdges) {
            this->G->delEdge(e); 
        }
    }
    std::vector<ogdf::node> delNodes;
    for(ogdf::node v: this->G->nodes){
        if(!v->degree()){
            delNodes.push_back(v);
        }
    }
    for(ogdf::node v: delNodes){
        int levelSize = emb[levelNode[v]].size();
        for(int i = 0 ; i < levelSize ; i++){
            if(emb[levelNode[v]][i] == v){
                emb[levelNode[v]][i] = emb[levelNode[v]][levelSize - 1]; 
                emb[levelNode[v]].pop_back(); 
                levelSize--;
            } 
        }
        this->G->delNode(v);
    }
    if(this->validGraph && !ogdf::isConnected(*this->G)){
        /*
           ogdf::NodeArray<int> components(G);
           std::map<int, int> compSizes; 
           int num_cc = ogdf::connectedComponents(G, components, nullptr, nullptr);
           int size_ccs[num_cc];
           for(int i = 0; i < num_cc ; i++){
           size_ccs[i] = 0;
           }
           for(const auto& node: G.nodes){
           size_ccs[components[node]]++;    
           }
           int max = 0;
           int big_cc = -1; 
           for(int i = 0; i < num_cc; i++){
           if(max < size_ccs[i]){
           max = size_ccs[i]; 
           big_cc = i;
           }
           }
           ogdf::NodeArray<ogdf::node> GtoSubGnodes(G); 
           for (ogdf::node n : G.nodes){
           GtoSubGnodes[n] = nullptr;
           }
           std::vector<std::vector<ogdf::node>> newEmb;
           ogdf::Graph subG; 
           bool createNewLevel = false;
           for(const auto& level : emb){
           createNewLevel = false;
           for(const auto& node: level){
           if(components[node] == big_cc){
           if(!createNewLevel){
           createNewLevel = true; 
           newEmb.emplace_back();
           }
           ogdf::node addedNode = subG.newNode();
           newEmb.back().push_back(addedNode); 
           GtoSubGnodes[node] = addedNode;
           } 
           } 
           }
           for(const auto& level : emb){
           std::cout << "size of level : " << level.size() << std::endl;
           }
           for(const auto& e: G.edges){
           if((GtoSubGnodes[e->source()] != nullptr) && (GtoSubGnodes[e->target()] != nullptr)) {
           ogdf::node src = GtoSubGnodes[e->source()];
           ogdf::node trg = GtoSubGnodes[e->target()];
           subG.newEdge(src, trg); 
           }
           }

           G.clear();
           this->emb.clear();
           this->emb.reserve(newEmb.size());
           ogdf::NodeArray<ogdf::node>subtoG(subG, nullptr);
           for(const auto& level : newEmb){
           emb.emplace_back();
           for(const auto& node: level){
           ogdf::node vNew = G.newNode(); 
           subtoG[node] = vNew;
           emb.back().push_back(vNew);
           } 
           }
           for(ogdf::edge e : subG.edges){
           ogdf::node u = subtoG[e->source()]; 
           ogdf::node v = subtoG[e->target()]; 
           if(u && v) G.newEdge(u,v);
           }
           this->GA.init(G, ogdf::GraphAttributes::all);
           this->LVL.init(G);
           this->CG = ogdf::ClusterGraph(G);
        //this->GA.init(G, ogdf::GraphAttributes::all);
        std::cout << "Graph NOT CONNECTED, shrinked to a subgraph of a connected component"  << std::endl;
        if(!ogdf::isConnected(G)){
            this->validGraph = false;
            std::cout << "Shouldn't happen, the subgraph induced by a connected comp of G is also not connected ?!" << std::endl;
        }
        */
            this->validGraph = false;
        std::cout << "Couldn't find a connected graph" << std::endl;
    }
    /*
       if(ogdf::isPlanar(G)){
       std::cout << "the graph G is Planar." << std::endl;  
       } else {
       std::cout << "the graph G is NOT Planar." << std::endl;  
       } 
       */
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
        ogdf::ClusterGraphAttributes CGA(*this->CG, ogdf::ClusterGraphAttributes::all);
    }
    postTraitement();


}
