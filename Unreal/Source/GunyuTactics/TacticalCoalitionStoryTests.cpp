#include "TacticalTrial.h"
#include "TacticalState.h"
#include "JourneyAutoBattle.hpp"
#include "JourneyBattlefield.hpp"
#include "JourneySave.hpp"
#include <memory>

bool ATacticalController::PrepareBattlefieldLayoutCapture(int Variant)
{
    using namespace frontline;
    if(Variant<0 || Variant>int(JourneyMap::CoalitionRoad)) return false;
    const auto Map=static_cast<JourneyMap>(Variant);
    CloseEpisode();
    Field->Issue(ETacticalCommand::Skip);
    auto Save=std::make_unique<JourneySaveData>();
    auto& J=Save->journey;
    J.ResetCampaign();J.SelectDifficulty(JourneyDifficulty::Story);J.Meet();
    switch(Map)
    {
    case JourneyMap::RefugeeRoad:
    case JourneyMap::RefugeeWood:
        if(!J.SelectPromise(JourneyPromise::EscortRefugees) ||
            (Map==JourneyMap::RefugeeWood && !J.SelectConvoyRoute(JourneyConvoyRoute::SouthernWood))) return false;
        break;
    case JourneyMap::Port:
    case JourneyMap::PortNorth:
        J.mission=1;
        if(Map==JourneyMap::PortNorth)
        {
            J.history[0].completed=J.history[0].won=J.history[0].story.recorded=true;
            J.history[0].story.reply=JourneyPledge::Preserve;
        }
        break;
    case JourneyMap::Valley:
        J.mission=2;
        break;
    case JourneyMap::Guangzong:
        J.mission=2;J.stage=JourneyStage::Complete;
        J.history[2].completed=J.history[2].won=true;
        if(!J.Continue()) return false;
        break;
    case JourneyMap::CoalitionFord:
    case JourneyMap::CoalitionRoad:
        J.mission=YellowTurbanMission;J.stage=JourneyStage::Complete;J.won=true;
        J.history[YellowTurbanMission].completed=J.history[YellowTurbanMission].won=true;
        if(!J.ChooseCoalitionPlan(Map==JourneyMap::CoalitionRoad ? CoalitionPlan::Protect:CoalitionPlan::Advance)) return false;
        break;
    case JourneyMap::Retreat:
        break;
    }
    if(J.Battlefield()!=Map || !J.ApplyPlanExample()) return false;
    Save->beforeBattle=J;
    if(!J.Prepare(0,1)) return false;
    std::vector<std::uint8_t> Bytes;
    if(!EncodeJourneySave(*Save,Bytes) || !Field->ImportSession(Bytes)) return false;
    CameraYaw=90;CameraElevation=69.f;CameraPreset=2;
    Focus=FVector(1760,1400,0);Distance=ViewDistance=4500.f;
    LabelLayoutFrame=MAX_uint64;
    return Field->Presentation().bBattle && Field->Presentation().Title.Contains(JourneyText(BattlefieldDefinition(Map).name));
}

