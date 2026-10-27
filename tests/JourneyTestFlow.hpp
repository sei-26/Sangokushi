#pragma once
#include "../NewGame/StrategistJourney.hpp"
namespace frontline
{
    // Existing playthroughs choose a real pledge through the same public rules.
    inline bool ContinueJourneyWithPledge(StrategistJourney& journey)
    {
        if(journey.NeedsStoryReply() && !journey.ChooseStoryReply(JourneyPledge::Preserve)) return false;
        return journey.Continue();
    }
}
