#pragma once
#include "JourneyRealtime.hpp"
#include "JourneyChainSignal.hpp"
#include "JourneyBattleWatch.hpp"
#include <optional>
#include <cstdint>

namespace frontline
{
    // The authoritative session only; render caches and in-flight animations are rebuilt.
    struct JourneySaveData
    {
        StrategistJourney journey;
        std::optional<StrategistJourney> beforeBattle;
        JourneyRealtime realtime;
        JourneyChainSignal signal;
        JourneyBattleWatch watch;
        std::vector<ChainOrder> orders;
        std::array<ChainTactic,3> tactics{ChainTactic::Lure,ChainTactic::Ambush,ChainTactic::Breakthrough};
        int selected=0,target=3,dialogue=0,preparation=0,interlocutor=0;
        bool realtimeMode=false,pauseOnAlert=true,threatOverlay=true;
    };
    constexpr std::size_t JourneySaveLimit=4*1024*1024;
    // Versioned, checksummed little-endian data. Failure never mutates the destination.
    bool EncodeJourneySave(const JourneySaveData&,std::vector<std::uint8_t>&);
    bool DecodeJourneySave(const std::vector<std::uint8_t>&,JourneySaveData&);
    bool ValidJourneySave(const JourneySaveData&);
}