bool ATacticalController::PrepareCoalitionStoryPreview(int Page)
{
    using namespace frontline;
    CloseEpisode();
    Field->Issue(ETacticalCommand::Skip);
    auto Save=std::make_unique<JourneySaveData>();
    auto& Journey=Save->journey;
    Journey.ResetCampaign();
    Journey.SelectDifficulty(JourneyDifficulty::Story);
    Journey.Meet();
    for(int Mission=0;Mission<=YellowTurbanMission;++Mission)
    {
        if(Mission==YellowTurbanMission)
        {
            if(Page==-3) break;
            if(!Journey.CanBeginYellowTurban() || !Journey.Continue()) return false;
            if(Page==-2) break;
            Save->beforeBattle=Journey;
        }
        if(Journey.mission!=Mission || !Journey.Prepare(1,Journey.enemyPolicy)) return false;
        if(Mission==YellowTurbanMission && Page==-4) break;
        for(int Cycle=0;Cycle<18 && Journey.stage==JourneyStage::Battle;++Cycle)
        {
            auto Next=PlanJourneyAutoBattle(Journey);
            if(!Next.tactics.empty()) Journey.Execute(Next.tactics);
            if(Journey.stage==JourneyStage::Battle && !Next.movement.Resolve(Journey)) return false;
        }
        if(!Journey.won) return false;
        if(Journey.NeedsStoryReply() && !Journey.ChooseStoryReply(JourneyPledge::Coordinate)) return false;
        if(!Journey.Continue()) return false;
    }
    const bool CoalitionBattle=Page<=-6;
    const bool Protect=Page==-7 || Page==-9;
    if(CoalitionBattle)
    {
        if(!Journey.ChooseCoalitionPlan(Protect ? CoalitionPlan::Protect:CoalitionPlan::Advance)) return false;
        Save->beforeBattle=Journey;
        if(!Journey.Prepare(1,Journey.enemyPolicy)) return false;
        if(Page<=-8)
        {
            for(int Cycle=0;Cycle<18 && Journey.stage==JourneyStage::Battle;++Cycle)
            {
                auto Next=PlanJourneyAutoBattle(Journey);
                if(!Next.tactics.empty()) Journey.Execute(Next.tactics);
                if(Journey.stage==JourneyStage::Battle && !Next.movement.Resolve(Journey)) return false;
            }
            if(!Journey.won || !Journey.Continue()) return false;
        }
    }
    std::vector<std::uint8_t> Bytes;
    if(!EncodeJourneySave(*Save,Bytes) || !Field->ImportSession(Bytes)) return false;
    const auto& P=Field->Presentation().Journey;
    if(CoalitionBattle)
    {
        if(Page>=-7) return Field->Presentation().bBattle &&
            Field->Presentation().GoalLabel==(Protect ? TEXT("北の烽火"):TEXT("董卓軍の前哨"));
        OpenEpisode();
        if(!bEpisodeOpen || EpisodeSnapshot.Lines.Num()!=3) return false;
        StepEpisode(1);
        NovelElapsed=1000;NovelStageTime=NovelLineTime=1;
        return CurrentEpisodeLine().Text.Contains(Protect ? TEXT("張邈"):TEXT("曹操"));
    }
    if(Page==-3) return P.bComplete && P.CoalitionEpisode.Lines.IsEmpty() && P.PrimaryLabel.Contains(TEXT("黄巾の乱"));
    if(Page==-4) return Field->Presentation().bBattle && Field->Presentation().GoalLabel==TEXT("黄巾の前陣");
    if(Page==-2)
    {
        if(!P.bPreparation || !P.CoalitionEpisode.Lines.IsEmpty()) return false;
        OpenJourneyEntry();
        CouncilElapsed=1000;CouncilStageTime=CouncilLineTime=1;
        return bNovelPlan && CouncilExchange.Subject.Contains(TEXT("広宗"));
    }
    if(!P.bComplete || P.CoalitionEpisode.Lines.Num()!=12 || Field->Presentation().Title!=TEXT("黄巾の乱の記録")) return false;
    if(Page==-1) return true;
    OpenEpisode(Page>=0);
    const int DisplayPage=FMath::Max(0,Page);
    if(!bEpisodeOpen || !EpisodeSnapshot.Lines.IsValidIndex(DisplayPage)) return false;
    while(EpisodePage<DisplayPage) StepEpisode(1);
    NovelElapsed=1000;NovelStageTime=NovelLineTime=1;
    return Page!=11 || CanChooseCoalitionFromNovel();
}

