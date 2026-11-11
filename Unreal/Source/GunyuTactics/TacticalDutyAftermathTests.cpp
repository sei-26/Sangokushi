#include "TacticalTrial.h"
#include "TacticalState.h"
#include "JourneyAftermathRole.hpp"
#include "../../../tests/JourneyDutyAftermathPlayer.hpp"

using namespace frontline;

bool ATacticalController::PrepareDutyAftermathPreview(int Variant,bool bSummary)
{
    if(Variant<0 || Variant>3) return false;
    CloseEpisode();
    Field->Issue(ETacticalCommand::Skip);
    const auto Save=dutyaftermathqa::Play(Variant);
    if(!Save) return false;
    const auto Duty=ReadDutyAftermath(Save->journey);
    if(Duty.lines.size()!=1) return false;
    std::vector<std::uint8_t> Bytes;
    Field->bLegacyJourney=false;
    if(!EncodeJourneySave(*Save,Bytes) || !Field->ImportSession(Bytes)) return false;
    OpenJourneyEntry();
    const auto& P=Field->Presentation().Journey;
    if(!P.bDebrief || bEpisodeOpen || !P.CampaignChange.Contains(JourneyText(Duty.summary))) return false;
    if(bSummary) return true;

    OpenEpisode();
    const auto& Line=Duty.lines.front();
    const FString Text=JourneyText(Line.text);
    const int Page=EpisodeSnapshot.Lines.IndexOfByPredicate([&](const auto& PageLine)
    { return PageLine.Speaker==Line.speaker && PageLine.Text==Text; });
    if(!bEpisodeOpen || Page<0) return false;
    while(EpisodePage<Page) StepEpisode(1);
    NovelElapsed=1000;NovelStageTime=NovelLineTime=1;
    return true;
}

bool ATacticalController::DutyAftermathSmokeTest()
{
    auto Check=[](bool Valid,const TCHAR* Why)
    { if(!Valid) UE_LOG(LogTemp,Error,TEXT("GUNYU_DUTY_AFTERMATH_UI: FAIL %s"),Why);return Valid; };
    std::vector<std::uint8_t> Original,Before,After;
    CloseEpisode();
    Field->Issue(ETacticalCommand::Skip);
    const bool Legacy=Field->bLegacyJourney;
    if(!Check(Field->ExportSession(Original),TEXT("original session exports"))) return false;
    for(int Variant=0;Variant<4;++Variant)
    {
        if(!Check(PrepareDutyAftermathPreview(Variant,true) && Field->ExportSession(Before),TEXT("real debrief shows the optional duty summary"))) return false;
        const auto Summary=Field->Presentation().Journey.CampaignChange;
        auto Save=std::make_unique<JourneySaveData>();
        if(!Check(DecodeJourneySave(Before,*Save),TEXT("settled session decodes"))) return false;
        const auto Duty=ReadDutyAftermath(Save->journey);
        const auto& Line=Duty.lines.front();
        const FString Words=JourneyText(Line.text);
        OpenEpisode();
        if(!Check(bEpisodeOpen,TEXT("the player can open the main scene"))) return false;
        int Found=0;
        while(bEpisodeOpen)
        {
            if(CurrentEpisodeLine().Speaker==Line.speaker && CurrentEpisodeLine().Text==Words) ++Found;
            StepEpisode(1);
        }
        BuildInterface();
        if(!Check(Found==1 && Field->ExportSession(After) && Before==After,TEXT("one voiced page and reading changes no save data"))) return false;
        if(!Check(Field->ImportSession(Before) && Field->Presentation().Journey.CampaignChange==Summary,TEXT("the same summary survives reload"))) return false;
        OpenEpisode();
        if(!Check(bEpisodeOpen && EpisodeSnapshot.Lines.ContainsByPredicate([&](const auto& Page) { return Page.Speaker==Line.speaker && Page.Text==Words; }),TEXT("the same speaker survives reload"))) return false;
        CloseEpisode();
        if(Variant<2)
        {
            if(!Check(Field->CanIssue(ETacticalCommand::Continue),TEXT("a win remains ready to continue"))) return false;
            Field->Issue(ETacticalCommand::Continue);
            if(!Check(Field->Presentation().Journey.bPreparation,TEXT("the next council opens normally"))) return false;
        }
        else
        {
            if(!Check(!Field->CanIssue(ETacticalCommand::Continue) && Field->CanIssue(ETacticalCommand::Retry),TEXT("a withdrawal remains a retry"))) return false;
            Field->Issue(ETacticalCommand::Retry);
            if(!Check(Field->Presentation().Journey.bPreparation && Field->ExportSession(After) && DecodeJourneySave(After,*Save) &&
                ReadDutyAftermath(Save->journey).summary.empty() && !Save->journey.assignments[Line.speaker].completed,TEXT("retry restores the unpaid plan without past execution credit"))) return false;
        }
    }
    Field->bLegacyJourney=Legacy;
    if(!Check(Field->ImportSession(Original),TEXT("original session restores"))) return false;
    UE_LOG(LogTemp,Display,TEXT("GUNYU_DUTY_AFTERMATH_UI: PASS (port, delivered convoy, two retreats, optional page, save/load and retry)"));
    return true;
}
