#pragma once
#include "TacticalPresentation.h"
#include "StrategistJourney.hpp"
#include "JourneyRealtime.hpp"
#include "JourneyChainSignal.hpp"
#include "JourneyBattleWatch.hpp"
#include "JourneyBattleChoices.hpp"
#include "JourneySupport.hpp"
#include "JourneySupportActions.hpp"
#include "TacticalMissionResponse.h"
#include <memory>

struct FRetryDefenseAid
{
    int Turn=-1,Supporter=-1,Guard=-1,Goal=-1,Target=-1;
    bool bDefeated=false;
};

struct FJourneyState
{
    frontline::StrategistJourney Journey;
    frontline::JourneyRealtime Realtime;
    frontline::JourneyChainSignal ChainSignal;
    frontline::ChainSignalRead SignalRead;
    frontline::JourneyBattleWatch BattleWatch;
    std::array<frontline::JourneyOrderRead,3> OrderReads{};
    bool bWatchPending=false,bWatchPauseAfterPlayback=false;
    bool bAutoBattle=false,bAutoCyclePending=false; // Explicit delegation; saves resume in manual control.
    bool bRealtimeInBattle=false,bRealtimeStepping=false,bRealtimeChainRequested=false,bRealtimePauseAfterPlayback=false;
    TArray<TPair<int,int>> RealtimeRoutes;
    std::vector<frontline::ChainOrder> Orders;
    std::vector<int> BattleChoiceKey;
    std::vector<frontline::JourneyBattleChoice> BattleChoices;
    int BattleChoiceVersion=0; // Display-only generation; never saved.
    std::vector<frontline::JourneySupportAction> SupportActions;
    std::vector<int> SupportActionKey;
    bool bSupportBattleChoices=false;
    std::vector<int> SupportKey;
    std::vector<frontline::JourneySupportChoice> SupportChoices;
    int SupportVersion=0;
    std::array<frontline::ChainTactic, 3> SelectedTactics{frontline::ChainTactic::Lure, frontline::ChainTactic::Ambush, frontline::ChainTactic::Breakthrough};
    int Selected = 0, Target = 3;
    ETacticalInput Input = ETacticalInput::Select;
    int ComparedRelief=-1; // Hypothetical successor; no automatic or persisted handover.
    int ComparedPosition=-1; // Transient map selection; never a saved order.
    bool bConfirmEndTurn = false, bConfirmWithdrawal = false;
    bool bSupplyPlayback=false; // Supply feedback advances while the tactical clock stays paused.
    bool bThreatOverlay = true;
    FTacticalPresentation HUD;
    TArray<int> Reachable;
    TArray<FLinearColor> FoliageClearings;
    FVector ActionCamera=FVector::ZeroVector;
    bool bHasActionCamera=false; // Display-only: never part of a journey save.
    TArray<int32> TerrainSnapshot, LandscapeSnapshot, BackdropSnapshot;
    TArray<FVector> LandscapePeaks, LandscapeRivers, LandscapeRoad;
    TArray<TPair<FVector,FVector>> LandscapeRidges;
    FString Report;
    TArray<FBattleBeat> Beats;
    TArray<FVisualUnit> Units;
    int BeatIndex = 0;
    float BeatTime = 0, VisualTime = 0;
    bool bNoPlayback = false, bImpactPlayed = false;
    gunyuresponse::FRead MissionResponse;
    float MissionResponseRemaining=0;
    FRetryDefenseAid RetryDefenseAid; // Observed solo support awaiting this cycle's real defense check.
    int RoadClearArmy=-1,RoadClearStep=-1,RoadClearTile=-1; // Transient observed contribution, never saved.
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
