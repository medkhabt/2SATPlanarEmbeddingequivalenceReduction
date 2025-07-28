#pragma once
#include <ogdf/basic/Graph.h>
#include "type.hpp"
// TODO customize the naming of the creation of a layout.
void createLayout(std::string nameFile, ogdf::Graph& G);

bool planarityCheck(std::vector<equivalentClassesAssignement>& eqAs, equivalentClasses& eq);

bool AcyclicRelation(std::string title, std::vector<equivalentClassesAssignement>& assignement);

std::vector<equivalentClassesAssignement> fillEquivalentClasses(const equivalentClasses& eq);

void print_eq(const equivalentClasses& eq);

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
