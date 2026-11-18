#pragma once
#include "CampaignTypes.hpp"

namespace frontline
{
    inline constexpr int GrandBattleArmyCount=11;
    inline constexpr int GrandCommander=10;
    inline constexpr int GrandRally=11+6*Width;
    struct JourneyGrandBattle
    {
        bool active=false;
        int openedBy=-1,openedOn=0,ralliedOn=0;
        int alliedStarted=0,alliedLosses=0;
    };
}
