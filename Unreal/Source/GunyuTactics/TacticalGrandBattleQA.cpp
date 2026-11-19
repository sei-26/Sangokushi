#include "TacticalTrial.h"
#include "TacticalState.h"
#include "JourneyEpisode.hpp"
using namespace frontline;

bool ATacticalField::PrepareGrandBattlePreview(int Mission,int Phase,int Approach)
{
    if((Mission!=YellowTurbanMission && Mission!=YangrenMission) || Phase<0 || Phase>3 || (Phase==3 && Mission!=YangrenMission)) return false;
    Issue(ETacticalCommand::Skip);
    bLegacyJourney=false;bGrandHistoricalBattles=true;bStartWithCouncil=false;bRealtimeCommand=true;
    FirstMission=0;State->bNoPlayback=true;bPauseOnBattleAlert=false;Issue(ETacticalCommand::Restart);
    auto& J=State->Journey;
    J.ResetCampaign();J.Meet();J.grandCampaign=true;J.mission=Mission;
    if(Mission==YangrenMission) J.coalitionPlan=CoalitionPlan::Advance;
    for(int Previous=0;Previous<Mission;++Previous)
        J.history[Previous].completed=J.history[Previous].won=J.history[Previous].campaignFlow=true;
    J.SelectDifficulty(JourneyDifficulty::Story);
    State->PreparationChoice=1;Refresh();Issue(ETacticalCommand::Deploy);
    if(!J.HasGrandBattle() || VisualUnits().Num()!=GrandBattleArmyCount) return false;
    if(Approach>0 && !UseGrandApproach(Approach-1)) return false;
    if(Phase>0)
    {
        Issue(ETacticalCommand::AutoBattle);
        for(int Cycle=0;Cycle<32 && J.stage==JourneyStage::Battle;++Cycle)
        {
            AdvanceRealtime(12.1f);AdvancePresentation(60);
            const auto* reserve=J.intelligence[0].Find(8);
            if(Phase==3 && J.battleReview.grand.yangren.counterOn && reserve && reserve->current) break;
            AdvanceRealtime(.1f);AdvancePresentation(60);
            if(Phase==1 && J.battleReview.grand.openedOn) break;
        }
        if(IsAutoBattle()) StopAutoBattle();
        if(Phase==1 && !J.battleReview.grand.openedOn) return false;
        if(Phase==2 && !J.won) return false;
        if(Phase==3 && (!J.battleReview.grand.yangren.counterOn || J.battleReview.grand.yangren.repelledOn)) return false;
    }
    State->bNoPlayback=false;Refresh();
    return true;
}

bool ATacticalField::GrandBattleSmokeTest()
{
    auto Check=[](bool Value,const TCHAR* Reason)
    { if(!Value) UE_LOG(LogTemp,Error,TEXT("GUNYU_GRAND_BATTLE: FAIL %s"),Reason);return Value; };
    const bool Legacy=bLegacyJourney,Council=bStartWithCouncil,Mode=bRealtimeCommand,Grand=bGrandHistoricalBattles;
    const bool Playback=State->bNoPlayback,Alerts=bPauseOnBattleAlert;
    const int Mission=FirstMission;
    for(int Battle:{YellowTurbanMission,YangrenMission})
    {
        if(!Check(PrepareGrandBattlePreview(Battle,0),TEXT("normal deployment creates the main army, both wings and all eleven formations"))) return false;
        auto& J=State->Journey;
        if(!Check(State->HUD.bGrandBattle && State->HUD.MissionStep.Contains(Battle==YangrenMission ? TEXT("孫堅"):TEXT("左翼")) &&
            BuildJourneyEpisode(*State->BeforeBattle).lines.size()>=6,TEXT("council and battle describe the same large engagement"))) return false;
        std::vector<std::uint8_t> Bytes;
        if(!Check(ExportSession(Bytes) && ImportSession(Bytes) && State->Journey.HasGrandBattle(),TEXT("the large battle saves and reloads paused"))) return false;
        if(!Check(PrepareGrandBattlePreview(Battle,1) && State->Journey.stage==JourneyStage::Battle &&
            State->HUD.MissionAdvice.Contains(Battle==YangrenMission ? TEXT("反撃"):TEXT("本陣")),TEXT("capturing the first objective starts the mission's second phase"))) return false;
        if(!Check(PrepareGrandBattlePreview(Battle,2) && State->Journey.history[Battle].review.grand.ralliedOn>0 &&
            (State->Journey.HasYangrenStrategy() ? State->Journey.YangrenCounterRepelled():State->Journey.GrandHeadquartersBroken()) && BuildJourneyEpisode(State->Journey).lines.size()>=4,
            TEXT("the autonomous clock reaches the headquarters victory and its aftermath"))) return false;
    }
    if(!Check(PrepareGrandBattlePreview(YellowTurbanMission,0) && State->HUD.bGrandOrdersPending && CanUseGrandApproach(),
        TEXT("the new Guangzong offers two main-army approaches at the opening"))) return false;
    const int HeroTile=State->Journey.world.armies[0].tile;
    if(!Check(UseGrandApproach(0) && !UseGrandApproach(1) && !State->HUD.bGrandOrdersPending &&
        State->Journey.GrandMainTarget()==GrandNorthPass && State->HUD.GrandAdvanceRoute.Num()>1 &&
        State->Journey.world.armies[0].tile==HeroTile && !State->Journey.chain.spent[0],
        TEXT("the explicit rescue redirects the main army, keeps heroes and budgets, and cannot be changed for free"))) return false;
    std::vector<std::uint8_t> StrategyBytes;
    if(!Check(ExportSession(StrategyBytes) && ImportSession(StrategyBytes) &&
        State->Journey.battleReview.grand.approach==GrandApproach::RelieveLeft,
        TEXT("the new approach and terrain survive save/reload"))) return false;
    if(!Check(PrepareGrandBattlePreview(YellowTurbanMission,2,1) && State->Journey.history[YellowTurbanMission].review.grand.relievedOn>0 &&
        State->Journey.history[YellowTurbanMission].review.grand.leftReturned>0,
        TEXT("the real automatic rescue defeats the north front, joins the left wing and brings it home"))) return false;
    bLegacyJourney=Legacy;bStartWithCouncil=Council;bRealtimeCommand=Mode;bGrandHistoricalBattles=Grand;
    bPauseOnBattleAlert=Alerts;FirstMission=Mission;State->bNoPlayback=Playback;Issue(ETacticalCommand::Restart);
    UE_LOG(LogTemp,Display,TEXT("GUNYU_GRAND_BATTLE: PASS (eleven formations, autonomous wings, actual breach and general assault, headquarters victory, council, aftermath, save/reload)"));
    return true;
}
