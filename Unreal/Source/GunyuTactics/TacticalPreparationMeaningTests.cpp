#include "TacticalTrial.h"
#include "JourneySave.hpp"
#include <memory>
bool ATacticalController::PreparationMeaningSmokeTest()
{
    using namespace frontline;
    auto Check=[](bool V,const TCHAR* Why) { if(!V) UE_LOG(LogTemp,Error,TEXT("GUNYU_PREPARATION_MEANING: FAIL %s"),Why);return V; };
    CloseEpisode();Field->bLegacyJourney=false;
    auto Save=std::make_unique<JourneySaveData>();Save->journey.ResetCampaign();Save->journey.Meet();Save->journey.mission=1;Save->realtimeMode=true;
    Save->journey.AssignDuty(1,Campaign::At(9,9),JourneyDuty::Ambush);
    std::vector<std::uint8_t> Bytes;
    if(!Check(EncodeJourneySave(*Save,Bytes) && Field->ImportSession(Bytes),TEXT("new council imports"))) return false;
    const auto& Plans=Field->Presentation().Journey.Plans;
    if(!Check(Plans[0].Title==TEXT("先遣伏兵") && Plans[1].Title==TEXT("陣中補給") && Plans[1].Description.Contains(TEXT("士気＋20")) && Plans[0].Tradeoff.Contains(TEXT("露見")),TEXT("cards explain distinct playable tools and their cost"))) return false;
    const auto* Start=Field->Presentation().Journey.Map.Starts.FindByPredicate([](const FPlanningStart& V) { return V.Officer==1; });
    if(!Check(Start && Start->Tile==Campaign::At(9,9),TEXT("map previews the actual paid forward start"))) return false;
    Field->Issue(ETacticalCommand::Deploy);Field->ExportSession(Bytes);DecodeJourneySave(Bytes,*Save);
    if(!Check(Save->journey.world.armies[1].tile==Campaign::At(9,9) && Save->journey.chain.concealed[1] && !Save->journey.chain.moved[1] && !Field->CanIssue(ETacticalCommand::Resupply),TEXT("first plan delivers prepared ambush, opening action and no reserve"))) return false;
    // Restore the actual unpaid council, switch plans, and deploy through the public command.
    Save->journey=*Save->beforeBattle;Save->beforeBattle.reset();Save->preparation=1;
    EncodeJourneySave(*Save,Bytes);Field->ImportSession(Bytes);
    if(!Check(Field->Presentation().Journey.Map.Starts[1].Tile==Campaign::At(8,9),TEXT("switching preparation updates the map start"))) return false;
    Field->Issue(ETacticalCommand::Deploy);Field->ExportSession(Bytes);DecodeJourneySave(Bytes,*Save);
    if(!Check(Save->journey.reserveFood==1800 && Field->CanIssue(ETacticalCommand::Resupply) && Field->Presentation().bHasResupplyPlan,TEXT("second plan enables a real battlefield command"))) return false;
    Save->journey.world.armies[0].morale=50;Save->journey.world.armies[1].morale=60;Save->journey.world.armies[2].morale=95;
    Save->journey.chain.spent[0]=true;Save->journey.chain.signatureUsed[1]=true;
    const int Day=Save->journey.world.day,Turn=Save->journey.turn,Food=Save->journey.world.cities[0].food;
    EncodeJourneySave(*Save,Bytes);Field->ImportSession(Bytes);
    const auto PreviousElapsed=Save->realtime.elapsed;
    Field->Issue(ETacticalCommand::Resupply);
    if(!Check(Field->IsPlaying() && Field->RealtimePaused(),TEXT("supply feedback plays with the tactical clock paused"))) return false;
    Field->AdvancePresentation(6.f);Field->AdvanceRealtime(.25f);
    if(!Check(!Field->IsPlaying() && Field->RealtimePaused(),TEXT("supply feedback naturally finishes without a skip or resume command"))) return false;
    Field->ExportSession(Bytes);DecodeJourneySave(Bytes,*Save);
    if(!Check(Save->journey.world.armies[0].morale==70 && Save->journey.world.armies[1].morale==80 && Save->journey.world.armies[2].morale==100 && Save->journey.world.armies[0].food==1600,TEXT("actual button distributes paid rations and restores capped morale"))) return false;
    if(!Check(Save->realtime.elapsed==PreviousElapsed && Save->journey.world.day==Day && Save->journey.turn==Turn && Save->journey.world.cities[0].food==Food && Save->journey.chain.spent[0] && Save->journey.chain.signatureUsed[1] && Field->RealtimePaused(),TEXT("support pauses for review without granting extra actions, signatures or time"))) return false;
    if(!Check(!Field->CanIssue(ETacticalCommand::Resupply) && Save->journey.reserveFood==0 && Field->Presentation().ResupplyLabel.Contains(TEXT("なし")),TEXT("used reserve has visible feedback and cannot be reused"))) return false;
    if(!Check(Field->ImportSession(Bytes) && !Field->CanIssue(ETacticalCommand::Resupply),TEXT("save/load cannot recreate a spent reserve"))) return false;
    Field->Issue(ETacticalCommand::Withdraw);Field->Issue(ETacticalCommand::Withdraw);Field->Issue(ETacticalCommand::Skip);Field->Issue(ETacticalCommand::Retry);
    Field->ExportSession(Bytes);DecodeJourneySave(Bytes,*Save);
    if(!Check(Save->journey.stage==JourneyStage::Preparation && Save->journey.reserveFood==0 && Save->journey.world.cities[0].food==Save->beforeBattle->world.cities[0].food,TEXT("retry restores unpaid council, not a second supply reward"))) return false;
    Field->bLegacyJourney=true;CloseEpisode();
    UE_LOG(LogTemp,Display,TEXT("GUNYU_PREPARATION_MEANING: PASS (paid ambush start/map/action, reserve command, real capped morale/food, no extra turn/signatures, save/load and retry)"));return true;
}
