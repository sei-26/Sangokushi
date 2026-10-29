#include "TacticalTrial.h"
#include "JourneySave.hpp"
#include "JourneyPlanExample.hpp"
#include <memory>
bool ATacticalController::PlanAdviceSmokeTest()
{
    using namespace frontline;
    auto Check=[](bool V,const TCHAR* Why) { if(!V) UE_LOG(LogTemp,Error,TEXT("GUNYU_PLAN_ADVICE: FAIL %s"),Why);return V; };
    CloseEpisode();Field->bLegacyJourney=false;
    auto Save=std::make_unique<JourneySaveData>();Save->journey.ResetCampaign();Save->journey.Meet();Save->journey.mission=1;Save->realtimeMode=true;
    auto& H=Save->journey.history[0];H.completed=H.won=H.campaignFlow=true;H.survivors=4200;H.review.preparationTracked=true;H.review.openingAmbushes=1;
    Save->journey.AssignDuty(0,Campaign::At(9,10),JourneyDuty::Hold);
    std::vector<std::uint8_t> Bytes,Before,After;
    if(!Check(EncodeJourneySave(*Save,Bytes) && Field->ImportSession(Bytes),TEXT("settled previous battle can supply a new council proposal"))) return false;
    const auto& P=Field->Presentation().Journey;
    if(!Check(P.ExampleAdviser==1 && P.ExampleReason.Contains(TEXT("初手の伏兵")) && P.ExampleDuties.Num()==3 && P.ExampleDuties[1].Tile==Campaign::At(9,7) && P.Map.Duties[0].Tile==Campaign::At(9,10),TEXT("proposal and current orders remain distinct typed data"))) return false;
    Field->ExportSession(Before);OpenEpisode();OpenNovelPlan();OpenPlanningExample();Field->ExportSession(After);
    if(!Check(bNovelMap && bPlanningExample && Before==After && PlanningPreviewRoute().IsEmpty(),TEXT("direct proposal review opens a read-only preview with no stale route"))) return false;
    ApplyPlanningExample();Field->ExportSession(Bytes);DecodeJourneySave(Bytes,*Save);
    auto& J=Save->journey;const auto Cost=J.PreparationCosts(0);const auto Camp=J.world.cities[0];
    if(!Check(!bPlanningExample && PlanningNotice.Contains(TEXT("採用")) && J.assignments[0].tile==Campaign::At(10,8) && J.assignments[1].tile==Campaign::At(9,7) && J.assignments[2].duty==JourneyDuty::None && J.world.cities[0].gold==900,TEXT("explicit adoption applies only real roles and shows acknowledgement"))) return false;
    if(!Check(Field->ImportSession(Bytes) && Field->Presentation().Journey.Map.Duties[1].Tile==Campaign::At(9,7),TEXT("adopted positions survive save/load"))) return false;
    CloseEpisode();Field->Issue(ETacticalCommand::Deploy);Field->ExportSession(Bytes);DecodeJourneySave(Bytes,*Save);
    if(!Check(J.world.armies[1].tile==Campaign::At(9,7) && J.world.cities[0].food==Camp.food-Cost.TotalFood() && J.world.cities[0].gold==Camp.gold-Cost.gold && Field->RealtimePaused(),TEXT("proposal connects to the real forward deployment and exact normal bill"))) return false;
    Field->Issue(ETacticalCommand::Withdraw);Field->Issue(ETacticalCommand::Withdraw);Field->Issue(ETacticalCommand::Skip);Field->Issue(ETacticalCommand::Retry);
    Field->ExportSession(Bytes);DecodeJourneySave(Bytes,*Save);
    if(!Check(J.stage==JourneyStage::Preparation && J.world.cities[0].food==Camp.food && J.assignments[1].tile==Campaign::At(9,7),TEXT("retry retains the adopted proposal and restores unpaid supplies"))) return false;
    Save=std::make_unique<JourneySaveData>();Save->journey.ResetCampaign();Save->journey.Meet();Save->journey.mission=2;Save->realtimeMode=true;
    auto& Previous=Save->journey.history[1];Previous.completed=Previous.won=true;Previous.survivors=4200;Previous.review.preparationTracked=true;Previous.review.resupplyTurn=2;Previous.review.postSupplySteps=Previous.review.tacticSteps=1;
    if(!Check(EncodeJourneySave(*Save,Bytes) && Field->ImportSession(Bytes),TEXT("previous supply and subsequent tactic evidence imports"))) return false;
    if(!Check(Field->Presentation().Journey.ExampleAdviser==2 && Field->Presentation().Journey.ExampleDuties[1].Kind==0 && Field->Presentation().Journey.ExampleDuties[2].Tile==Campaign::At(8,9),TEXT("Zhang volunteers for southern defense while Guan stays free"))) return false;
    Field->Issue(ETacticalCommand::PlanExample);Field->Issue(ETacticalCommand::Deploy);Field->ExportSession(Bytes);DecodeJourneySave(Bytes,*Save);
    if(!Check(Save->realtime.orders[1].kind==JourneyDirective::None && Save->realtime.orders[2].arrival==JourneyArrival::Hold,TEXT("the free officer receives no hidden standing order"))) return false;
    CloseEpisode();Field->bLegacyJourney=true;
    UE_LOG(LogTemp,Display,TEXT("GUNYU_PLAN_ADVICE: PASS (evidence source, immutable map preview, explicit adoption, actual deployment, paid costs, save/load/retry and free officer)"));return true;
}
