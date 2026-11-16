#include "TacticalTrial.h"
#include "JourneySave.hpp"
#include "TacticalContributionQA.h"
#include "Misc/Parse.h"
#include "Misc/CommandLine.h"
#include <memory>
bool ATacticalController::CampaignPacingSmokeTest()
{
    using namespace frontline;
    auto Check=[](bool V,const TCHAR* Why) { if(!V) UE_LOG(LogTemp,Error,TEXT("GUNYU_CAMPAIGN_PACING: FAIL %s"),Why);return V; };
    CloseEpisode();Field->bLegacyJourney=false;
    auto Save=std::make_unique<JourneySaveData>();Save->journey.ResetCampaign();Save->journey.Meet();Save->journey.mission=1;Save->journey.SelectDifficulty(JourneyDifficulty::Story);Save->realtimeMode=true;
    std::vector<std::uint8_t> Bytes,Before,After;
    auto Shortage=std::make_unique<JourneySaveData>();
    Shortage->journey.ResetCampaign();Shortage->journey.Meet();
    Shortage->journey.world.cities[0].food=0;
    if(!Check(EncodeJourneySave(*Shortage,Bytes) && Field->ImportSession(Bytes),TEXT("restore a real council without food"))) return false;
    OpenJourneyEntry();
    if(!Check(bNovelPlan && !Field->CanIssue(ETacticalCommand::Deploy) && Field->CanIssue(ETacticalCommand::AcceptSetback) &&
        Field->Presentation().Journey.SetbackLabel.Contains(TEXT("出陣を見合わせ")),TEXT("the normal council shows an explicit path out of resource shortage"))) return false;
    if(FParse::Param(FCommandLine::Get(),TEXT("StorySetbackPreview")))
    {
        CouncilElapsed=1000;CouncilStageTime=CouncilLineTime=1;
        return true;
    }
    ResolveCampaignSetback();
    if(!Check(bEpisodeOpen && Field->Presentation().Journey.bComplete && !Field->CanIssue(ETacticalCommand::Retry) &&
        Field->ExportSession(Bytes) && DecodeJourneySave(Bytes,*Shortage) &&
        Shortage->journey.history[0].review.resolution==JourneyResolution::Unfought && Shortage->journey.world.cities[0].food==0,
        TEXT("the council choice plays an honest scene and saves the unchanged supplies"))) return false;
    EpisodePage=EpisodeSnapshot.Lines.Num()-1;NovelElapsed=1000;ReadNextNovel();
    if(!Check(Field->Presentation().Journey.bPreparation && Field->Presentation().Mission==1,
        TEXT("reading the last line advances to the next real council"))) return false;
    CloseEpisode();
    Shortage->journey.ResetCampaign();Shortage->journey.Meet();Shortage->journey.world.cities[0].gold=0;
    if(!Check(EncodeJourneySave(*Shortage,Bytes) && Field->ImportSession(Bytes) && Field->CanIssue(ETacticalCommand::Deploy),
        TEXT("ordinary departure is available with zero military funds"))) return false;
    Field->Issue(ETacticalCommand::Deploy);
    Field->Issue(ETacticalCommand::Withdraw);Field->Issue(ETacticalCommand::Withdraw);Field->Issue(ETacticalCommand::Skip);
    if(!Check(Field->CanIssue(ETacticalCommand::Retry) && Field->CanIssue(ETacticalCommand::AcceptSetback),
        TEXT("actual withdrawal gives a choice between retry and accepting the result"))) return false;
    ResolveCampaignSetback();
    if(!Check(Field->ExportSession(Bytes) && DecodeJourneySave(Bytes,*Shortage) &&
        Shortage->journey.history[0].withdrawn && !Shortage->journey.history[0].won,
        TEXT("accepting an actual withdrawal preserves the real result on disk"))) return false;
    CloseEpisode();
    UE_LOG(LogTemp,Display,TEXT("GUNYU_STORY_SETBACK: PASS (zero-gold deployment, no-food council, next episode, actual withdrawal, retry and save)"));
    if(!Check(EncodeJourneySave(*Save,Bytes) && Field->ImportSession(Bytes),TEXT("new campaign council imports"))) return false;
    Field->ExportSession(Before);OpenJourneyEntry();Field->ExportSession(After);
    if(!Check(bNovelPlan && bEpisodeOpen && Before==After && Field->Presentation().Journey.CampaignPhase.Contains(TEXT("河港")),TEXT("council goes directly to a plot-aware operation review without spending"))) return false;
    OpenCouncilVoice(2);
    if(!Check(CurrentCouncilLine().Text.Contains(TEXT("南橋")),TEXT("Zhang Fei explains how to draw the counterattack away from the capture party"))) return false;
    HandleEpisodeKey(EKeys::M);
    if(!Check(PlanningTile==Campaign::At(11,11) && PlanningTitle()==TEXT("南橋"),TEXT("river-port advice selects the actual bridge"))) return false;
    OpenPlanningExample();ApplyPlanningExample();HandleEpisodeKey(EKeys::D);
    Field->ExportSession(Bytes);DecodeJourneySave(Bytes,*Save);
    if(!Check(Save->journey.world.armies.size()==5 && !Save->journey.HasConvoy() && Field->Presentation().GoalLabel==TEXT("河港の指揮所"),TEXT("real deployment is an assault with no cart or cargo"))) return false;
    const auto& Battle=Field->Presentation();
    if(!Check(Save->selected==2 && Battle.Selected==2 && Battle.Officers.Num()==3 &&
        Battle.Officers[0].DutyBadge==TEXT("指定地点を守る") && Battle.Officers[1].DutyBadge==TEXT("伏兵の準備") &&
        Battle.Officers[2].DutyBadge==TEXT("自由に指揮") && Save->realtime.orders[2].kind==JourneyDirective::None &&
        Save->realtime.orders[0].kind==JourneyDirective::March,
        TEXT("adopted river-port duties remain visible and the free officer is ready for the player's first order"))) return false;
    // Settled-objective fixture: UX and next chapter transitions, not combat balance.
    Save->journey.world.armies[3].troops=Save->journey.world.armies[4].troops=0;
    Save->journey.world.armies[0].tile=Save->journey.goal;Save->journey.RefreshIntelligence();Save->journey.EndTurn();
    if(!Check(Save->journey.won && Save->journey.CanContinue() && Save->journey.NeedsStoryReply(),TEXT("win allows next chapter with the vow still optional"))) return false;
    EncodeJourneySave(*Save,Bytes);Field->ImportSession(Bytes);OpenJourneyEntry();
    if(!Check(!bEpisodeOpen && Field->CanIssue(ETacticalCommand::Continue) && Field->Presentation().Journey.Dialogue.Contains(TEXT("共同作戦")),TEXT("debrief shows story consequences without mandatory repeated dialogue"))) return false;
    Field->Issue(ETacticalCommand::Continue);Field->ExportSession(Bytes);DecodeJourneySave(Bytes,*Save);
    if(!Check(Save->journey.mission==2 && Field->Presentation().Journey.CampaignPhase.Contains(TEXT("数週間")),TEXT("one victory advances to the final operation"))) return false;
    OpenJourneyEntry();OpenCouncilVoice(2);
    if(!Check(CurrentCouncilLine().Text.Contains(TEXT("退いても追わねえ")),TEXT("defense advice calls for holding ground instead of leaving its beacon"))) return false;
    HandleEpisodeKey(EKeys::M);
    if(!Check(PlanningTile==Save->journey.Definition().secondGoal && PlanningTitle()==TEXT("南の烽火"),TEXT("defense advice selects the southern beacon"))) return false;
    CloseEpisode();
    // An old battle keeps its convoy, but retry restores unpaid supplies then upgrades the next attempt.
    auto Old=std::make_unique<JourneySaveData>();Old->journey.Reset();Old->journey.Meet();Old->journey.mission=1;Old->beforeBattle=Old->journey;Old->journey.Prepare(0,1);
    const int Food=Old->beforeBattle->world.cities[0].food;
    EncodeJourneySave(*Old,Bytes);Field->ImportSession(Bytes);Field->ExportSession(Bytes);DecodeJourneySave(Bytes,*Old);
    if(!Check(Old->journey.HasConvoy(),TEXT("an old in-flight escort is never silently reinterpreted"))) return false;
    Field->Issue(ETacticalCommand::Withdraw);Field->Issue(ETacticalCommand::Withdraw);Field->Issue(ETacticalCommand::Skip);Field->Issue(ETacticalCommand::Retry);
    Field->ExportSession(Bytes);DecodeJourneySave(Bytes,*Old);
    if(!Check(Old->journey.campaignFlow && !Old->journey.HasConvoy() && Old->journey.world.cities[0].food==Food,TEXT("retry upgrades the new attempt to capture only after restoring the real unpaid council"))) return false;
    for(int Variant=0;Variant<4;++Variant)
    {
        CloseEpisode();if(!Check(PrepareContributionPreview(Field,Variant),TEXT("actual shared-rule contribution playthrough imports"))) return false;
        OpenJourneyEntry();
        if(!Check(!bEpisodeOpen && Field->CanIssue(ETacticalCommand::Continue),TEXT("named contribution does not force another scene or gate continuation"))) return false;
        const auto& P=Field->Presentation().Journey;const TCHAR* Names[]={TEXT("劉備"),TEXT("関羽"),TEXT("張飛")};
        const auto Summary=P.CampaignChange;
        if(!Check(Variant==3 ? Summary.Contains(TEXT("連鎖に参加")):Summary.Contains(FString(TEXT("河港に旗を立てた："))+Names[Variant]),TEXT("normal debrief names the actual captor or saved chain participants"))) return false;
        if(!Check(Variant!=2 || !Summary.Contains(TEXT("連鎖に参加")),TEXT("ordinary attacks are never mislabeled as a cooperative tactic"))) return false;
        OpenEpisode();Field->ExportSession(Before);BuildInterface();StepEpisode(1);NovelHistoryText(0);Field->ExportSession(After);
        if(!Check(EpisodeSnapshot.Lines.Num()>=3 && EpisodeSnapshot.Lines.Num()<=4 && (Variant==3 || EpisodeSnapshot.Lines[1].Speaker==Variant) && Before==After,TEXT("actual speaker, portrait and optional short scene preserve the real save"))) return false;
        CloseEpisode();if(!Check(Field->ImportSession(Before),TEXT("contribution debrief reloads"))) return false;
        OpenEpisode();if(!Check(Field->Presentation().Journey.CampaignChange==Summary,TEXT("reload preserves the same named contribution without new rewards"))) return false;
        CloseEpisode();Field->Issue(ETacticalCommand::Continue);Field->ExportSession(Bytes);DecodeJourneySave(Bytes,*Save);
        if(!Check(Save->journey.stage==JourneyStage::Preparation && Save->journey.mission==(Variant==3 ? 1:2),TEXT("the named debrief continues through the ordinary chapter transition"))) return false;
    }
    UE_LOG(LogTemp,Display,TEXT("GUNYU_CONTRIBUTION_AFTERMATH: PASS (real three-captor and chain playthroughs, ordinary-attack witness, actual speakers, optional immutable scenes, save/load and chapter advance)"));
    CloseEpisode();Field->bLegacyJourney=true;
    UE_LOG(LogTemp,Display,TEXT("GUNYU_CAMPAIGN_PACING: PASS (direct council, real capture objective, no cart, optional vow, short aftermath and campaign advance)"));return true;
}
