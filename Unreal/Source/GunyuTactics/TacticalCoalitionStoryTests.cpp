#include "TacticalTrial.h"
#include "TacticalState.h"
#include "JourneyAutoBattle.hpp"
#include "JourneyBattlefield.hpp"
#include "JourneyEpisode.hpp"
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
    const bool YellowPortrait=Page>=-12 && Page<=-10;
    const bool YellowField=Page==-13;
    for(int Mission=0;Mission<=YellowTurbanMission;++Mission)
    {
        if(Mission==YellowTurbanMission)
        {
            if(Page==-3) break;
            if(!Journey.CanBeginYellowTurban() || !Journey.Continue()) return false;
            if(Page==-2 || YellowPortrait) break;
            Save->beforeBattle=Journey;
        }
        if(Journey.mission!=Mission || !Journey.Prepare(1,Journey.enemyPolicy)) return false;
        if(Mission==YellowTurbanMission && (Page==-4 || YellowField))
        {
            if(YellowField)
            {
                Journey.world.armies[0].tile=Campaign::At(11,9);
                Journey.world.armies[3].morale=75;
                Journey.RefreshIntelligence();
                if(!Journey.EndTurn() || Journey.world.tacticEvents.empty()) return false;
            }
            break;
        }
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
    const bool CoalitionBattle=Page>=-9 && Page<=-6;
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
        if(!bEpisodeOpen) return false;
        const int Speaker=Protect ? ZhangMiao:CaoCao;
        while(CurrentEpisodeLine().Speaker!=Speaker && EpisodePage+1<EpisodeSnapshot.Lines.Num()) StepEpisode(1);
        NovelElapsed=1000;NovelStageTime=NovelLineTime=1;
        return CurrentEpisodeLine().Speaker==Speaker && HistoricalPortraitBrush(Speaker);
    }
    if(Page==-3) return P.bComplete && P.CoalitionEpisode.Lines.IsEmpty() && P.PrimaryLabel.Contains(TEXT("黄巾の乱"));
    if(Page==-4) return Field->Presentation().bBattle && Field->Presentation().GoalLabel==TEXT("黄巾の前陣");
    if(YellowField)
    {
        Field->SelectAt(Journey.world.armies[3].tile);
        bEnemyDetailsOpen=true;
        CameraYaw=90;CameraElevation=54;
        Focus=ATacticalField::TileLocation(Campaign::At(11,8));Distance=ViewDistance=2700;
        return Field->VisualUnits()[3].HistoricalSpeaker==ZhangLiang &&
            Field->Presentation().EnemyCounter.Contains(TEXT("鼓舞"));
    }
    if(YellowPortrait)
    {
        OpenEpisode();
        for(int I=0;I<Page+13;++I) StepEpisode(1);
        NovelElapsed=1000;NovelStageTime=NovelLineTime=1;
        return bEpisodeOpen && CurrentEpisodeLine().Speaker>=ZhangJiao && CurrentEpisodeLine().Speaker<=HuangfuSong;
    }
    if(Page==-2)
    {
        if(!P.bPreparation || !P.CoalitionEpisode.Lines.IsEmpty()) return false;
        OpenEpisode();OpenNovelPlan();
        CouncilElapsed=1000;CouncilStageTime=CouncilLineTime=1;
        return bNovelPlan && CouncilExchange.Subject.Contains(TEXT("広宗"));
    }
    if(!P.bComplete || P.CoalitionEpisode.Lines.Num()!=14 || Field->Presentation().Title!=TEXT("黄巾の乱の記録")) return false;
    if(Page==-1) return true;
    OpenEpisode(Page>=0);
    const int DisplayPage=FMath::Max(0,Page);
    if(!bEpisodeOpen || !EpisodeSnapshot.Lines.IsValidIndex(DisplayPage)) return false;
    while(EpisodePage<DisplayPage) StepEpisode(1);
    NovelElapsed=1000;NovelStageTime=NovelLineTime=1;
    return EpisodePage+1!=EpisodeSnapshot.Lines.Num() || CanChooseCoalitionFromNovel();
}

