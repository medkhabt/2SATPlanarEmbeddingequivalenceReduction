#include "GraphBuilder.h"
#include "2SatCompute.hpp"
#include "algorithm.hpp"
#include <gtest/gtest.h>

//TODO need other tests to make sure this functions correctly
TEST(LOCAL_TRANSITIVITY, customGraphLocalTransitivity){
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
    std::vector<int> eq1 = {2*nodesSize + 14}; 
    expected_eq.push_back(eq1); 
    std::vector<int> eq2 = {14*nodesSize + 3, 2*nodesSize + 3, 7*nodesSize + 11};
    expected_eq.push_back(eq2); 
    std::vector<int> eq3 = {14*nodesSize + 4, 2*nodesSize + 4, 7*nodesSize + 12, 7*nodesSize + 13 };
    expected_eq.push_back(eq3); 
    std::vector<int> eq4 = {14*nodesSize + 1, 2*nodesSize + 1, 7*nodesSize + 10};
    expected_eq.push_back(eq4); 
    std::vector<int> eq5 = {12*nodesSize + 13, 12*nodesSize + 11, 13*nodesSize + 11, 1*nodesSize + 3, 1*nodesSize + 4, 
        4*nodesSize + 3, 8*nodesSize + 9, 10*nodesSize + 12, 10*nodesSize + 13, 10*nodesSize + 11};
    expected_eq.push_back(eq5); 

    std::vector<int> eq1_rev = {14*nodesSize + 2}; 
    expected_eq.push_back(eq1_rev); 
    std::vector<int> eq2_rev = {3*nodesSize + 14, 3*nodesSize + 2, 11*nodesSize + 7};
    expected_eq.push_back(eq2_rev); 
    std::vector<int> eq3_rev = {4 *nodesSize + 14, 4*nodesSize + 2, 12 *nodesSize + 7, 13*nodesSize + 7 };
    expected_eq.push_back(eq3_rev); 
    std::vector<int> eq4_rev = {1*nodesSize + 14, 1*nodesSize + 2, 10*nodesSize + 7};
    expected_eq.push_back(eq4_rev); 
    std::vector<int> eq5_rev = {13*nodesSize + 12, 11*nodesSize + 12, 11*nodesSize + 13, 3*nodesSize + 1, 4*nodesSize + 1, 
        3*nodesSize + 4, 9*nodesSize + 8, 12*nodesSize + 10, 13*nodesSize + 10, 11*nodesSize + 10};
    expected_eq.push_back(eq5_rev); 

    // Lets take vertex 0 
    std::vector<int>adjOut = {5, 2, 3};
    std::vector<int>adjIn= {};


    //ACTION
    compute2SATClasses(builder, eq);
    //Contribution::addAdjacentEdgesRestrition(eq, adjOut, adjIn, nodesSize);

    //TEST
    /*
    for(const auto& eq_ele : expected_eq){
        int eqIdSet = -1; 
        for(const int ele: eq_ele) {
             int eleIdSet = eq.disjointSets.getRepresentative(eq.pairId[ele]);  
            if(eqIdSet == -1 ){
               eqIdSet = eleIdSet;
            } 
         std::cout << "ele : " << ele / nodesSize << ", " << ele % nodesSize <<  " = " << eleIdSet << " with : " << eqIdSet << std::endl;
         EXPECT_EQ(eqIdSet, eleIdSet);
        } 
    }
    */
    //CLEAN UP

}