bool ATacticalController::CoalitionStorySmokeTest()
{
    auto Check=[](bool Valid,const TCHAR* Why)
    { if(!Valid) UE_LOG(LogTemp,Error,TEXT("GUNYU_COALITION_UI: FAIL %s"),Why);return Valid; };
    CloseEpisode();
    OpenEpisode(true);
    if(!Check(!bEpisodeOpen,TEXT("the opening council cannot open the next chapter"))) return false;
    if(!Check(PrepareCoalitionStoryPreview(-3),TEXT("three real battles unlock Yellow Turban but not the coalition"))) return false;
    OpenEpisode(true);
    if(!Check(!bEpisodeOpen,TEXT("the prologue cannot open the coalition directly"))) return false;
    OpenJourneyEntry();
    while(EpisodePage+1<EpisodeSnapshot.Lines.Num()) StepEpisode(1);
    NovelElapsed=1000;
    HandleEpisodeKey(EKeys::Enter);
    if(!Check(bEpisodeOpen && bNovelPlan && Field->Presentation().Mission==frontline::YellowTurbanMission &&
        CouncilExchange.Subject.Contains(TEXT("広宗")),TEXT("the last prologue line leads to the Yellow Turban council"))) return false;
    DeployNovelPlan();
    if(!Check(Field->Presentation().bBattle && Field->Presentation().GoalLabel==TEXT("黄巾の前陣"),TEXT("the normal council deploys the new capture battle"))) return false;
    Field->Issue(ETacticalCommand::Withdraw);Field->Issue(ETacticalCommand::Withdraw);Field->Issue(ETacticalCommand::Skip);
    if(!Check(Field->CanIssue(ETacticalCommand::Retry) && !Field->CanIssue(ETacticalCommand::Continue) &&
        Field->Presentation().Journey.CoalitionEpisode.Lines.IsEmpty(),TEXT("withdrawal still requires a retry before the coalition"))) return false;
    Field->Issue(ETacticalCommand::Retry);
    if(!Check(Field->Presentation().Journey.bPreparation && Field->Presentation().Mission==frontline::YellowTurbanMission,
        TEXT("retry restores the Yellow Turban council"))) return false;
    if(!Check(PrepareCoalitionStoryPreview(-1),TEXT("the actual capture victory unlocks chapter two"))) return false;
    std::vector<std::uint8_t> Before,After;
    if(!Check(Field->ExportSession(Before),TEXT("the ending exports"))) return false;
    OpenJourneyEntry();
    if(!Check(bEpisodeOpen && !bCoalitionEpisode && CurrentEpisodeLine().Text.Contains(TEXT("皇甫嵩")),TEXT("the Yellow Turban ending opens first"))) return false;
    while(EpisodePage+1<EpisodeSnapshot.Lines.Num()) StepEpisode(1);
    NovelElapsed=1000;
    HandleEpisodeKey(EKeys::Enter);
    if(!Check(bEpisodeOpen && bCoalitionEpisode && EpisodePage==0 && EpisodeSnapshot.Title.Contains(TEXT("第二章　反董卓連合")),
        TEXT("the final epilogue button opens the next chapter from page one"))) return false;
    UpdateEpisodeInput();
    if(!Check(bEpisodeOpen && CurrentEpisodeLine().Text.Contains(TEXT("何進")),TEXT("the chapter stays open across normal input updates"))) return false;
    for(int Page=0;Page<EpisodeSnapshot.Lines.Num()-1;++Page)
    {
        if(CurrentEpisodeLine().Speaker==3 && !Check(!IsEnsembleEpisode(),TEXT("the strategist appears when speaking"))) return false;
        NovelElapsed=1000;
        HandleEpisodeKey(EKeys::Enter);
    }
    NovelElapsed=1000;
    HandleEpisodeKey(EKeys::Enter);
    if(!Check(CanChooseCoalitionFromNovel() && Field->ExportSession(After) && Before==After,TEXT("reading and Enter cannot choose a route"))) return false;
    for(int Route=0;Route<2;++Route)
    {
        const bool Protect=Route==1;
        const TCHAR* Partner=Protect ? TEXT("張邈"):TEXT("曹操");
        CloseEpisode();
        if(!Check(Field->ImportSession(Before),TEXT("Yellow Turban completion reloads at the choice boundary"))) return false;
        OpenEpisode(true);
        if(!Check(bEpisodeOpen && bCoalitionEpisode && EpisodePage==0,TEXT("the coalition scene can be reopened"))) return false;
        while(EpisodePage+1<EpisodeSnapshot.Lines.Num()) StepEpisode(1);
        NovelElapsed=1000;
        HandleEpisodeKey(Protect ? EKeys::Two:EKeys::One);
        if(!Check(bEpisodeOpen && bNovelPlan && CouncilExchange.Subject.Contains(Partner) && Field->Presentation().Mission==frontline::CoalitionMission,
            TEXT("either reply opens the matching preparation"))) return false;
        DeployNovelPlan();
        const auto& H=Field->Presentation();
        if(!Check(H.bBattle && H.GoalLabel==(Protect ? TEXT("北の烽火"):TEXT("董卓軍の前哨")),
            TEXT("deployment applies the selected objective"))) return false;
        if(!Check(Field->ExportSession(After) && Field->ImportSession(After),TEXT("a deployed route saves and reloads"))) return false;
        Field->Issue(ETacticalCommand::Withdraw);Field->Issue(ETacticalCommand::Withdraw);Field->Issue(ETacticalCommand::Skip);
        if(!Check(!Field->CanIssue(ETacticalCommand::Continue) && Field->CanIssue(ETacticalCommand::Retry),TEXT("withdrawal cannot finish the route"))) return false;
        Field->Issue(ETacticalCommand::Retry);
        OpenJourneyEntry();
        if(!Check(bNovelPlan && CouncilExchange.Subject.Contains(Partner),TEXT("retry retains the selected partner"))) return false;
        if(!Check(PrepareCoalitionStoryPreview(Protect ? -9:-8),TEXT("each real battle win has the matching partner's aftermath"))) return false;
    }
    CloseEpisode();
    UE_LOG(LogTemp,Display,TEXT("GUNYU_COALITION_UI: PASS (Yellow Turban gate, explicit route choice, both deployments, retry/reload and both actual victories)"));
    return true;
}
