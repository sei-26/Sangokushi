#pragma once
#include <string_view>

namespace frontline::preview
{
    struct Mode { std::string_view name; int id; };
    inline constexpr Mode Modes[] = {
        {"journey-escort", 84},
        {"journey-defense", 85},
        {"journey-escort-compact", 86},
        {"journey-defense-progress", 87},
        {"journey-defense-compact", 88},
        {"journey-encounter", 77},
        {"journey-council", 78},
        {"journey-chain", 79},
        {"journey-cutin", 80},
        {"journey-result", 81},
        {"journey-counter", 82},
        {"journey-compact", 83},
        {"strategist-council", 69},
        {"strategist-operation", 74},
        {"strategist-scouting", 75},
        {"strategist-operation-compact", 76},
        {"strategist-battle", 70},
        {"strategist-compact", 71},
        {"strategist-result", 72},
        {"strategist-menu", 73},
        {"china-atlas-zoom", 67},
        {"china-atlas-zoom-compact", 68},
        {"china-atlas-route", 66},
        {"china-atlas", 63},
        {"china-atlas-city", 64},
        {"china-atlas-compact", 65},
        {"ui-settings", 61},
        {"ui-compact", 62},
        {"terrain-detail", 59},
        {"terrain-coast", 60},
        {"territory-world", 58},
        {"metropolis-map", 57},
        {"city-growth", 54},
        {"national-policy", 55},
        {"policy-compact", 56},
        {"domestic", 51},
        {"domestic-progress", 52},
        {"domestic-compact", 53},
        {"story-strike", 49},
        {"story-firestorm", 50},
        {"training-compact", 48},
        {"joint-training", 46},
        {"training-progress", 47},
        {"formation-info", 44},
        {"formation-map", 45},
        {"siege-overview", 43},
        {"battle-immersion", 41},
        {"city-drama", 42},
        {"story-intro", 38},
        {"story-cinema", 39},
        {"story-spectacle", 40},
        {"governance-compact", 37},
        {"governance", 36},
        {"info-city", 30},
        {"info-officer", 31},
        {"info-relations", 32},
        {"info-faction", 33},
        {"info-army", 34},
        {"info-records", 35},
        {"tactic-cutin", 28},
        {"tactic-fire", 29},
        {"officers", 27},
        {"world-wide", 26},
        {"regions", 24},
        {"battlefield", 23},
        {"ai-assembly", 22},
        {"story-depth", 19},
        {"battle-depth", 20},
        {"story-vow", 21},
        {"convoy", 17},
        {"convoy-plan", 18},
        {"assignments", 16},
        {"world", 14},
        {"world-front", 15},
        {"story-prepare", 12},
        {"story-links", 13},
        {"start", 11},
        {"story-event", 9},
        {"story-cutin", 10},
        {"story-menu", 4},
        {"story", 5},
        {"story-battle", 6},
        {"story-civil", 7},
        {"story-ending", 8},
        {"history", 2},
        {"tactics", 3},
    };

    // Unknown names retain the existing default preview.
    constexpr int FindMode(std::string_view name) noexcept
    {
        for (const auto& mode : Modes)
            if (mode.name == name) return mode.id;
        return 1;
    }

    constexpr bool IsCompact(int mode) noexcept
    {
        return mode == 37 || mode == 48 || mode == 53 || mode == 56 ||
               mode == 62 || mode == 65 || mode == 68 || mode == 71 || mode == 76 || mode == 83 || mode == 86 || mode == 88;
    }
}
