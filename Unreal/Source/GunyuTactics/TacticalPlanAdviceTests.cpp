#include "TacticalTrial.h"
#include "JourneySave.hpp"
#include "JourneyPlanExample.hpp"
#include "TacticalContributionQA.h"
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
    for(int Variant:{0,3})
    {
        CloseEpisode();if(!Check(PrepareCooperationCouncilPreview(Field,Variant),TEXT("actual prior victory reaches its next unpaid council"))) return false;
        Field->ExportSession(Before);OpenJourneyEntry();BuildInterface();Field->ExportSession(After);
        const int Duty=Variant==3 ? 2:1,Destination=Variant==3 ? Campaign::At(10,10):Campaign::At(7,6);
        if(!Check(CouncilSceneVisible() && CurrentCouncilLine().Speaker==Duty && Field->Presentation().Journey.ExampleAdviser==Duty && Before==After,TEXT("actual participant volunteers without changing the plan or paying"))) return false;
        HandleEpisodeKey(EKeys::One);Field->ExportSession(After);
        if(!Check(bNovelMap && bPlanningExample && Before==After,TEXT("the first visible reply opens the same read-only proposal with keyboard input"))) return false;
        ApplyPlanningExample();Field->ExportSession(Bytes);DecodeJourneySave(Bytes,*Save);
        const auto Unpaid=Save->journey.world.cities[0];const auto Bill=Save->journey.PreparationCosts(Save->preparation);
        if(!Check(Save->journey.assignments[Duty].tile==Destination && Save->journey.assignments[0].duty==JourneyDuty::None && Save->journey.officerTrust==Save->beforeBattle->officerTrust,TEXT("explicit adoption changes the real roles and frees Liu without awarding trust"))) return false;
        CloseEpisode();if(!Check(Field->ImportSession(Bytes),TEXT("adopted cooperation proposal reloads"))) return false;
        OpenJourneyEntry();DeployNovelPlan();Field->ExportSession(Bytes);DecodeJourneySave(Bytes,*Save);
        if(!Check(Save->journey.stage==JourneyStage::Battle && Field->RealtimePaused() && Save->realtime.orders[0].kind==JourneyDirective::None && Save->realtime.orders[Duty].kind==JourneyDirective::March && Save->realtime.orders[Duty].destination==Destination &&
            Save->journey.world.cities[0].gold==Unpaid.gold-Bill.gold && Save->journey.world.cities[0].food==Unpaid.food-Bill.TotalFood(),TEXT("normal paid deployment installs only the adopted marching duties and starts paused"))) return false;
        Field->Issue(ETacticalCommand::Withdraw);Field->Issue(ETacticalCommand::Withdraw);Field->Issue(ETacticalCommand::Skip);Field->Issue(ETacticalCommand::Retry);
        Field->ExportSession(Bytes);DecodeJourneySave(Bytes,*Save);
        if(!Check(Save->journey.stage==JourneyStage::Preparation && Save->journey.assignments[Duty].tile==Destination && Save->journey.world.cities[0].food==Unpaid.food && Save->journey.world.cities[0].gold==Unpaid.gold,TEXT("retry restores the unpaid council while retaining the chosen roles"))) return false;
    }
    UE_LOG(LogTemp,Display,TEXT("GUNYU_COOPERATION_PLAN: PASS (actual prior victories, volunteered roles, read-only keyboard preview, explicit adoption, normal paid orders, save/load and retry)"));
    CloseEpisode();Field->bLegacyJourney=true;
    UE_LOG(LogTemp,Display,TEXT("GUNYU_PLAN_ADVICE: PASS (evidence source, immutable map preview, explicit adoption, actual deployment, paid costs, save/load/retry and free officer)"));return true;
}
