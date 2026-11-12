#include "TacticalTrial.h"
#include "JourneyAutoBattle.hpp"
#include "JourneySave.hpp"
#include <memory>

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
    std::vector<std::uint8_t> Bytes;
    if(!EncodeJourneySave(*Save,Bytes) || !Field->ImportSession(Bytes)) return false;
    const auto& P=Field->Presentation().Journey;
    if(Page==-3) return P.bComplete && P.CoalitionEpisode.Lines.IsEmpty() && P.PrimaryLabel.Contains(TEXT("黄巾の乱"));
    if(Page==-4) return Field->Presentation().bBattle && Field->Presentation().GoalLabel==TEXT("黄巾の前陣");
    if(Page==-2)
    {
        if(!P.bPreparation || !P.CoalitionEpisode.Lines.IsEmpty()) return false;
        OpenJourneyEntry();
        CouncilElapsed=1000;CouncilStageTime=CouncilLineTime=1;
        return bNovelPlan && CouncilExchange.Subject.Contains(TEXT("広宗"));
    }
    if(!P.bComplete || P.CoalitionEpisode.Lines.Num()!=9 || Field->Presentation().Title!=TEXT("黄巾の乱の記録")) return false;
    if(Page==-1) return true;
    OpenEpisode(Page>=0);
    const int DisplayPage=FMath::Max(0,Page);
    if(!bEpisodeOpen || !EpisodeSnapshot.Lines.IsValidIndex(DisplayPage)) return false;
    while(EpisodePage<DisplayPage) StepEpisode(1);
    NovelElapsed=1000;NovelStageTime=NovelLineTime=1;
    return true;
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
    int Pages=0;
    while(bEpisodeOpen)
    {
        if(CurrentEpisodeLine().Speaker==3 && !Check(!IsEnsembleEpisode(),TEXT("the strategist appears when speaking"))) return false;
        NovelElapsed=1000;
        HandleEpisodeKey(EKeys::Enter);
        ++Pages;
    }
    if(!Check(Pages==9 && Field->ExportSession(After) && Before==After,TEXT("reading returns to the ending without changing its save"))) return false;
    if(!Check(Field->ImportSession(Before),TEXT("completed campaign reloads"))) return false;
    OpenEpisode(true);
    if(!Check(bEpisodeOpen && bCoalitionEpisode && EpisodePage==0,TEXT("the next chapter remains available after reload"))) return false;
    CloseEpisode();
    UE_LOG(LogTemp,Display,TEXT("GUNYU_COALITION_UI: PASS (prologue, Yellow Turban council/deploy/retry, fourth battle, ending, coalition and reload)"));
    return true;
}
