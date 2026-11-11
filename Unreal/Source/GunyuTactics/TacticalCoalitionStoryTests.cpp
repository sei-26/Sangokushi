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
    for(int Mission=0;Mission<3;++Mission)
    {
        if(Journey.mission!=Mission || !Journey.Prepare(1,Journey.enemyPolicy)) return false;
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
    if(!P.bComplete || P.CoalitionEpisode.Lines.Num()!=9) return false;
    if(Page<0) return true;
    OpenEpisode(true);
    if(!bEpisodeOpen || !EpisodeSnapshot.Lines.IsValidIndex(Page)) return false;
    while(EpisodePage<Page) StepEpisode(1);
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
    if(!Check(PrepareCoalitionStoryPreview(-1),TEXT("three real battles unlock the next chapter"))) return false;
    std::vector<std::uint8_t> Before,After;
    if(!Check(Field->ExportSession(Before),TEXT("the ending exports"))) return false;
    OpenJourneyEntry();
    if(!Check(bEpisodeOpen && !bCoalitionEpisode,TEXT("the ordinary epilogue still opens first"))) return false;
    while(EpisodePage+1<EpisodeSnapshot.Lines.Num()) StepEpisode(1);
    NovelElapsed=1000;
    HandleEpisodeKey(EKeys::Enter);
    if(!Check(bEpisodeOpen && bCoalitionEpisode && EpisodePage==0 && EpisodeSnapshot.Title.Contains(TEXT("反董卓連合")),
        TEXT("the final epilogue button opens the next chapter from page one"))) return false;
    UpdateEpisodeInput();
    if(!Check(bEpisodeOpen && CurrentEpisodeLine().Text.Contains(TEXT("何進")),TEXT("the chapter stays open across normal input updates"))) return false;
    int Pages=0;
    while(bEpisodeOpen)
    {
        NovelElapsed=1000;
        HandleEpisodeKey(EKeys::Enter);
        ++Pages;
    }
    if(!Check(Pages==9 && Field->ExportSession(After) && Before==After,TEXT("reading returns to the ending without changing its save"))) return false;
    if(!Check(Field->ImportSession(Before),TEXT("completed campaign reloads"))) return false;
    OpenEpisode(true);
    if(!Check(bEpisodeOpen && bCoalitionEpisode && EpisodePage==0,TEXT("the next chapter remains available after reload"))) return false;
    CloseEpisode();
    UE_LOG(LogTemp,Display,TEXT("GUNYU_COALITION_UI: PASS (actual three-battle campaign, epilogue entry, nine pages, return and reload)"));
    return true;
}
