#pragma once
#include "CoreMinimal.h"

enum class ETacticalCommand : uint8
{
    Move, Strike, PrepareAmbush, Hold, QueueTactic, Execute, EndTurn,
    Sample, Clear, Continue, Restart, Cancel, Skip,
    Dialogue, Reinforce, PlanAmbush, PlanSupplies, Deploy, Retry,
    PromiseRoad, PromiseRescue, Withdraw, TalkLiu, TalkGuan, TalkZhang,
    RealtimeMode, RealtimePause, RealtimeSpeed, RealtimeStep, RealtimeStop, RealtimeChainSignal, RealtimeAlertPause,
    RealtimeUndoWaypoint, RealtimeArrival, RouteRoad, RouteSouthernWood
};
enum class ETacticalInput : uint8 { Select, Move, Lure };

struct FOfficerPresentation
{
    FString Name, Status, Tactic, Command;
    bool bLiving = false, bSelected = false;
    int Tile = -1;
};
struct FOrderPresentation
{
    FString Title, Detail, ActorName, TacticName;
    int Actor = -1, Target = -1;
    bool bSuccess = false, bEvaluated = false;
};
struct FPreparationPresentation
{
    FString Title, Description, Cost, Remaining;
    bool bSelected = false, bAffordable = false;
};
struct FTacticOptionPresentation
{
    int Id = 0;
    FString Name, Description, Cost;
    bool bSelected = false, bUnique = false, bUsed = false;
};
struct FJourneyPresentation
{
    FString Heading, Speaker, Dialogue, Resources, Outcome, Relationships, Context, PrimaryLabel, PromiseText, History;
    TArray<FPreparationPresentation> Plans;
    int Portrait = 0;
    FString ConvoyRouteInfo,ReviewFacts,ReviewLesson;
    bool bChooseRoute=false,bSouthernRoute=false;
    bool bVisible = false, bPreparation = false, bDebrief = false, bComplete = false, bChoosePromise = false, bRescue = false;
};
struct FEnemyReadPresentation
{
    int Tile = -1;
    FString Name, Intent, Counter;
    bool bSelected = false;
};
struct FBattleNoticePresentation
{
    int Army=-1;
    FString Title,Detail;
};
struct FMarchPointPresentation
{
    int Tile=-1;
    FString Label;
};
enum class EBattleCue : uint8 { March, Strike, Lure, Ambush, Breakthrough, Guard, Conceal, Enemy, Stop, Withdraw, Feint, Disrupt, Rally, AzureDragon, SerpentRoar };
struct FVisualUnit
{
    int Id = -1, Troops = 0, Loss = 0;
    int Morale = 0;
    FString Name;
    FVector Position = FVector::ZeroVector;
    bool bEnemy = false, bConcealed = false, bGuarded=false;
    bool bDisordered = false, bPressured = false, bInspired = false;
    float Motion = 0;
};
struct FBattleBeat
{
    TArray<FVisualUnit> Before, After;
    EBattleCue Kind = EBattleCue::March;
    int Actor = -1, Target = -1;
    FString Title, Reason;
    float Duration = 2.4f;
    bool bSignature = false;
    int ChainStep=0;
};
struct FTacticCinematicRead
{
    FString Officer,Tactic,Voice,Chain,Primary,Secondary;
    FLinearColor Accent=FLinearColor::White;
    int Actor=-1,Target=-1;
    float Intro=0,Impact=0,CameraWeight=0;
    bool bVisible=false,bResolved=false,bHit=false;
};
// UI receives observed information and cached predictions only.
struct FTacticalPresentation
{
    FString Title, Objective, Target, Hint, Report, NextLabel, WithdrawLabel;
    FString ClockLabel, StandingOrderLabel, SpeedLabel;
    FString ChainSignalLabel;
    bool bChainSignalArmed=false,bChainSignalReady=false;
    float ClockProgress=0;
    bool bRealtime=false,bRealtimePaused=true;
    bool bAlertPause=true;
    TArray<FBattleNoticePresentation> BattleNotices;
    TArray<FMarchPointPresentation> MarchPoints;
    TArray<int> MarchRoute;
    TArray<int> ConvoyRoute;
    FString MarchSummary,ArrivalLabel;
    TArray<FOfficerPresentation> Officers;
    TArray<FOrderPresentation> Orders;
    TArray<FTacticOptionPresentation> Tactics;
    TArray<FEnemyReadPresentation> EnemyReads;
    TArray<int> ThreatCells;
    int ThreatFrom = -1, ThreatNext = -1;
    FString EnemyCounter;
    bool bThreatOverlay = true;
    FString SelectedTacticName;
    TArray<TPair<int, int>> Routes;
    FJourneyPresentation Journey;
    int Predicted = 0, Selected = 0;
    int GoalTile = -1, SecondGoalTile = -1;
    FString GoalLabel;
    bool bBattle = false, bWon = false, bComplete = false, bBusy = false;
};
