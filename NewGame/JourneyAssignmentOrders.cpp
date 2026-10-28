#include "JourneyRealtime.hpp"
namespace frontline
{
    void JourneyRealtime::ApplyAssignments(const StrategistJourney& j)
    {
        if(j.stage!=JourneyStage::Battle) return;
        for(int i=0;i<3;++i)
        {
            const auto& a=j.assignments[i];if(a.duty==JourneyDuty::None) continue;
            if(Queue(j,i,JourneyDirective::March,a.tile))
                SetArrival(j,i,a.duty==JourneyDuty::Ambush ? JourneyArrival::Ambush:JourneyArrival::Hold);
        }
    }
}
