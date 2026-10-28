#pragma once
#include "TacticalPresentation.h"
#include "StrategistJourney.hpp"
#include "JourneyRealtime.hpp"
#include "JourneyChainSignal.hpp"
#include "JourneyBattleWatch.hpp"
#include <memory>

struct FJourneyState
{
    frontline::StrategistJourney Journey;
    frontline::JourneyRealtime Realtime;
    frontline::JourneyChainSignal ChainSignal;
    frontline::ChainSignalRead SignalRead;
    frontline::JourneyBattleWatch BattleWatch;
    std::array<frontline::JourneyOrderRead,3> OrderReads{};
    bool bWatchPending=false,bWatchPauseAfterPlayback=false;
    bool bRealtimeInBattle=false,bRealtimeStepping=false,bRealtimeChainRequested=false,bRealtimePauseAfterPlayback=false;
    TArray<TPair<int,int>> RealtimeRoutes;
    std::vector<frontline::ChainOrder> Orders;
    std::array<frontline::ChainTactic, 3> SelectedTactics{frontline::ChainTactic::Lure, frontline::ChainTactic::Ambush, frontline::ChainTactic::Breakthrough};
    int Selected = 0, Target = 3;
    ETacticalInput Input = ETacticalInput::Select;
    bool bConfirmEndTurn = false, bConfirmWithdrawal = false;
    bool bSupplyPlayback=false; // Supply feedback advances while the tactical clock stays paused.
    bool bThreatOverlay = true;
    FTacticalPresentation HUD;
    TArray<int> Reachable;
    TArray<FLinearColor> FoliageClearings;
    TArray<int32> TerrainSnapshot, LandscapeSnapshot, BackdropSnapshot;
    TArray<FVector> LandscapePeaks, LandscapeRivers, LandscapeRoad;
    TArray<TPair<FVector,FVector>> LandscapeRidges;
    FString Report;
    TArray<FBattleBeat> Beats;
    TArray<FVisualUnit> Units;
    int BeatIndex = 0;
    float BeatTime = 0, VisualTime = 0;
    bool bNoPlayback = false, bImpactPlayed = false;
    int CheckpointStage=-1,CheckpointMission=-1;
    int DialogueIndex = 0, PreparationChoice = 0, Interlocutor = 0;
    std::unique_ptr<frontline::StrategistJourney> BeforeBattle;
};
inline FString JourneyText(const std::u32string& Value)
{
    FString Result;
    for (char32_t Code : Value)
    {
        if (Code <= 0xffff) Result.AppendChar(static_cast<TCHAR>(Code));
        else
        {
            const uint32 Pair = static_cast<uint32>(Code) - 0x10000;
            Result.AppendChar(static_cast<TCHAR>(0xd800 + (Pair >> 10)));
            Result.AppendChar(static_cast<TCHAR>(0xdc00 + (Pair & 0x3ff)));
        }
    }
    return Result;
}
