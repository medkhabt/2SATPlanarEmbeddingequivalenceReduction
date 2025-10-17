#include "contribution.hpp"
#include "GraphBuilder.h"
#include "type.hpp"

class ContributionSimpleGreedyApproach : public Contribution{
        void enforceTransitivity(GraphBuilder& builder, equivalenceClasses& eq) override;
    public :
        ContributionSimpleGreedyApproach(PROFILING profiling = PROFILING::DISABLE, GENERATING_OUTPUT canGenerate = GENERATING_OUTPUT::DISABLE, DEBUGING debug = DEBUGING::DISABLE) :
        Contribution(profiling, canGenerate , debug){}
};
