#pragma once
#include "TacticalPreparationMap.h"
#include "CoreMinimal.h"

enum class ETacticalCommand : uint8
{
    Move, Strike, PrepareAmbush, Hold, QueueTactic, Execute, EndTurn,
    Sample, Clear, Continue, Restart, Cancel, Skip,
    Dialogue, Reinforce, PlanAmbush, PlanSupplies, Deploy, Retry,
    PromiseRoad, PromiseRescue, Withdraw, TalkLiu, TalkGuan, TalkZhang,
    RealtimeMode, RealtimePause, RealtimeSpeed, RealtimeStep, RealtimeStop, RealtimeChainSignal, RealtimeAlertPause,
    RealtimeUndoWaypoint, RealtimeArrival, RouteRoad, RouteSouthernWood, StoryReplyPreserve, StoryReplyCoordinate, DifficultyStory, DifficultyStandard, PlanExample, Resupply
};
enum class ETacticalInput : uint8 { Select, Move, Lure };

struct FOfficerPresentation
{
    FString Name, Status, Tactic, Command, Duty;
    bool bLiving = false, bSelected = false;
    int Tile = -1;
};
struct FOrderPresentation
{
    FString Title, Detail, ActorName, TacticName, Combo;
    int Actor = -1, Target = -1;
    bool bSuccess = false, bEvaluated = false;
};
struct FPreparationPresentation
{
    FString Title, Description, Tradeoff, Cost, Remaining, MoraleText, Shortage;
    int Food=0,Gold=0,MinimumTroops=0,MissingFood=0,MissingGold=0,MissingTroops=0;
    TArray<int> Morale;
    bool bSelected = false, bAffordable = false;
};
struct FTacticOptionPresentation
{
    int Id = 0;
    FString Name, Description, Cost;
    bool bSelected = false, bUnique = false, bUsed = false;
};
struct FDialoguePresentation
{ int Speaker=0;FString Name,Text,Direction;bool bProtagonist=false; };
struct FJourneyEpisodePresentation
{
    FString Key,Title,Place,Setting,Question;
    TArray<FDialoguePresentation> Lines;
};
struct FJourneyPresentation
{
    FString Heading, Speaker, Dialogue, Resources, Outcome, Relationships, Context, PrimaryLabel, PromiseText, History;
    TArray<FPreparationPresentation> Plans;
    int Portrait = 0;
    FString ConvoyRouteInfo,ReviewFacts,ReviewLesson;
    TArray<FDialoguePresentation> Conversation;
    FString StoryTitle,StoryFacts,PersonalTrust,PledgeText,ReplyPrompt;
    FJourneyEpisodePresentation Episode;
    FString PlanDifference,PlanReadiness,PlanObjective,PlanRoute;
    FString DifficultyName,DifficultyDetail,ExampleName,ExamplePurpose,ExampleManual,ExampleCaution;
    TArray<FString> ExampleOrders;
    bool bStoryDifficulty=false,bCampaignFlow=false;
    FString CampaignPhase,CampaignChange,CampaignNext,CampaignStanding;
    FPreparationMap Map;
    TArray<FDialoguePresentation> PlanAdvice;
    bool bNeedsReply=false;
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
    int ChainStep=0, Combo=0, ComboPercent=100;
};
struct FTacticCinematicRead
{
    FString Officer,Tactic,Voice,Chain,Primary,Secondary,NextOfficer;
    FLinearColor Accent=FLinearColor::White;
    int Actor=-1,Target=-1,NextActor=-1;
    float Intro=0,Impact=0,CameraWeight=0,Relay=0;
    bool bVisible=false,bResolved=false,bHit=false;
};
// UI receives observed information and cached predictions only.
struct FTacticalPresentation
{
    FString Title, Objective, Target, Hint, Report, NextLabel, WithdrawLabel, Difficulty;
    FString ClockLabel, StandingOrderLabel, SpeedLabel;
    FString ChainSignalLabel,ResupplyLabel;
    bool bHasResupplyPlan=false;
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
    FString SelectedTacticName, TacticAvailability;
    bool bTacticUnavailable=false;
    TArray<TPair<int, int>> Routes;
    FJourneyPresentation Journey;
    int Predicted = 0, Selected = 0;
    int GoalTile = -1, SecondGoalTile = -1;
    int Mission = 0;
    FString GoalLabel,SecondGoalLabel;
    bool bBattle = false, bWon = false, bComplete = false, bBusy = false;
};
