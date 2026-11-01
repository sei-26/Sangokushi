#pragma once
#include "JourneyEpisode.hpp"
#include <array>
namespace frontline
{
    class StrategistJourney;
    struct ChainEvent;
    std::u32string CouncilBattleVoice(const StrategistJourney&,const ChainEvent&,int step);
    enum class CouncilDecision { None, OpenRoad, Escort, Ambush, Reserves };
    struct CouncilStageBeat
    {
        JourneyEpisodeLine dialogue;
        std::array<std::u32string,3> reactions{};
    };
    struct CouncilExchange
    {
        std::u32string subject;
        std::vector<CouncilStageBeat> beats;
    };
    // Presentation only: proposing, debating and acknowledging an explicitly chosen order.
    // No resources, assignments, relationships, hidden enemy policy or history are modified.
    CouncilExchange BuildCouncilExchange(const StrategistJourney&,int voice,int preparation,CouncilDecision accepted=CouncilDecision::None);
}