bool ATacticalController::CoalitionStorySmokeTest()
{
    using namespace frontline;
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
    if(!Check(bEpisodeOpen && !bNovelPlan && Field->Presentation().Mission==YellowTurbanMission,TEXT("the historical opening plays before the new council"))) return false;
    int SeenYellowTurban=0;
    for(int Page=0;Page<EpisodeSnapshot.Lines.Num();++Page)
    {
        const int Speaker=CurrentEpisodeLine().Speaker;
        if(Speaker>=ZhangJiao && Speaker<=HuangfuSong && HistoricalPortraitBrush(Speaker)) SeenYellowTurban|=1<<(Speaker-ZhangJiao);
        NovelElapsed=1000;
        HandleEpisodeKey(EKeys::Enter);
    }
    if(!Check(SeenYellowTurban==7,TEXT("Zhang Jiao, Zhang Liang and Huangfu Song each have their own speaking portrait"))) return false;
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
    int SeenCoalition=0;
    for(int Page=0;Page<EpisodeSnapshot.Lines.Num()-1;++Page)
    {
        if(CurrentEpisodeLine().Speaker==3 && !Check(!IsEnsembleEpisode(),TEXT("the strategist appears when speaking"))) return false;
        const int Speaker=CurrentEpisodeLine().Speaker;
        if(Speaker>=CaoCao && Speaker<=LuBu && HistoricalPortraitBrush(Speaker) && !IsEnsembleEpisode()) SeenCoalition|=1<<(Speaker-CaoCao);
        NovelElapsed=1000;
        HandleEpisodeKey(EKeys::Enter);
    }
    NovelElapsed=1000;
    HandleEpisodeKey(EKeys::Enter);
    if(!Check(SeenCoalition==63 && CanChooseCoalitionFromNovel() && Field->ExportSession(After) && Before==After,
        TEXT("six coalition-era portraits play before the explicit route choice"))) return false;
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

bool ATacticalController::PrepareYangrenPreview(int View)
{
    using namespace frontline;
    if(View<0 || View>4 || !PrepareCoalitionStoryPreview(View==1 ? -9:-8)) return false;
    CloseEpisode();
    if(!Field->CanIssue(ETacticalCommand::Continue)) return false;
    Field->Issue(ETacticalCommand::Continue);
    OpenJourneyEntry();
    if(!bEpisodeOpen || bNovelPlan || Field->Presentation().Mission!=YangrenMission) return false;
    if(View<2)
    {
        StepEpisode(1);
        NovelElapsed=1000;NovelStageTime=NovelLineTime=1;
        return CurrentEpisodeLine().Speaker==(View==0 ? CaoCao:ZhangMiao);
    }
    while(EpisodePage+1<EpisodeSnapshot.Lines.Num()) StepEpisode(1);
    NovelElapsed=1000;
    HandleEpisodeKey(EKeys::Enter);
    if(!bNovelPlan) return false;
    if(View==2) { TogglePlanningMap();return bNovelMap; }
    DeployNovelPlan();
    if(!Field->Presentation().bBattle) return false;
    const auto& U=Field->VisualUnits();
    if(!U.IsValidIndex(5) || U[5].HistoricalSpeaker!=SunJian || U[5].bTransport) return false;
    std::vector<std::uint8_t> Bytes;
    auto Save=std::make_unique<JourneySaveData>();
    if(!Field->ExportSession(Bytes) || !DecodeJourneySave(Bytes,*Save)) return false;
    auto& J=Save->journey;
    if(View==3)
    {
        // The starting forest can conceal the enemy. March the lead company
        // along the open road until the normal observation reveals the cavalry.
        JourneyRealtime Orders;
        if(!Orders.Queue(J,0,JourneyDirective::March,Campaign::At(10,8))) return false;
        for(int Cycle=0;Cycle<3;++Cycle)
        {
            const auto* Contact=J.intelligence[0].Find(3);
            if(Contact && Contact->current) break;
            if(!Orders.Resolve(J)) return false;
        }
        if(!EncodeJourneySave(*Save,Bytes) || !Field->ImportSession(Bytes)) return false;
        CameraYaw=90;CameraElevation=61;CameraPreset=2;
        Focus=FVector(2010,1410,0);Distance=ViewDistance=3150;
        const auto& Cavalry=Field->VisualUnits()[3];
        if(Cavalry.HistoricalSpeaker==LuBu && Cavalry.bCavalry) return true;
        UE_LOG(LogTemp,Error,TEXT("GUNYU_YANGREN: FAIL cavalry not visible after marching (turn=%d)"),J.turn);
        return false;
    }
    for(int Cycle=0;Cycle<18 && J.stage==JourneyStage::Battle;++Cycle)
    {
        auto Next=PlanJourneyAutoBattle(J);
        if(!Next.tactics.empty()) J.Execute(Next.tactics);
        if(J.stage==JourneyStage::Battle && !Next.movement.Resolve(J)) return false;
    }
    if(!J.won || !J.SunJianAtRally() || !J.Continue() || !EncodeJourneySave(*Save,Bytes) || !Field->ImportSession(Bytes)) return false;
    OpenEpisode();
    NovelElapsed=1000;NovelStageTime=NovelLineTime=1;
    return bEpisodeOpen && CurrentEpisodeLine().Speaker==SunJian;
}

bool ATacticalController::YangrenSmokeTest()
{
    auto Check=[](bool Valid,const TCHAR* Reason)
    { if(!Valid) UE_LOG(LogTemp,Error,TEXT("GUNYU_YANGREN: FAIL %s"),Reason);return Valid; };
    if(!Check(PrepareYangrenPreview(0) && PrepareYangrenPreview(1),TEXT("both Suanzao choices reach their own Yangren dialogue"))) return false;
    if(!Check(PrepareYangrenPreview(2) && Field->Presentation().Journey.Map.Name==TEXT("陽人の山麓"),
        TEXT("normal episode controls open the dedicated planning map"))) return false;
    if(!Check(PrepareYangrenPreview(3) && Field->Presentation().SecondGoalLabel==TEXT("孫堅と合流"),
        TEXT("actual deployment presents Sun Jian as a combat ally and marks the rally site"))) return false;
    std::vector<std::uint8_t> Bytes;
    if(!Check(Field->ExportSession(Bytes) && Field->ImportSession(Bytes) && Field->VisualUnits()[5].HistoricalSpeaker==frontline::SunJian,
        TEXT("reloading preserves the allied unit and both objectives"))) return false;
    Field->Issue(ETacticalCommand::Withdraw);Field->Issue(ETacticalCommand::Withdraw);Field->Issue(ETacticalCommand::Skip);
    Field->Issue(ETacticalCommand::Retry);
    if(!Check(Field->Presentation().Mission==frontline::YangrenMission && Field->Presentation().Journey.bPreparation,
        TEXT("withdrawal restarts the same council"))) return false;
    if(!Check(PrepareYangrenPreview(4) && Field->Presentation().bComplete,TEXT("actual victory opens the Sun Jian epilogue"))) return false;
    CloseEpisode();
    UE_LOG(LogTemp,Display,TEXT("GUNYU_YANGREN: PASS (two councils, real deployment, reload, withdrawal retry, allied victory and ending)"));
    return true;
}
