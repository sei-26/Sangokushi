#pragma once
#include "JourneyMission.hpp"

namespace frontline
{
    inline constexpr int CoalitionBridge=7+9*Width;
    inline constexpr int CoalitionRetreat=5+9*Width;
    inline constexpr int CoalitionVanguard=10+6*Width;
    struct JourneyCoalitionBattle
    {
        bool active=false;
        int rescuedBy=-1,rescuedOn=0,returnedOn=0;
        int caoReturned=0,reliefReturned=0;
    };
    inline const JourneyMission& CoalitionGrandMission(CoalitionPlan plan)
    {
        static const JourneyMission advance{U"汴水の会戦 ／ 曹操と対岸を押せ",
            U"曹操の前陣へ渡河し、徐栄軍の本陣を撃破・戦意低下させる",
            U"先行する曹操と張邈の援軍が対岸で戦っている。二つの橋を使い、曹操と攻めるか、南の援軍も救うかを決めよう。",
            JourneyObjective::CaptureCamp,18,CoalitionVanguard,-1,0,
            {{5+6*Width,5+8*Width,6+9*Width,10+6*Width,11+9*Width}}};
        static const JourneyMission protect{U"汴水の会戦 ／ 連合軍を連れ帰れ",
            U"南の敵を崩して張邈の援軍へ合流し、曹操と援軍を西岸へ連れ帰る",
            U"徐栄軍は南から橋を切ろうとしている。援軍の隣へ実際に救援を届け、南橋の西で曹操たちを迎えよう。残敵掃討は不要。",
            JourneyObjective::CaptureCamp,18,CoalitionRetreat,-1,0,
            {{5+6*Width,5+8*Width,6+9*Width,10+6*Width,11+9*Width}}};
        return plan==CoalitionPlan::Protect ? protect:advance;
    }
}
