#include "GraphBuilder.h"
#include "2SatCompute.hpp"
#include "algorithmSimplev1.hpp"
#include "utils.hpp"
#include <gtest/gtest.h>

int randomInt(int low, int high) {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_int_distribution<> dist(low, high);
    return dist(gen);
}
TEST(contribution1, randomGraphs){
    //TODO I need to provide just the data without relying on other functions, for now i don't have time, but this is a must in the future
    //GIVEN 
    int max_nodes, max_levels; 
    max_nodes = randomInt(10, 1000); 
    max_levels = randomInt(1, max_nodes);
    std::cout << "nodes: "  << max_nodes << " level: " << max_levels << std::endl; 
    GraphBuilder builder; 
    std::cout << "Graph with nodes: " << max_nodes << " and levels: "  << max_levels << std::endl; 
    builder.buildRandomLevelGraph(max_nodes, max_levels);

    equivalenceClasses eq ;
    /*
>> Vertex 0 ,  , 2 3
>> Vertex 2,0,5
>> Vertex 1,,8
>> Vertex 3,0,9
>> Vertex 4,,10 11
>> Vertex 12,,5
>> Vertex 5,2 12,
>> Vertex 11,4,7
>> Vertex 9,3,7
>> Vertex 10,4,6
>> Vertex 8,1,6
>> Vertex 6,8 10,
>> Vertex 7,9 11,
*/

    //TODO there is a feature to make use of the same data set for multiple tests.
    int nodesSize = builder.G.numberOfNodes();
    std::vector<std::vector<int>> expected_eq;


    //ACTION
    compute2SATClasses(builder, eq);
    Contribution1::enforceTransitivity(builder, eq);
     bool test = testEmbedding(builder, eq, "random_v_" + std::to_string(max_nodes) + "_l_" + std::to_string(max_levels) + "_test" );
    //TEST
         EXPECT_EQ(test, true);
    //CLEAN UP

}
