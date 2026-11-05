#include "JourneyDefenseAdvice.hpp"
#include "StrategistJourney.hpp"
namespace frontline
{
    std::optional<JourneyDefenseLesson> ReadDefenseRetryLesson(const StrategistJourney& j,const StrategistJourney& before)
    {
        if(j.stage!=JourneyStage::Debrief || j.won || !j.campaignFlow || j.mission!=2 ||
           before.stage!=JourneyStage::Preparation || !before.campaignFlow || before.mission!=j.mission ||
           j.Definition().kind!=JourneyObjective::HoldPositions || before.Definition().kind!=JourneyObjective::HoldPositions) return {};
        const auto& h=j.history[j.mission];const auto& r=h.review;
        if(!h.completed || h.won || h.withdrawn!=j.withdrawn || r.endedAt!=j.turn || r.endedAt<=0 || r.endedAt>16 ||
           r.spentGuardFailures<=0 || r.spentGuardFailures>7 || r.spentGuardFailures!=j.battleReview.spentGuardFailures ||
           r.endedAt!=j.battleReview.endedAt || r.resolution!=j.battleReview.resolution) return {};
        if(j.withdrawn ? r.resolution!=JourneyResolution::Withdrawn:
           r.resolution!=JourneyResolution::ForceLost && r.resolution!=JourneyResolution::Deadline) return {};
        return JourneyDefenseLesson{j.mission,r.spentGuardFailures,r.endedAt,r.resolution};
    }
}
