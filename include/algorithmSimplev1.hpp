#include "algorithm.hpp"
#include "GraphBuilder.h"
#include "utils.hpp"
#include "type.hpp"
#include <unordered_set>

class Contribution1 {
    public : 
        static void enforceTransitivity(GraphBuilder& builder, equivalenceClasses& eq);
};
