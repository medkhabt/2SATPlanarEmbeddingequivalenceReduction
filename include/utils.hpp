#pragma once
#include <utility>
#include <ogdf/basic/Graph.h>
#include <ogdf/basic/RegisteredArray.h>
#include "type.hpp"
#include "GraphWithPairNode.hpp"
#include <ogdf/basic/GraphList.h>
#include "GraphBuilder.h"
#include "2SatCompute.hpp"
// TODO customize the naming of the creation of a layout.

namespace ogdf {

    class PairNodeElement;
    using pairNode = PairNodeElement*; 
    namespace internal{
        using GraphPairNodeRegistry = GraphRegistry<PairNodeElement>; 
    }

    template<typename Value, bool WithDefault= true> 
        using PairNodeArray = RegisteredArray<PairNodeElement, Value, WithDefault, GraphWithPairNode>;

    
}

void createLayout(std::string nameFile, ogdf::Graph& G);

//bool planarityCheck(std::vector<equivalentClassesAssignement>& eqAs, equivalentClasses& eq);

bool AcyclicRelation(std::string title, std::vector<equivalenceClassesAssignement>& assignement, GraphBuilder builder);

//std::vector<equivalentClassesAssignement> fillEquivalentClasses(const equivalentClasses& eq);

//void print_eq(const equivalentClasses& eq);

template < class Key>
int partition(std::vector<Key> arr, int leftIndex, int rightIndex, std::map<int, Key>& comparator )
    {
        int pivotIndex = leftIndex;
        int pivotValue = arr[pivotIndex];

        int i = leftIndex;
        int j = rightIndex;
        

        while(i < j){
            while(comparator[arr[i]] <= comparator[pivotValue]){
                i++;
                if(i >= rightIndex) break;
            }
            while(comparator[arr[j]] >= comparator[pivotValue]){
                j--;
                if(j <= leftIndex) break;
            }
            if(i < j){
                int temp = arr[i];
                arr[i] = arr[j];
                arr[j] = temp;
            }
        }

        // swap
        arr[pivotIndex] = arr[j];
        arr[j] = pivotValue;

        return j;
    }
template <class Key> 
    void quickSort(std::vector<Key>& arr, int left, int right, std::map<int,Key>& comparator)
   {    
   
      if (left < right)
      {
        int pivot = partition(arr, left, right, comparator);
        quickSort(arr, left, pivot - 1, comparator);
        quickSort(arr, pivot + 1, right, comparator);
      }
      return;
   }


inline int eqId(int u, int v, int nodesSize, equivalenceClasses& eq, int level){
   return eq.disjointSets.getRepresentative(eq.pairId[level][localIndex(eq, u, v, level)]);  
}
inline void mergeTwoEqs(int u1, int v1, int u2, int v2, int nodesSize, equivalenceClasses& eq, int level1, int level2){
    eq.disjointSets.quickUnion(eq.pairId[level1][localIndex(eq, u1, v1, level1)], eq.pairId[level2][localIndex(eq, u2, v2, level2)]); 
    eq.disjointSets.quickUnion(eq.pairId[level1][localIndex(eq, v1, u1, level1)], eq.pairId[level2][localIndex(eq, v2, u2, level2)]); 
}

bool testEmbedding(GraphBuilder& builder, equivalenceClasses& eq, std::string title);
