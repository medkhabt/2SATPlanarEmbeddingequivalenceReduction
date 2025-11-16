#pragma once
#include <utility>
#include <ogdf/basic/Graph.h>
#include "type.hpp"
#include <ogdf/basic/GraphList.h>
#include "GraphBuilder.h"
// TODO customize the naming of the creation of a layout.


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
   return eq.disjointSets.getRepresentative(eq.pairId[level][localIndex(eq, u, v, level)]);  
}
inline void mergeTwoEqs(int u1, int v1, int u2, int v2, int nodesSize, equivalenceClasses& eq, int level1, int level2){
    eq.disjointSets.quickUnion(eq.pairId[level1][localIndex(eq, u1, v1, level1)], eq.pairId[level2][localIndex(eq, u2, v2, level2)]); 
    eq.disjointSets.quickUnion(eq.pairId[level1][localIndex(eq, v1, u1, level1)], eq.pairId[level2][localIndex(eq, v2, u2, level2)]); 
}

bool testEmbedding(GraphBuilder& builder, equivalenceClasses& eq, std::string title, bool canGenerate = false, int** ordering = nullptr, bool isTotalOrder = false);
