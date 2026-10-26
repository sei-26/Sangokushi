#include "TacticalTrial.h"
#include "TacticalState.h"
using namespace frontline;

void ATacticalField::PrepareOpeningPreview(bool bDebrief)
{
    bStartWithCouncil=true;FirstMission=0;EnemyPolicy=1;bRealtimeCommand=false;Issue(ETacticalCommand::Restart);
    for (int I=0;I<3;++I) Issue(ETacticalCommand::Dialogue);
    Issue(ETacticalCommand::PromiseRescue);Issue(ETacticalCommand::RouteSouthernWood);Issue(ETacticalCommand::TalkGuan);
    if (!bDebrief) return;
    const bool SavedPlayback=State->bNoPlayback;State->bNoPlayback=true;Issue(ETacticalCommand::Deploy);
    auto& J=State->Journey;JourneyRealtime escort;bool passedForest=false,reserveHandled=false;
    escort.Queue(J,2,JourneyDirective::March,Campaign::At(8,12));
    auto ReserveTactic=[this](int Army,ChainTactic Tactic,int EnemyTile)
    { SelectOfficer(Army);SelectTactic(static_cast<int>(Tactic));SelectAt(EnemyTile);Issue(ETacticalCommand::QueueTactic); };
    for (int Phase=1;Phase<=J.TurnBudget() && J.stage==JourneyStage::Battle;++Phase)
    {
        if (Phase==1) { SelectOfficer(0);CommandAt(Campaign::At(8,11)); }
        if (Phase==3) { SelectOfficer(0);CommandAt(Campaign::At(9,11)); }
        if (!passedForest && J.world.armies[2].tile==Campaign::At(8,12))
        { passedForest=true;escort.Queue(J,2,JourneyDirective::March,Campaign::At(12,11)); }
        auto* Seen=J.intelligence[0].Find(4);
        const bool Engaged=Seen && Seen->current && J.world.CanStrike(2,Seen->observed.tile);
        const auto Path=escort.Route(J,2);
        if (!reserveHandled && !Engaged && Path.size()>1) { SelectOfficer(2);CommandAt(Path[1]); }
        Seen=J.intelligence[0].Find(4);
        if (Seen && Seen->current && J.world.CanStrike(2,Seen->observed.tile))
        {
            const std::vector<ChainOrder> Push{{2,4,ChainTactic::Breakthrough,-1}};
            if (J.Preview(Push).completed==1)
            { ReserveTactic(2,ChainTactic::Breakthrough,Seen->observed.tile);Issue(ETacticalCommand::Execute);reserveHandled=true; }
            else if (!J.chain.signatureUsed[2])
            {
                const std::vector<ChainOrder> Stop{{0,4,ChainTactic::Feint,-1},{2,4,ChainTactic::SerpentRoar,-1}};
                if (J.Preview(Stop).completed==2)
                { const int Tile=Seen->observed.tile;ReserveTactic(0,ChainTactic::Feint,Tile);ReserveTactic(2,ChainTactic::SerpentRoar,Tile);Issue(ETacticalCommand::Execute); }
            }
            else { SelectOfficer(2);SelectAt(Seen->observed.tile);Issue(ETacticalCommand::Strike); }
        }
        SelectOfficer(0);if (CanIssue(ETacticalCommand::Hold)) Issue(ETacticalCommand::Hold);
        if (J.stage==JourneyStage::Battle) Issue(ETacticalCommand::EndTurn);
    }
    State->bNoPlayback=SavedPlayback;
}
bool ATacticalField::OpeningUpgradeSmokeTest()
{
    const bool SavedMode=bRealtimeCommand,SavedCouncil=bStartWithCouncil,SavedPlayback=State->bNoPlayback;
    const int SavedMission=FirstMission,SavedPolicy=EnemyPolicy;
    auto Check=[](bool Value,const TCHAR* Reason)
    { if (!Value) UE_LOG(LogTemp,Error,TEXT("GUNYU_OPENING_UPGRADE: FAIL: %s"),Reason);return Value; };
    State->bNoPlayback=true;PrepareOpeningPreview(false);
    auto& J=State->Journey;const auto Camp=J.world.cities[0];
    if (!Check(Presentation().Journey.bSouthernRoute && Presentation().Journey.bChooseRoute && J.PreparationCosts(0).gold==320,
        TEXT("council shows a real alternative and its guide cost"))) return false;
    Issue(ETacticalCommand::RouteRoad);Issue(ETacticalCommand::RouteSouthernWood);
    if (!Check(J.world.cities[0].gold==Camp.gold && J.world.cities[0].food==Camp.food,TEXT("route selection cannot charge or duplicate resources"))) return false;
    Issue(ETacticalCommand::Deploy);
    if (!Check(J.world.cities[0].gold==Camp.gold-320 && Presentation().ConvoyRoute.Num()==8 && Presentation().Objective.Contains(TEXT("7区間")),
        TEXT("deployment charges once and presents the actual seven-leg route"))) return false;
    Issue(ETacticalCommand::Withdraw);Issue(ETacticalCommand::Withdraw);Issue(ETacticalCommand::Retry);
    if (!Check(J.stage==JourneyStage::Preparation && J.convoyRoute==JourneyConvoyRoute::SouthernWood && J.world.cities[0].gold==Camp.gold,
        TEXT("retry restores the council and its selected route without settlement exploits"))) return false;
    Issue(ETacticalCommand::RouteRoad);Issue(ETacticalCommand::Deploy);
    if (!Check(J.world.cities[0].gold==Camp.gold-200 && J.battleReview.tacticSteps==0,TEXT("replanning starts with clean battle facts and the new bill"))) return false;
    PrepareOpeningPreview(true);
    if (!Check(J.won && J.history[0].rescued && J.world.armies[3].troops>450 && J.battleReview.longestChain==2,
        TEXT("public UI commands deliver the southern convoy without breaking the front or a three-officer chain"))) return false;
    if (!Check(Presentation().Journey.ReviewFacts.Contains(TEXT("最大2")) && !Presentation().Journey.ReviewLesson.IsEmpty() && Presentation().Journey.Outcome.Contains(TEXT("護送")),
        TEXT("debrief reports this battle's objective, facts and a next decision"))) return false;
    const int Bond=J.world.Affinity(0,1);Issue(ETacticalCommand::TalkLiu);Issue(ETacticalCommand::TalkGuan);Issue(ETacticalCommand::TalkZhang);
    if (!Check(J.world.Affinity(0,1)==Bond && Presentation().Journey.Dialogue.Contains(TEXT("林")),TEXT("officers remember the chosen route without farming relationships"))) return false;
    Issue(ETacticalCommand::Continue);
    if (!Check(J.history[0].route==JourneyConvoyRoute::SouthernWood && J.history[0].review.longestChain==2,TEXT("the result and method persist into the next council"))) return false;
    bRealtimeCommand=SavedMode;bStartWithCouncil=SavedCouncil;FirstMission=SavedMission;EnemyPolicy=SavedPolicy;
    State->bNoPlayback=SavedPlayback;Issue(ETacticalCommand::Restart);
    UE_LOG(LogTemp,Display,TEXT("GUNYU_OPENING_UPGRADE: PASS (real route choice, guide bill, public alternative victory, factual review, retry and carryover)"));
    return true;
}
