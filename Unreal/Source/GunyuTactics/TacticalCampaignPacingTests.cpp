#include "TacticalTrial.h"
#include "JourneySave.hpp"
#include <memory>
bool ATacticalController::CampaignPacingSmokeTest()
{
    using namespace frontline;
    auto Check=[](bool V,const TCHAR* Why) { if(!V) UE_LOG(LogTemp,Error,TEXT("GUNYU_CAMPAIGN_PACING: FAIL %s"),Why);return V; };
    CloseEpisode();Field->bLegacyJourney=false;
    auto Save=std::make_unique<JourneySaveData>();Save->journey.ResetCampaign();Save->journey.Meet();Save->journey.mission=1;Save->journey.SelectDifficulty(JourneyDifficulty::Story);Save->realtimeMode=true;
    std::vector<std::uint8_t> Bytes,Before,After;
    if(!Check(EncodeJourneySave(*Save,Bytes) && Field->ImportSession(Bytes),TEXT("new campaign council imports"))) return false;
    Field->ExportSession(Before);OpenJourneyEntry();Field->ExportSession(After);
    if(!Check(bNovelPlan && bEpisodeOpen && Before==After && Field->Presentation().Journey.CampaignPhase.Contains(TEXT("河港")),TEXT("council goes directly to a plot-aware operation review without spending"))) return false;
    TogglePlanningMap();bPlanningExample=true;ApplyPlanningExample();HandleEpisodeKey(EKeys::D);
    Field->ExportSession(Bytes);DecodeJourneySave(Bytes,*Save);
    if(!Check(Save->journey.world.armies.size()==5 && !Save->journey.HasConvoy() && Field->Presentation().GoalLabel==TEXT("河港の指揮所"),TEXT("real deployment is an assault with no cart or cargo"))) return false;
    // Settled-objective fixture: UX and next chapter transitions, not combat balance.
    Save->journey.world.armies[3].troops=Save->journey.world.armies[4].troops=0;
    Save->journey.world.armies[0].tile=Save->journey.goal;Save->journey.RefreshIntelligence();Save->journey.EndTurn();
    if(!Check(Save->journey.won && Save->journey.CanContinue() && Save->journey.NeedsStoryReply(),TEXT("win allows next chapter with the vow still optional"))) return false;
    EncodeJourneySave(*Save,Bytes);Field->ImportSession(Bytes);OpenJourneyEntry();
    if(!Check(!bEpisodeOpen && Field->CanIssue(ETacticalCommand::Continue) && Field->Presentation().Journey.Dialogue.Contains(TEXT("共同作戦")),TEXT("debrief shows story consequences without mandatory repeated dialogue"))) return false;
    Field->Issue(ETacticalCommand::Continue);Field->ExportSession(Bytes);DecodeJourneySave(Bytes,*Save);
    if(!Check(Save->journey.mission==2 && Field->Presentation().Journey.CampaignPhase.Contains(TEXT("数週間")),TEXT("one victory advances to the final operation"))) return false;
    // An old battle keeps its convoy, but retry restores unpaid supplies then upgrades the next attempt.
    auto Old=std::make_unique<JourneySaveData>();Old->journey.Reset();Old->journey.Meet();Old->journey.mission=1;Old->beforeBattle=Old->journey;Old->journey.Prepare(0,1);
    const int Food=Old->beforeBattle->world.cities[0].food;
    EncodeJourneySave(*Old,Bytes);Field->ImportSession(Bytes);Field->ExportSession(Bytes);DecodeJourneySave(Bytes,*Old);
    if(!Check(Old->journey.HasConvoy(),TEXT("an old in-flight escort is never silently reinterpreted"))) return false;
    Field->Issue(ETacticalCommand::Withdraw);Field->Issue(ETacticalCommand::Withdraw);Field->Issue(ETacticalCommand::Skip);Field->Issue(ETacticalCommand::Retry);
    Field->ExportSession(Bytes);DecodeJourneySave(Bytes,*Old);
    if(!Check(Old->journey.campaignFlow && !Old->journey.HasConvoy() && Old->journey.world.cities[0].food==Food,TEXT("retry upgrades the new attempt to capture only after restoring the real unpaid council"))) return false;
    CloseEpisode();Field->bLegacyJourney=true;
    UE_LOG(LogTemp,Display,TEXT("GUNYU_CAMPAIGN_PACING: PASS (direct council, real capture objective, no cart, optional vow, short aftermath and campaign advance)"));return true;
}
