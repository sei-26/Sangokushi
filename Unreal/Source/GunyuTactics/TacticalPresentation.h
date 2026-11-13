#pragma once
#include "TacticalPreparationMap.h"
#include "CoreMinimal.h"

enum class ETacticalCommand : uint8
{
    Move, Strike, PrepareAmbush, Hold, QueueTactic, Execute, EndTurn, AutoBattle,
    Sample, Clear, Continue, Restart, Cancel, Skip,
    Dialogue, Reinforce, PlanAmbush, PlanSupplies, Deploy, Retry,
    PromiseRoad, PromiseRescue, Withdraw, TalkLiu, TalkGuan, TalkZhang,
    RealtimeMode, RealtimePause, RealtimeSpeed, RealtimeStep, RealtimeStop, RealtimeChainSignal, RealtimeAlertPause,
    RealtimeUndoWaypoint, RealtimeArrival, RouteRoad, RouteSouthernWood, StoryReplyPreserve, StoryReplyCoordinate, DifficultyStory, DifficultyStandard, PlanExample, Resupply, ReleaseGuard, ResumeDuty,
    CoalitionAdvance, CoalitionProtect
};
enum class ETacticalInput : uint8 { Select, Move, Lure, Position };

struct FOfficerPresentation
{
    FString Name, Status, Tactic, Command, Duty, DutyBadge;
    bool bLiving = false, bSelected = false;
    int Tile = -1, DutyTile = -1;
};
struct FOrderPresentation
{
    FString Title, Detail, ActorName, TacticName, Combo;
    int Actor = -1, Target = -1;
    bool bSuccess = false, bEvaluated = false;
};
struct FTacticTargetPresentation
{
    int Army=-1, Tile=-1, Step=0, ComboPercent=0, Damage=0, RetreatChoices=0;
    FString Badge, Detail;
    bool bReady=false, bSelected=false;
};
struct FTacticPositionCell
{
    int Tile=-1, Step=0, Damage=0, ComboPercent=0;
    bool bMoveAllowed=false, bReady=false;
    FString Result, Risk;
};
struct FReliefOfficerRead
{
    int Army=-1, Tile=-1;
    FString Name, Status, Detail, Readiness, Directive;
    bool bAdjacent=false, bMoveReady=false, bReserved=false;
    bool bCanIssue=false, bSecured=false;
};
struct FReliefApproachRead
{
    bool bVisible=false,bBlocked=false,bCanQueue=false;
    int Officer=-1,Staging=-1,Blocker=-1,BlockedTile=-1;
    FString Summary,Warning,ConfirmLabel;
    TArray<int> Route;
};
struct FTacticReliefRead
{
    bool bVisible=false, bCanIssue=false, bSecured=false;
    int Source=-1, Incoming=-1, IncomingTile=-1;
    FString Detail, Cost, ConfirmLabel, CurrentGuard, IncomingReadiness, Decision;
    bool bCanStopDeparture=false;
    TArray<FReliefOfficerRead> Officers;
    FReliefApproachRead Approach;
};
struct FTacticPositionComparison
{
    bool bVisible=false, bCanCommit=false;
    int SelectedTile=-1;
    FString Title, Result, Risk, Timing, ConfirmLabel;
    TArray<FTacticPositionCell> Cells;
    FTacticReliefRead Relief;
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
{ int Speaker=0;FString Name,Text,Direction;bool bProtagonist=false;int WarmMask=0; };
struct FCouncilStageLine
{
    FDialoguePresentation Dialogue;
    FString Reactions[3];
};
struct FCouncilStageExchange
{
    FString Subject;
    TArray<FCouncilStageLine> Lines;
};
struct FJourneyEpisodePresentation
{
    FString Key,Title,Place,Setting,Question;
    TArray<FDialoguePresentation> Lines;
};
struct FSupportChoicePresentation
{
    int Actor=-1,Goal=-1;
    FString Title,Outcome,Cost,Detail;
    bool bEnabled=false;
};
struct FJourneyPresentation
{
    FString Heading, Speaker, Dialogue, Resources, Outcome, Relationships, Context, PrimaryLabel, PromiseText, History;
    TArray<FPreparationPresentation> Plans;
    int Portrait = 0;
    FString ConvoyRouteInfo,ReviewFacts,ReviewLesson;
    TArray<FDialoguePresentation> Conversation;
    FString StoryTitle,StoryFacts,PersonalTrust,PledgeText,ReplyPrompt;
    FJourneyEpisodePresentation Episode,CoalitionEpisode;
    FString CoalitionChoices[2];
    FString PlanDifference,PlanReadiness,PlanObjective,PlanRoute;
    FString DifficultyName,DifficultyDetail,ExampleName,ExamplePurpose,ExampleManual,ExampleCaution,ExampleReason,ExampleWarning;
    int ExampleAdviser=-1;
    TArray<FPlanningDuty> ExampleDuties;
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
    int HistoricalSpeaker = -1;
    int Morale = 0;
    FString Name;
    FVector Position = FVector::ZeroVector;
    bool bEnemy = false, bConcealed = false, bGuarded=false;
    bool bDisordered = false, bPressured = false, bInspired = false;
    bool bTransport = false, bCavalry = false;
    float Motion = 0;
};
struct FBeaconDefenseProgress
{
    int Goal=-1,Guard=-1,Turn=0,Reason=0;
    bool bSecured=false,bGuarded=false,bAttackSpent=false;
};
struct FBeaconDefensePresentation
{
    FString Current,Last,Detail;
    int Goal=-1,Guard=-1;
    bool bReady=false,bLastSecured=false;
};
struct FMissionProgress
{
    int Convoy=-1,Total=0,Held=-1,Required=0;
    int ConvoyNext=-1,DirectedMovers=0;
    int CompletedDuties=0,PresentDuties=0,AmbushDuties=0;
    TArray<int> ConfirmedReliefs;
    int CommandTurn=0;
    int PrimaryGoal=-1,SecondaryGoal=-1;
    TArray<FBeaconDefenseProgress> DefenseChecks;
    bool bBattle=false,bRescue=false;
};
struct FBattleBeat
{
    TArray<FVisualUnit> Before, After;
    EBattleCue Kind = EBattleCue::March;
    int Actor = -1, Target = -1;
    FString Title, Reason, CouncilVoice;
    float Duration = 2.4f;
    bool bSignature = false;
    int ChainStep=0, Combo=0, ComboPercent=100;
    FMissionProgress MissionBefore,MissionAfter;
    int RoadClearedBy=-1,ReliefIncoming=-1,ReliefOutgoing=-1;
    bool bReliefRecovered=false;
    int SupportGoal=-1,SupportGuard=-1;
    bool SupportNorth=true;
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
    struct FBattleChoice
    {
        FString Title,Outcome,Cost,Detail;
        int Actor=-1;
    };
    TArray<FBattleChoice> BattleChoices;
    FString BattleChoiceReason,BattleChoiceTarget;
    int BattleChoiceVersion=0;
    FString MissionStep,MissionAdvice,MissionDetail;
    struct FDutyProgress
    {
        FString Caption,Detail,InspectLabel;
        int Tile=-1,InspectTile=-1;
        bool bCanResume=false,bTraffic=false;
    } DutyProgress;
    TArray<FBeaconDefensePresentation> Defenses;
    FString BattleChoicePrompt;
    int MissionFocusTile=-1;
    bool bConvoyMission=false;
    FString Title, Objective, Target, Hint, Report, NextLabel, WithdrawLabel, Difficulty;
    FString ClockLabel, StandingOrderLabel, SpeedLabel;
    FString ChainSignalLabel,ResupplyLabel;
    FString GuardContext,GuardHint,GuardButtonLabel,BattlefieldHint,EnemyPlanHint;
    bool bGuardOrder=false;
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
    TArray<FSupportChoicePresentation> SupportChoices;
    int SupportVersion=0;
    TArray<FOrderPresentation> Orders;
    TArray<FTacticOptionPresentation> Tactics;
    TArray<FEnemyReadPresentation> EnemyReads;
    TArray<int> ThreatCells;
    int ThreatFrom = -1, ThreatNext = -1;
    FString EnemyCounter;
    bool bThreatOverlay = true;
    FString SelectedTacticName, TacticAvailability;
    bool bTacticUnavailable=false;
    FTacticPositionComparison Position;
    TArray<FTacticTargetPresentation> TacticTargets;
    FString TacticTargetSummary;
    int ReadyTacticTargets=0;
    bool bTacticTargetOverlay=false;
    TArray<TPair<int, int>> Routes;
    FJourneyPresentation Journey;
    int Predicted = 0, Selected = 0;
    int GoalTile = -1, SecondGoalTile = -1;
    int Mission = 0;
    FString GoalLabel,SecondGoalLabel;
    bool bBattle = false, bWon = false, bComplete = false, bBusy = false;
};
