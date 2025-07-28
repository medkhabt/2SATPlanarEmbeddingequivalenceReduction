#pragma once
#include <utility> 
#include <map>
#include <set>
#include <unordered_set>
#include <unordered_map>
#include <memory>
#include <boost/container/flat_map.hpp>

struct pair_hash {
    template <typename T1, typename T2>
    std::size_t operator()(const std::pair<T1, T2> &p) const noexcept {
        std::size_t h1 = std::hash<T1>{}(p.first);
        std::size_t h2 = std::hash<T2>{}(p.second);
        return h1 ^ (h2 << 1); // or use boost::hash_combine
    }
};
using nodePair = std::pair<int, int>;
using nodePairSet = std::unordered_set<nodePair,pair_hash>; 
using sharedNodePairSet = std::shared_ptr<nodePairSet>;
//using equivalentClasses = std::map<nodePair, sharedNodePairSet>;
using equivalentClasses = std::map<nodePair, sharedNodePairSet>;
//using equivalentClassesAssignement = std::map<nodePair, int>;
using equivalentClassesAssignement = boost::container::flat_map<nodePair, int>;

