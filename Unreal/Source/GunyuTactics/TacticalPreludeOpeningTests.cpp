#include "TacticalTrial.h"
#include "JourneySave.hpp"
#include "JourneyPreludeOpening.hpp"
#include <memory>
using namespace frontline;
bool ATacticalController::PreparePreludeOpeningPreview(int Phase,int Choice)
{
    if(Phase<0 || Phase>6 || Choice<0 || Choice>1 || (Phase==6 && Choice!=1)) return false;
    CloseEpisode();Field->Issue(ETacticalCommand::Skip);Field->bLegacyJourney=false;Field->bRealtimeCommand=true;
    auto S=std::make_unique<JourneySaveData>();auto& J=S->journey;
    J.ResetCampaign();J.Meet();J.grandCampaign=J.grandStrategyCampaign=J.yangrenStrategyCampaign=J.coalitionBattleCampaign=J.shortPrelude=true;
    J.SelectDifficulty(JourneyDifficulty::Story);S->realtimeMode=true;S->realtime.paused=true;S->pauseOnAlert=false;
    std::vector<std::uint8_t> Bytes;if(!EncodeJourneySave(*S,Bytes) || !Field->ImportSession(Bytes)) return false;
    bDetailedBattle=false;OpenJourneyEntry();
    if(!CouncilSceneVisible()) return false;
    if(Phase>0) ChoosePreludeOpening(Choice);
    if(Phase>=2)
    {
        DeployNovelPlan();if(!Field->Presentation().bBattle) return false;
        if(Phase==5) { Field->Issue(ETacticalCommand::Withdraw);Field->Issue(ETacticalCommand::Withdraw);Field->AdvancePresentation(60); }
        else if(Phase==6)
        {
            Field->SelectOfficer(2);Field->Issue(ETacticalCommand::Move);Field->CommandAt(Campaign::At(12,11));
            for(int I=0;I<12 && Field->Presentation().bBattle;++I)
            { Field->Issue(ETacticalCommand::RealtimeStep);Field->AdvancePresentation(60); }
            Field->ExportSession(Bytes);
            if(!DecodeJourneySave(Bytes,*S) || !S->journey.won || S->journey.battleReview.resolution!=JourneyResolution::ExitSecured) return false;
        }
        else if(Phase>=3)
        {
            Field->Issue(ETacticalCommand::AutoBattle);
            for(int I=0;I<24 && Field->Presentation().bBattle;++I)
            { Field->AdvanceRealtime(12.1f);Field->AdvancePresentation(60);Field->AdvanceRealtime(.1f);Field->AdvancePresentation(60); }
            if(Field->IsAutoBattle()) Field->StopAutoBattle();
            Field->ExportSession(Bytes);if(!DecodeJourneySave(Bytes,*S) || !S->journey.won) return false;
        }
    }
    if(Phase==4 || Phase==5) OpenEpisode();
    if(Phase==5)
    {
        Field->ExportSession(Bytes);DecodeJourneySave(Bytes,*S);
        UE_LOG(LogTemp,Display,TEXT("GUNYU_PRELUDE_WITHDRAWAL: stage=%d playing=%d opened=%d lines=%d retry=%d before=%d"),
            int(S->journey.stage),Field->IsPlaying(),bEpisodeOpen,EpisodeSnapshot.Lines.Num(),Field->CanIssue(ETacticalCommand::Retry),S->beforeBattle.has_value());
    }
    CouncilElapsed=NovelElapsed=1000;return true;
}
bool ATacticalController::PreludeOpeningSmokeTest()
{
    auto Check=[](bool V,const TCHAR* Why) { if(!V) UE_LOG(LogTemp,Error,TEXT("GUNYU_PRELUDE_OPENING: FAIL %s"),Why);return V; };
    if(!Check(PreparePreludeOpeningPreview(0,0) && CouncilExchange.Lines.Num()==3 &&
        CouncilChoiceLabel(0).Contains(TEXT("関羽")) && CouncilChoiceLabel(1).Contains(TEXT("劉備")),TEXT("a short crisis leads straight to two actual opening plans"))) return false;
    std::vector<std::uint8_t> Before,After;Field->ExportSession(Before);
    OpenCouncilMap();HandleEpisodeKey(EKeys::Escape);Field->ExportSession(After);
    if(!Check(Before==After,TEXT("reading the map adopts and spends nothing"))) return false;
    for(int Choice:{0,1})
    {
        if(!Check(PreparePreludeOpeningPreview(1,Choice),TEXT("the explicit choice adopts a plan"))) return false;
        auto S=std::make_unique<JourneySaveData>();Field->ExportSession(Before);
        if(!Check(DecodeJourneySave(Before,*S) && MatchesPreludeOpening(S->journey,Choice) && S->preparation==Choice &&
            S->journey.world.cities[0].food==18000 && S->journey.trust==0 && CouncilExchange.Lines.Num()==2 &&
            CouncilChoiceLabel(Choice).Contains(TEXT("採用中")),TEXT("roles and supplies are selected together without paying or awarding trust"))) return false;
        if(!Check(Field->ImportSession(Before) && Field->PreludePlanLabel(Choice).Contains(TEXT("採用中")),TEXT("adopted roles survive save and reload without new save fields"))) return false;
        OpenJourneyEntry();DeployNovelPlan();Field->ExportSession(After);DecodeJourneySave(After,*S);
        if(!Check(S->journey.stage==JourneyStage::Battle && S->realtime.paused &&
            S->journey.world.cities[0].food==18000-S->journey.PreparationCosts(Choice).TotalFood() &&
            (Choice==0 ? S->journey.chain.concealed[1]:S->realtime.orders[0].kind==JourneyDirective::March) &&
            !Field->ChoosePreludePlan(1-Choice),TEXT("deployment pays once and creates a real ambush or march, with no stale battle adoption"))) return false;
        if(Choice==0)
        {
            Field->ExportSession(Before);OpenFirstCommand();Field->ExportSession(After);
            if(!Check(bTacticPickerOpen && Field->RealtimePaused() && Before==After,
                TEXT("the opening cue leads to Liu Bei's tactics without sending a unit onto an occupied enemy tile or spending an action"))) return false;
            ToggleTacticPicker();
        }
        if(!Check(PreparePreludeOpeningPreview(3,Choice),TEXT("both paid opening plans can finish a real initial battle"))) return false;
        const auto& P=Field->Presentation().Journey;
        if(!Check(P.CampaignChange.Contains(TEXT("「")) && P.Episode.Lines.Num()==4 && Field->CanIssue(ETacticalCommand::Continue),
            TEXT("a real win gets a named response and optional four-line aftermath without gating the next chapter"))) return false;
        Field->ExportSession(Before);OpenEpisode();StepEpisode(1);CloseEpisode();Field->ExportSession(After);
        if(!Check(Before==After,TEXT("reading the aftermath changes no battle result or trust"))) return false;
        Field->Issue(ETacticalCommand::Continue);
        if(!Check(Field->Presentation().Mission==YellowTurbanMission,TEXT("the initial battle still leads directly to the Yellow Turban chapter"))) return false;
    }
    if(!Check(PreparePreludeOpeningPreview(6,1) && Field->Presentation().Journey.CampaignChange.Contains(TEXT("敵の旗は残っている")),TEXT("a manual march and southern diversion secure the exit with the enemy still alive and get a different response"))) return false;
    if(!Check(PreparePreludeOpeningPreview(5,0) && EpisodeSnapshot.Lines.Num()==4 && Field->CanIssue(ETacticalCommand::Retry),TEXT("a withdrawn first battle has its own voice and keeps retry"))) return false;
    CloseEpisode();UE_LOG(LogTemp,Display,TEXT("GUNYU_PRELUDE_OPENING: PASS (short crisis, explicit real plans, immutable reading, saved roles, paid deployment, actual victories, optional aftermath and historical continuation)"));return true;
}
