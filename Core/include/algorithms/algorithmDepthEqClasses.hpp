#pragma once
#include "contribution.hpp"
#include "GraphBuilder.h"
#include "type.hpp"

class ContributionSimpleGreedyWithEquivalenceClassDepth: public Contribution{
        int enforceTransitivity(GraphBuilder& builder, equivalenceClasses& eq) override;
        void depthAndStartOfEquivalenceClasses(GraphBuilder& builder, equivalenceClasses& eq, std::vector<std::vector<int>>& startOfEquivalenceClasses, int* depthOfEquivalenceClasses);
    public :
        ContributionSimpleGreedyWithEquivalenceClassDepth(PROFILING profiling = PROFILING::DISABLE, GENERATING_OUTPUT canGenerate = GENERATING_OUTPUT::DISABLE, DEBUGING debug = DEBUGING::DISABLE) :
        Contribution(profiling, canGenerate , debug){
            this->algName = "greedy-depth-prio";
        }
};
