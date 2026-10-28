#pragma once
#include <string>
namespace frontline
{
    class StrategistJourney;
    struct JourneyCampaignBeat { std::u32string phase,change,next,standing; };
    JourneyCampaignBeat CampaignBeat(const StrategistJourney&);
}
