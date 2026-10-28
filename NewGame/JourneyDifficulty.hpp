#pragma once
namespace frontline
{
    enum class JourneyDifficulty { Standard, Story };
    struct JourneyCombatSettings { int enemyPercent=100,extraCycles=0; };
    inline JourneyCombatSettings CombatSettings(JourneyDifficulty difficulty)
    { return difficulty==JourneyDifficulty::Story ? JourneyCombatSettings{80,3}:JourneyCombatSettings{}; }
    inline const char32_t* DifficultyName(JourneyDifficulty difficulty)
    { return difficulty==JourneyDifficulty::Story ? U"物語重視":U"標準"; }
}
