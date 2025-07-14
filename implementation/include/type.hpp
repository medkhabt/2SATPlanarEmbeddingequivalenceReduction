#pragma once
#include <utility> 
#include <map>
#include <set>
#include <memory>

using nodePair = std::pair<int, int>;
using nodePairSet = std::set<nodePair>; 
using sharedNodePairSet = std::shared_ptr<nodePairSet>;
using equivalentClasses = std::map<nodePair, sharedNodePairSet>;
using equivalentClassesAssignement = std::map<nodePair, int>;
