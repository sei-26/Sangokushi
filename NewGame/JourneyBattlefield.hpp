#pragma once
#include "CampaignTypes.hpp"
#include <algorithm>
#include <array>
#include <string_view>

namespace frontline
{
    // Authored battle compositions, shared by navigation, sight, AI and rendering.
    // Rows correspond to playable y=1..12, columns to x=1..14.
    // . plain, F forest, M mountain (cover/LOS), ~ impassable river, = bridge.
    struct JourneyBattlefield
    {
        const char32_t* name;
        const char32_t* reading;
        std::array<std::string_view, 12> rows;
    };
    inline const JourneyBattlefield& BattlefieldDefinition(int mission)
    {
        static const std::array<JourneyBattlefield, 3> fields{{
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
               "..MMM.......MM"}}}
        }};
        return fields[std::clamp(mission, 0, 2)];
    }
    inline Terrain BattlefieldTerrain(int mission, int x, int y, bool rescue = false)
    {
        if (x < 1 || x > 14 || y < 1 || y > 12) return Terrain::Sea;
        const char cell = BattlefieldDefinition(mission).rows[y - 1][x - 1];
        if (cell == 'F' || (mission == 0 && rescue && y == 12 && x >= 7 && x <= 10)) return Terrain::Forest;
        if (cell == 'M') return Terrain::Mountain;
        if (cell == '~') return Terrain::River;
        if (cell == '=') return Terrain::Bridge;
        return Terrain::Plain;
    }
}
