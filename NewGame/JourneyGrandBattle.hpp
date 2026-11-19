#pragma once
#include "CampaignTypes.hpp"
#include "JourneyYangrenBattle.hpp"

namespace frontline
{
    inline constexpr int GrandBattleArmyCount=11;
    inline constexpr int GrandCommander=10;
    inline constexpr int GrandRally=11+6*Width;
    inline constexpr int GrandLeftAnchor=6+3*Width;
    inline constexpr int GrandNorthPass=8+3*Width;
    enum class GrandApproach { None, RelieveLeft, CentralAssault };
    struct JourneyGrandBattle
    {
        bool active=false;
        int openedBy=-1,openedOn=0,ralliedOn=0;
        int alliedStarted=0,alliedLosses=0;
        bool ordersEnabled=false;
        GrandApproach approach=GrandApproach::None;
        int chosenOn=0,relievedOn=0,reliefOfficer=-1,leftReturned=0;
        int forcedSteps=0;
        JourneyYangrenBattle yangren;
    };
}
