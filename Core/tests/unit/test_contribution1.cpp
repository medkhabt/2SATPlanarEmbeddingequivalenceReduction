#include "GraphBuilder.h"
#include "2SatCompute.hpp"
#include "algorithmSimplev1.hpp"
#include "utils.hpp"
#include <gtest/gtest.h>

//TODO need other tests to make sure this functions correctly
TEST(contribution1, customGraphenforceTransitivity){
    //TODO I need to provide just the data without relying on other functions, for now i don't have time, but this is a must in the future
    //GIVEN 
    GraphBuilder builder; 
    builder.buildLevelGraphFromGML("graphs/counterexample_local_transitivity.gml");

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
     bool test = testEmbedding(builder, eq,  "counterexample_local_transitivity_test");
    //TEST
     EXPECT_EQ(test, true);
    //CLEAN UP

}
