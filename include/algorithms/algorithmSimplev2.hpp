#include "contribution.hpp"
#include "GraphBuilder.h"
#include "type.hpp"

class ContributionSimpleGreedyWithSortApproach : public Contribution{
        void enforceTransitivity(GraphBuilder& builder, equivalenceClasses& eq) override;
        void verticesWithNeighborsFromUpperLevelFirst(const GraphBuilder& builder, std::vector<ogdf::node>& vertices);
    public :
        ContributionSimpleGreedyWithSortApproach(PROFILING profiling = PROFILING::DISABLE, GENERATING_OUTPUT canGenerate = GENERATING_OUTPUT::DISABLE, DEBUGING debug = DEBUGING::DISABLE) :
        Contribution(profiling, canGenerate , debug){}
};
