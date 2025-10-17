#pragma once
#include <utility>
#include <ogdf/basic/Graph.h>
#include <ogdf/basic/RegisteredArray.h>
#include "type.hpp"
#include "GraphWithPairNode.hpp"
#include <ogdf/basic/GraphList.h>
#include "GraphBuilder.h"
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
inline int localIndex(equivalenceClasses& eq, int u, int v, int level){
    int offset = eq.pairIdOffset[level];
    int size = eq.pairIdArraySize[level];
    int ulocal= eq.pairIdLocalIndex[level][u - offset];
    int vlocal= eq.pairIdLocalIndex[level][v - offset];
    return ulocal * sqrt(size/2) + vlocal; 

}
inline std::pair<int,int> localIndexInverse(equivalenceClasses& eq, int key,  int level){
    int offset = eq.pairIdOffset[level];
    int size = eq.pairIdArraySize[level];
    int ulocal = key / int(sqrt(size/2)); 
    int vlocal = key % int(sqrt(size/2)); 
    int u = eq.pairIdLocalIndexInverse[level][ulocal] + offset; 
    int v = eq.pairIdLocalIndexInverse[level][vlocal] + offset; 
    return std::pair<int,int>(u,v); 

}

inline int eqId(int u, int v, int nodesSize, equivalenceClasses& eq, int level){
   //std::cout << "u  :" << u << " v: " << v << std::endl;
   return eq.disjointSets.getRepresentative(eq.pairId[level][localIndex(eq, u, v, level)]);  
}
inline void mergeTwoEqs(int u1, int v1, int u2, int v2, int nodesSize, equivalenceClasses& eq){
    eq.disjointSets.quickUnion(eq.pairId[u1 * nodesSize + v1], eq.pairId[u2 * nodesSize + v2]); 
    eq.disjointSets.quickUnion(eq.pairId[v1 * nodesSize + u1], eq.pairId[v2 * nodesSize + u2]); 
}

inline void mergeTwoEqs(int u1, int v1, int id2, int nodesSize, equivalenceClasses& eq){
    int u2 = id2/nodesSize; 
    int v2 = id2%nodesSize; 
    mergeTwoEqs(u1, v1, u2, v2, nodesSize, eq); 
}

inline void mergeTwoEqs(int id1, int id2, int nodesSize, equivalenceClasses& eq){
    int u1 = id1/nodesSize; 
    int v1 = id1%nodesSize; 
    int u2 = id2/nodesSize; 
    int v2 = id2%nodesSize; 
    mergeTwoEqs(u1, v1, u2, v2, nodesSize, eq); 
}
bool testEmbedding(GraphBuilder& builder, equivalenceClasses& eq, std::string title);
