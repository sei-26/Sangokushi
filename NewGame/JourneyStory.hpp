#pragma once
#include <array>
#include <string>
#include <vector>
namespace frontline
{
    enum class JourneyPledge { None, Preserve, Coordinate };
    struct JourneyBattleStory
    {
        bool recorded=false, protectedPeople=false, lowLoss=false, costly=false, pledgeKept=false;
        int coordinatedOfficers=0, loss=0;
        JourneyPledge pledge=JourneyPledge::None, reply=JourneyPledge::None;
        std::array<int,3> trustDelta{};
    };
    struct JourneyDialogueLine { int speaker=0;std::u32string text; }; // 0 Liu, 1 Guan, 2 Zhang, 3 protagonist.
    class StrategistJourney;
    const char32_t* PledgeName(JourneyPledge);
    const char32_t* BattleStoryName(const JourneyBattleStory&,bool won,bool withdrawn);
    std::vector<JourneyDialogueLine> JourneyConversation(const StrategistJourney&,int officer);
}
