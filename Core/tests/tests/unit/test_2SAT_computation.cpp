#include "GraphBuilder.h"
#include "2SatCompute.hpp"
#include <gtest/gtest.h>

TEST(SAT2_COMPUTE_TESTS, customGraph2SAT_TEST){
    /***** INPUT ****/
    /*
(2,12) = 4

(12,2) = 5

(12,3) = 22 = (2,3) = (5,9)

(3,12) = (3,2) = (9,5)

(12,4) = (2,4) = (5,10) = (5,11)

(10,5) = (11,5) = (4,12) = (4,2)

(12,1) = (2,1) = (5,8)


(1,12) = (1,2) = (8,5)

(10,11) = (10,9) = (11,9) = (1,3) = (1,4) = (4,3) = (6,7) = (8,10) = (8,11) = (8,9)


(10,8) = (11,10) = (11,8) = (3,1) = (3,4) = (4,1) = (7,6) = (9,10) = (9,11) = (9,8)
*/
    // I should probably mock this part, or just use an input set. 
    GraphBuilder builder; 
    builder.buildLevelGraphFromGML("graphs/counterexample.gml");
    equivalenceClasses eq ;

    int nodesSize = builder.G.numberOfNodes();
    std::vector<std::vector<int>> expected_eq;
    std::vector<int> eq1 = {2*nodesSize + 12}; 
    expected_eq.push_back(eq1); 
    std::vector<int> eq2 = {12*nodesSize + 3, 2*nodesSize + 3, 5*nodesSize + 9};
    expected_eq.push_back(eq2); 
    std::vector<int> eq3 = {12*nodesSize + 4, 2*nodesSize + 4, 5*nodesSize + 10, 5*nodesSize + 11 };
    expected_eq.push_back(eq3); 
    std::vector<int> eq4 = {12*nodesSize + 1, 2*nodesSize + 1, 5*nodesSize + 8};
    expected_eq.push_back(eq4); 
    std::vector<int> eq5 = {10*nodesSize + 11, 10*nodesSize + 9, 11*nodesSize + 9, 1*nodesSize + 3, 1*nodesSize + 4, 
        4*nodesSize + 3, 6*nodesSize + 7, 8*nodesSize + 10, 8*nodesSize + 11, 8*nodesSize + 9};
    expected_eq.push_back(eq5); 

    std::vector<int> eq1_rev = {12*nodesSize + 2}; 
    expected_eq.push_back(eq1_rev); 
    std::vector<int> eq2_rev = {3*nodesSize + 12, 3*nodesSize + 2, 9*nodesSize + 5};
    expected_eq.push_back(eq2_rev); 
    std::vector<int> eq3_rev = {4 *nodesSize + 12, 4*nodesSize + 2, 10 *nodesSize + 5, 11*nodesSize + 5 };
    expected_eq.push_back(eq3_rev); 
    std::vector<int> eq4_rev = {1*nodesSize + 12, 1*nodesSize + 2, 8*nodesSize + 5};
    expected_eq.push_back(eq4_rev); 
    std::vector<int> eq5_rev = {11*nodesSize + 10, 9*nodesSize + 10, 9*nodesSize + 11, 3*nodesSize + 1, 4*nodesSize + 1, 
        3*nodesSize + 4, 7*nodesSize + 6, 10*nodesSize + 8, 11*nodesSize + 8, 9*nodesSize + 8};
    expected_eq.push_back(eq5_rev); 
    // How about not putting graphBuild an arg, is it necessary or we just need the drawing.
    //
    /**** ACTION ***/
    compute2SATClasses(builder, eq);
    /**** TEST ****/
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

    /**** CLEAN UP ***/

}
