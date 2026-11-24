#include "TacticalTrial.h"
#include "TacticalState.h"
#include "JourneyEpisode.hpp"
#include "Components/InstancedStaticMeshComponent.h"
using namespace frontline;

bool ATacticalField::PrepareGrandBattlePreview(int Mission,int Phase,int Approach)
{
    if((Mission!=YellowTurbanMission && Mission!=YangrenMission && Mission!=CoalitionMission) || Phase<0 || Phase>4 ||
        (Phase==3 && Mission!=YangrenMission && Mission!=YellowTurbanMission)) return false;
    Issue(ETacticalCommand::Skip);
    bLegacyJourney=false;bGrandHistoricalBattles=true;bStartWithCouncil=false;bRealtimeCommand=true;
    FirstMission=0;State->bNoPlayback=true;bPauseOnBattleAlert=false;Issue(ETacticalCommand::Restart);
    auto& J=State->Journey;
    J.ResetCampaign();J.Meet();J.grandCampaign=true;J.mission=Mission;
    if(Mission==YangrenMission) J.coalitionPlan=CoalitionPlan::Advance;
    if(Mission==CoalitionMission) J.coalitionPlan=Approach==2 ? CoalitionPlan::Protect:CoalitionPlan::Advance;
    for(int Previous=0;Previous<Mission;++Previous)
        J.history[Previous].completed=J.history[Previous].won=J.history[Previous].campaignFlow=true;
    J.SelectDifficulty(JourneyDifficulty::Story);
    State->PreparationChoice=1;Refresh();
    if(Phase==4) { State->bNoPlayback=false;Refresh();return J.stage==JourneyStage::Preparation && (J.HasCoalitionBattle() || J.HasYangrenStrategy() || J.HasGrandStrategy()); }
    Issue(ETacticalCommand::Deploy);
    if(!J.HasGrandBattle() || VisualUnits().Num()!=GrandBattleArmyCount) return false;
    if(Approach>0 && Mission!=CoalitionMission && !UseGrandApproach(Approach-1)) return false;
    if(Phase>0)
    {
        Issue(ETacticalCommand::AutoBattle);
        for(int Cycle=0;Cycle<32 && J.stage==JourneyStage::Battle;++Cycle)
        {
            AdvanceRealtime(12.1f);AdvancePresentation(60);
            const auto* reserve=J.intelligence[0].Find(8);
            if(Phase==3 && J.battleReview.grand.yangren.counterOn && reserve && reserve->current) break;
            if(Phase==3 && Mission==YellowTurbanMission && J.battleReview.grand.relievedOn) break;
            AdvanceRealtime(.1f);AdvancePresentation(60);
            if(Phase==1 && (J.battleReview.grand.openedOn || J.battleReview.grand.coalition.rescuedOn)) break;
        }
        if(IsAutoBattle()) StopAutoBattle();
        if(Phase==1 && !J.battleReview.grand.openedOn && !J.battleReview.grand.coalition.rescuedOn) return false;
        if(Phase==2 && !J.won) return false;
        if(Phase==3 && Mission==YangrenMission && (!J.battleReview.grand.yangren.counterOn || J.battleReview.grand.yangren.repelledOn)) return false;
        if(Phase==3 && Mission==YellowTurbanMission && !J.battleReview.grand.relievedOn) return false;
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
    if(!FParse::Param(FCommandLine::Get(),TEXT("GunyuGuangzongReadOnly")))
    {
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
    if(!Check(PrepareGrandBattlePreview(YangrenMission,4),TEXT("modern Yangren preparation opens without deploying"))) return false;
    const auto& Sites=State->HUD.Journey.Map.Sites;
    if(!Check(Sites.ContainsByPredicate([](const FPlanningSite& S){return S.Name==TEXT("義勇軍が守る辻");}) &&
        Sites.ContainsByPredicate([](const FPlanningSite& S){return S.Tile==YangrenRally && S.Name==TEXT("空けておく合流点");}),
        TEXT("the council distinguishes the heroes' crossroads from Sun Jian's arrival cell"))) return false;
    for(int Approach:{1,2})
    {
        if(!Check(PrepareGrandBattlePreview(YangrenMission,0,Approach),TEXT("each real Yangren approach deploys"))) return false;
        if(!Check(State->HUD.MissionAdvice.Contains(Approach==1 ? TEXT("森"):TEXT("南道")) &&
            (Approach!=1 || (State->HUD.GrandAdvanceRoute.IsEmpty() && State->HUD.SecondGoalLabel.Contains(TEXT("合図待ち")))),
            TEXT("the chosen opening has its own advice and forest waiting does not invent a march"))) return false;
        for(int Tile:{State->Journey.goal,YangrenRally,SunJianStart(false),SunJianStart(true)})
        {
            const FVector Offset(400,300,1500);
            if(!Check(RoadDistance(TileLocation(Tile))<.01f && PickSurfaceTile(SurfaceLocation(Tile)+Offset,-Offset.GetSafeNormal())==Tile,
                TEXT("the modern south street joins both possible starts, the rally and crossroads without changing picking"))) return false;
        }
        for(int Tile:State->HUD.GrandAdvanceRoute)
            if(!Check(Tile!=State->Journey.goal,TEXT("Sun Jian's forecast avoids the heroes' crossroads, just like his real march"))) return false;
        for(int Tile:{State->Journey.goal,YangrenRally})
            for(int I=0;I<Buildings->GetInstanceCount();++I)
            {
                FTransform T;Buildings->GetInstanceTransform(I,T,false);
                if(!Check(FVector::Dist2D(T.GetLocation(),SurfaceLocation(Tile))>=65 || T.GetLocation().Z<55,
                    TEXT("neither friendly site has an enemy command-post roof in its formation center"))) return false;
            }
        std::vector<std::uint8_t> Before,After;
        if(!ExportSession(Before)) return false;
        State->TerrainSnapshot.Reset();UpdateRoadTopology();BuildTerrain();Refresh();
        if(!Check(ExportSession(After) && Before==After,TEXT("Yangren scenery and guidance cannot change budgets, orders or history"))) return false;
        State->Journey.world.armies[5].tile=YangrenRally;Refresh();
        if(!Check(State->HUD.SecondGoalLabel.Contains(TEXT("辻を待つ")) && State->HUD.MissionAdvice.Contains(TEXT("辻を確保")) &&
            State->HUD.GrandAdvanceRoute.IsEmpty(),TEXT("an arrived-main-army presentation fixture asks the heroes to secure the crossroads, not clear an imaginary march"))) return false;
        if(!ImportSession(Before)) return false;
        if(!Check(PrepareGrandBattlePreview(YangrenMission,3,Approach) && State->HUD.bYangrenCounter &&
            State->HUD.MissionAdvice.Contains(TEXT("辻")),TEXT("both actual approaches reach the cavalry response while protecting Sun Jian and the crossroads"))) return false;
        if(!ExportSession(Before)) return false;
        const auto* Cavalry=State->Journey.intelligence[0].Find(8);
        if(!Check(Cavalry && Cavalry->current && State->HUD.MissionFocusTile==Cavalry->observed.tile,TEXT("counterattack focus uses the observed cavalry"))) return false;
        for(auto& C:State->Journey.intelligence[0].contacts) if(C.id==8) C.current=false;
        Refresh();
        if(!Check(State->HUD.MissionFocusTile==State->Journey.world.armies[5].tile,TEXT("unseen cavalry focuses the friendly main army, not its private enemy location"))) return false;
        State->Journey.battleReview.grand.yangren.repelledOn=State->Journey.turn;Refresh();
        if(!Check(!State->HUD.bYangrenCounter && State->HUD.MissionFocusTile==State->Journey.goal &&
            State->HUD.MissionAdvice.Contains(TEXT("確保し直")),TEXT("a presentation-only repelled fixture returns the instruction to securing the crossroads"))) return false;
        if(!ImportSession(Before)) return false;
        if(!Check(PrepareGrandBattlePreview(YangrenMission,2,Approach) && State->Journey.won,
            TEXT("both approaches still win through actual autonomous combat"))) return false;
    }
    UE_LOG(LogTemp,Display,TEXT("GUNYU_YANGREN_READ: PASS (distinct openings, real modern road, open friendly sites, immutable scenery, actual counters and victories, public focus and settled-goal guidance)"));
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
    if(!Check(PrepareGrandBattlePreview(YellowTurbanMission,4),TEXT("modern Guangzong opens its preparation without deploying"))) return false;
    const auto& Sites=State->HUD.Journey.Map.Sites;
    if(!Check(Sites.Num()==5 && Sites.ContainsByPredicate([](const FPlanningSite& S){return S.Tile==GrandLeftAnchor && S.Name==TEXT("北で耐える左翼");}) &&
        Sites.ContainsByPredicate([](const FPlanningSite& S){return S.Tile==Campaign::At(8,8) && S.Name==TEXT("中央橋");}) &&
        Sites.ContainsByPredicate([](const FPlanningSite& S){return S.Tile==Campaign::At(8,11) && S.Name==TEXT("南渡し");}),
        TEXT("the council has one northern relief and two real crossings, without the obsolete evacuation or port entries"))) return false;
    const auto Camp=State->Journey.world.cities[0];
    Issue(ETacticalCommand::PlanExample);
    if(!Check(State->Journey.assignments[1].duty==JourneyDuty::Ambush &&
        State->Journey.PlanningTerrain(State->Journey.assignments[1].tile)==Terrain::Forest &&
        State->Journey.world.cities[0].gold==Camp.gold && State->Journey.world.cities[0].food==Camp.food,
        TEXT("the actual preparation example applies in forest rather than failing on the new central bridge, without spending resources"))) return false;
    for(int Approach:{1,2})
    {
        if(!Check(PrepareGrandBattlePreview(YellowTurbanMission,0,Approach),TEXT("both Guangzong approaches deploy"))) return false;
        if(!Check(State->HUD.MissionAdvice.Contains(Approach==1 ? TEXT("北陣"):TEXT("中央橋")) &&
            State->HUD.Crossings[0].Tile==Campaign::At(8,8) && State->HUD.Crossings[1].Tile==Campaign::At(8,11),
            TEXT("the chosen approach has its own instruction and both actual bridges are labelled from the opening"))) return false;
        for(int Tile:{Campaign::At(4,6),GrandLeftAnchor,GrandNorthPass,Campaign::At(8,8),Campaign::At(8,11),State->Journey.goal})
        {
            const FVector Offset(400,300,1500);
            if(!Check(RoadDistance(TileLocation(Tile))<.01f && PickSurfaceTile(SurfaceLocation(Tile)+Offset,-Offset.GetSafeNormal())==Tile,
                TEXT("the northern road and both crossings connect real sites without changing their selection surface"))) return false;
        }
        const auto Plans=State->Journey.PlanGrandAllies();
        const auto Main=std::find_if(Plans.begin(),Plans.end(),[](const JourneyEnemyPlan& P){return P.army==5;});
        if(!Check(Main!=Plans.end() && State->HUD.GrandAdvanceRoute.Num()>1 && State->HUD.GrandAdvanceRoute[1]==Main->next,
            TEXT("the displayed first step is the main army's actual public movement plan"))) return false;
        std::vector<std::uint8_t> Before,After;
        if(!ExportSession(Before)) return false;
        State->TerrainSnapshot.Reset();UpdateRoadTopology();BuildTerrain();Refresh();
        if(!Check(ExportSession(After) && Before==After,TEXT("scenery and guidance cannot alter combat, budgets or history"))) return false;
        State->Journey.chain.disordered[5]=true;Refresh();
        if(!Check(State->HUD.GrandAdvanceRoute.IsEmpty() && State->HUD.GrandRouteLegend.Contains(TEXT("混乱")),
            TEXT("a disordered-main-army presentation fixture does not invent a march"))) return false;
        if(!ImportSession(Before)) return false;
        if(!Check(PrepareGrandBattlePreview(YellowTurbanMission,2,Approach) && State->Journey.won,
            TEXT("both real approaches still reach victory through autonomous combat"))) return false;
    }
    if(!Check(PrepareGrandBattlePreview(YellowTurbanMission,3,1) && State->Journey.stage==JourneyStage::Battle &&
        State->Journey.world.armies[6].troops>0 && State->Journey.battleReview.grand.relievedOn>0,
        TEXT("the actual northern relief can be inspected while the surviving left wing is still in battle"))) return false;
    std::vector<std::uint8_t> Relief;
    if(!ExportSession(Relief)) return false;
    State->Journey.world.armies[6].troops=0;Refresh();
    if(!Check(State->HUD.MissionStep.Contains(TEXT("救援後に壊滅")),TEXT("a later-loss presentation fixture does not call the wing a survivor"))) return false;
    if(!ImportSession(Relief)) return false;
    State->Journey.battleReview.grand.openedOn=State->Journey.turn;State->Journey.battleReview.grand.ralliedOn=0;Refresh();
    if(!Check(State->HUD.MissionFocusTile==State->Journey.world.armies[5].tile && State->HUD.MissionAdvice.Contains(TEXT("本隊を通")),
        TEXT("the opened-but-not-rallied presentation fixture directs attention to the friendly main army"))) return false;
    State->Journey.world.armies[GrandCommander].troops=0;State->Journey.battleReview.grand.ralliedOn=State->Journey.turn;Refresh();
    if(!Check(State->HUD.MissionFocusTile==State->Journey.goal && State->HUD.MissionAdvice.Contains(TEXT("確保し直")),
        TEXT("a broken-headquarters presentation fixture returns the heroes to securing their front"))) return false;
    if(!ImportSession(Relief)) return false;
    UE_LOG(LogTemp,Display,TEXT("GUNYU_GUANGZONG_READ: PASS (usable real preparation, two openings and victories, northern road and crossings, actual public march, immutable scenery, relief and remaining-goal guidance)"));
    bLegacyJourney=Legacy;bStartWithCouncil=Council;bRealtimeCommand=Mode;bGrandHistoricalBattles=Grand;
    bPauseOnBattleAlert=Alerts;FirstMission=Mission;State->bNoPlayback=Playback;Issue(ETacticalCommand::Restart);
    UE_LOG(LogTemp,Display,TEXT("GUNYU_GRAND_BATTLE: PASS (eleven formations, autonomous wings, actual breach and general assault, headquarters victory, council, aftermath, save/reload)"));
    return true;
}
