#pragma once
#include "CampaignTypes.hpp"
#include "JourneyMission.hpp"
#include "JourneyOpening.hpp"
#include <algorithm>
#include <array>
#include <string_view>

namespace frontline
{
    enum class JourneyMap { Retreat, Port, Valley, Guangzong, CoalitionFord, RefugeeRoad, RefugeeWood, PortNorth, CoalitionRoad };
    inline JourneyMap SelectJourneyMap(int mission,bool rescue,JourneyConvoyRoute route,bool north,CoalitionPlan coalition,bool legacy)
    {
        if(!legacy)
        {
            if(mission==0 && rescue) return route==JourneyConvoyRoute::SouthernWood ? JourneyMap::RefugeeWood:JourneyMap::RefugeeRoad;
            if(mission==1 && north) return JourneyMap::PortNorth;
            if(mission==CoalitionMission && coalition==CoalitionPlan::Protect) return JourneyMap::CoalitionRoad;
        }
        return static_cast<JourneyMap>(std::clamp(mission,0,CoalitionMission));
    }
    // Authored battle compositions, shared by navigation, sight, AI and rendering.
    // Rows correspond to playable y=1..12, columns to x=1..14.
    // . plain, F forest, M mountain (cover/LOS), ~ impassable river, = bridge.
    struct JourneyBattlefield
    {
        const char32_t* name;
        const char32_t* reading;
        std::array<std::string_view, 12> rows;
    };
    inline const JourneyBattlefield& BattlefieldDefinition(JourneyMap map)
    {
        static const std::array<JourneyBattlefield, 9> fields{{
            {U"林間の退路", U"森で誘い、街道を開く",
             {{"..MMM...MMMMM.",
               "..MMM..MMMMMMM",
               "..FFF.........",
               "..FFFF....FFF.",
               "..FFFFFFFFFFF.",
               "..FFFFFFFFFFF.",
               "....FFFFFF.FF.",
               "....FFFFFF....",
               "....FFFFFF....",
               "..............",
               "..............",
               ".............."}}},
            {U"河畔の渡し", U"北橋で回り込み、南橋で糧車を通す",
             {{"MMM.......~...",
               "MMM.......~.FF",
               "..FFF.....~.FF",
               "..FFF.....~.FF",
               "..FFF.....~...",
               "..........~...",
               "......FFF.~...",
               "......FFF.=..M",
               "......FFF.~..M",
               "..........~...",
               "..........=...",
               "..........~..."}}},
            {U"双烽火の渓谷", U"川の南北へ分担し、二つの橋で援護する",
             {{".......MMMM...",
               ".......MMMM...",
               "...MMM..MM....",
               "........MM....",
               ".....FF.MM....",
               ".....FF.......",
               "~~~~~~=~~~=~~~",
               ".........FFF..",
               ".........FFF..",
               "..MMM....FFFMM",
               "..MMM....FFFMM",
               "..MMM.......MM"}}},
            {U"広宗郊外", U"北の林で敵を引きつけ、南の平地から前陣へ入る",
             {{"..MM...MMMM...",
               "..MM...MMMM...",
               "...F..........",
               "...FFFF.......",
               "....FFFFFF....",
               ".....FFFFF....",
               ".....FFF......",
               ".....FFF......",
               "..............",
               "..............",
               "....MM........",
               "....MMMM......"}}},
            {U"酸棗西の渡し", U"北橋から前哨へ。南橋で敵の反撃を受け止める",
             {{"..MMMM....~...",
               "..MMMM....~.MM",
               "....FFF...~.MM",
               "....FFFF..~...",
               ".....FFF..~...",
               ".....FFF..=...",
               "....FFFF..~...",
               "....FFFF..~.FF",
               "..........~.FF",
               "..........=...",
               "...MM.....~...",
               "..MMMM....~..."}}},
            {U"季安の街道", U"畑の間をまっすぐ通る。前路の敵を道から離す",
             {{"MM...FFF......",
               "MM..FFFF......",
               "M...FFF.......",
               "....FF....FFF.",
               ".MM.......FFF.",
               ".MMM.......FF.",
               "..M.....FF.FF.",
               ".......FFFF...",
               "......FFFF....",
               "..............",
               "..............",
               "........F....."}}},
            {U"南の林道", U"尾根の南を回る。曲がり角と林の出口に隊を送る",
             {{"....MMMMMMMM..",
               "...MMMMMMMMM..",
               ".......MMM....",
               "..............",
               "..FF..FFF.....",
               "..FFF.FFFF....",
               "......FFFF....",
               "......FFFF....",
               "......FFFF....",
               "..............",
               "..............",
               "......FFFF...."}}},
            {U"河港北の曲流", U"北の森を抜けて上流の橋へ。東岸から港へ回り込む",
             {{"....MM...~....",
               "...MMM...~....",
               "...MM...~.....",
               "....FF...~....",
               ".....FFF.~....",
               ".....FFF..~...",
               "......FFF.~...",
               "......FFF.=...",
               "......FFF.~...",
               "..........~...",
               "..........=...",
               "..........~..."}}},
            {U"酸棗の集落街道", U"畑の間を通る街道を守る。森の切れ目から敵が来る",
             {{"..MMMMM.......",
               "...MMMMM......",
               ".......MMM....",
               "....FFF.MM.FF.",
               "....FFFF..FFF.",
               "....FF........",
               ".....FF...FFF.",
               ".....FFF..FFF.",
               "......FF...FF.",
               "..............",
               "..FF.......FF.",
               "..FF..MMMM..F."}}}
        }};
        return fields[static_cast<int>(map)];
    }
    inline const JourneyBattlefield& BattlefieldDefinition(int mission)
    { return BattlefieldDefinition(SelectJourneyMap(mission,false,JourneyConvoyRoute::Road,false,CoalitionPlan::None,true)); }

    inline Terrain BattlefieldTerrain(JourneyMap map,int x,int y,bool legacyRescue=false)
    {
        if (x < 1 || x > 14 || y < 1 || y > 12) return Terrain::Sea;
        const char cell = BattlefieldDefinition(map).rows[y - 1][x - 1];
        if (cell == 'F' || (map==JourneyMap::Retreat && legacyRescue && y==12 && x>=7 && x<=10)) return Terrain::Forest;
        if (cell == 'M') return Terrain::Mountain;
        if (cell == '~') return Terrain::River;
        if (cell == '=') return Terrain::Bridge;
        return Terrain::Plain;
    }
    inline Terrain BattlefieldTerrain(int mission,int x,int y,bool rescue=false)
    { return BattlefieldTerrain(SelectJourneyMap(mission,false,JourneyConvoyRoute::Road,false,CoalitionPlan::None,true),x,y,rescue); }
}
