#include "TacticalTrial.h"
#include "TacticalState.h"
#include "TacticalContributionQA.h"
#include "TacticalSupportQA.h"
#include "JourneySave.hpp"
#include "JourneyAftermathRole.hpp"
#include <memory>
bool ATacticalController::DefenseAftermathSmokeTest()
{
    using namespace frontline;
    auto Check=[](bool V,const TCHAR* Why) { if(!V) UE_LOG(LogTemp,Error,TEXT("GUNYU_DEFENSE_AFTERMATH_UI: FAIL %s"),Why);return V; };
    CloseEpisode();Field->Issue(ETacticalCommand::Skip);std::vector<std::uint8_t> Original,Before,After;
    const bool Legacy=Field->bLegacyJourney;if(!Check(Field->ExportSession(Original),TEXT("original session exports"))) return false;
    for(int Variant:{0,3,2,1})
    {
        CloseEpisode();if(!Check(PrepareCooperationCouncilPreview(Field,0),TEXT("actual prior victory opens the normal council"))) return false;
        Field->Issue(ETacticalCommand::PlanExample);Field->Issue(ETacticalCommand::PlanAmbush);Field->Issue(ETacticalCommand::Deploy);
        if(!Check(PrepareDefenseAftermathPreview(Field,Variant) && Field->ExportSession(Before),TEXT("authored contact fixture settles only through real tactics guard cycles or withdrawal"))) return false;
        auto Save=std::make_unique<JourneySaveData>();if(!Check(DecodeJourneySave(Before,*Save),TEXT("settled record decodes"))) return false;
        const auto& J=Save->journey;const auto& R=J.history[J.mission].review;const auto Scene=ReadRoleAftermath(J);
        const int Guard=Variant==3 ? 2:1;
        if(!Check(R.confirmedDefenders!=0 && R.spentGuardFailures==(Variant==2 ? 0:1<<Guard) &&
            R.coordinatedOfficers==(Variant==2 ? 0:(1|1<<Guard)),TEXT("real records distinguish the selected chain guard defenders and clean defense"))) return false;
        if(!Check(Scene.lines.size()==4 && Scene.lines[1].speaker==Guard &&
            Field->Presentation().Journey.CampaignChange.Contains(JourneyText(Scene.summary)),TEXT("settled summary names the actual roles without forcing conversation"))) return false;
        OpenEpisode();
        if(!Check(bEpisodeOpen && EpisodeSnapshot.Lines.Num()==4 && EpisodeSnapshot.Lines[1].Speaker==Guard && EpisodeSnapshot.Lines.Last().Speaker==3,
            TEXT("the optional main scene uses the recorded guard and protagonist"))) return false;
        StepEpisode(1);NovelHistoryText(0);VisibleNovelText();BuildInterface();Field->ExportSession(After);
        if(!Check(Before==After,TEXT("reading and redrawing the scene grant no trust resources progress or revised records"))) return false;
        CloseEpisode();if(!Check(Field->ImportSession(Before),TEXT("settled evidence survives field save/load"))) return false;
        OpenEpisode();
        if(!Check(EpisodeSnapshot.Lines.Num()==int(Scene.lines.size()) && EpisodeSnapshot.Lines[1].Text==JourneyText(Scene.lines[1].text),TEXT("reload repeats the same record-backed voice after transient battle reports are gone"))) return false;
        CloseEpisode();
        if(Variant==1)
        {
            if(!Check(!Field->CanIssue(ETacticalCommand::Continue) && Field->CanIssue(ETacticalCommand::Retry),TEXT("local roles cannot unlock victory after withdrawal"))) return false;
            Field->Issue(ETacticalCommand::Retry);
            if(!Check(Field->Presentation().Journey.bPreparation,TEXT("failed battle can return to its actual preparation"))) return false;
        }
        else
        {
            if(!Check(Field->CanIssue(ETacticalCommand::Continue),TEXT("optional scene keeps normal progression available"))) return false;
            Field->Issue(ETacticalCommand::Continue);
            if(!Check(Field->Presentation().Journey.bComplete,TEXT("the actual third-battle victory still reaches the epilogue"))) return false;
        }
    }
    CloseEpisode();Field->bLegacyJourney=Legacy;Field->ImportSession(Original);
    UE_LOG(LogTemp,Display,TEXT("GUNYU_DEFENSE_AFTERMATH_UI: PASS (actual guard/chain records, two guards, clean and withdrawn outcomes, optional four-line novel, immutable reads, save/load, epilogue and retry)"));return true;
}
