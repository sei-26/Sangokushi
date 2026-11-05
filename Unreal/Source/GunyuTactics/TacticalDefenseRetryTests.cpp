#include "TacticalTrial.h"
#include "TacticalState.h"
#include "TacticalContributionQA.h"
#include "TacticalSupportQA.h"
#include "JourneySave.hpp"
#include "JourneyPlanExample.hpp"
#include <memory>
bool ATacticalController::DefenseRetrySmokeTest()
{
    using namespace frontline;
    auto Check=[](bool V,const TCHAR* Why) { if(!V) UE_LOG(LogTemp,Error,TEXT("GUNYU_DEFENSE_RETRY_UI: FAIL %s"),Why);return V; };
    CloseEpisode();Field->Issue(ETacticalCommand::Skip);std::vector<std::uint8_t> Original,Bytes,Before,After;
    const bool Legacy=Field->bLegacyJourney;if(!Check(Field->ExportSession(Original),TEXT("original session exports"))) return false;
    for(int Variant:{0,1,2,3})
    {
        CloseEpisode();if(!Check(PrepareCooperationCouncilPreview(Field,Variant==2 ? 2:0),TEXT("actual prior campaign reaches an unpaid council"))) return false;
        Field->Issue(ETacticalCommand::PlanExample);Field->Issue(ETacticalCommand::PlanAmbush);
        auto Unpaid=std::make_unique<JourneySaveData>(),Save=std::make_unique<JourneySaveData>();
        if(!Check(Field->ExportSession(Bytes) && DecodeJourneySave(Bytes,*Unpaid),TEXT("capture the real adopted council before payment"))) return false;
        Field->Issue(ETacticalCommand::Deploy);
        if(!Check(PrepareDefenseRetryPreview(Field,Variant) && Field->ExportSession(Before) && DecodeJourneySave(Before,*Save),TEXT("real spent-guard failure or untracked withdrawal returns through normal retry"))) return false;
        const auto& J=Save->journey;const auto& Camp=Unpaid->journey.world.cities[0];
        if(!Check(J.world.cities[0].troops==Camp.troops && J.world.cities[0].food==Camp.food && J.world.cities[0].gold==Camp.gold &&
            J.officerTrust==Unpaid->journey.officerTrust && !J.history[2].completed && J.history[2].review.spentGuardFailures==0,
            TEXT("retry restores the actual unpaid resources trust and canonical history"))) return false;
        for(int A=0;A<3;++A) if(!Check(J.assignments[A].duty==Unpaid->journey.assignments[A].duty && J.assignments[A].tile==Unpaid->journey.assignments[A].tile,TEXT("the lesson cannot silently change current duties"))) return false;
        const auto Proposal=PlanExample(J);const int Free=Variant==0 ? 1:Variant==1 ? 2:0;
        if(Variant==3)
        {
            if(!Check(!J.retryDefense && !Proposal.remembersDefenseRetry,TEXT("a retreat without a checked spent guard cannot invent this lesson"))) return false;
            continue;
        }
        if(!Check(J.retryDefense && J.retryDefense->failedGuards==(1<<Free) && Proposal.remembersDefenseRetry && Proposal.adviser==Free && Proposal.assignments[Free].duty==JourneyDuty::None,
            TEXT("the recorded affected officer volunteers for free support instead of another guard duty"))) return false;
        OpenJourneyEntry();
        if(!Check(CouncilSceneVisible() && CurrentCouncilLine().Speaker==Free && Field->Presentation().Journey.ExampleAdviser==Free,TEXT("the actual affected officer opens the retry council"))) return false;
        HandleEpisodeKey(EKeys::One);Field->ExportSession(After);
        if(!Check(bNovelMap && bPlanningExample && Before==After,TEXT("the visible first reply opens the proposal without adopting or spending"))) return false;
        ApplyPlanningExample();Field->ExportSession(Bytes);DecodeJourneySave(Bytes,*Save);
        if(!Check(!bPlanningExample && Save->journey.assignments[Free].duty==JourneyDuty::None && !PlanningNotice.IsEmpty(),TEXT("only explicit adoption installs the support role"))) return false;
        int Holds=0;for(int A=0;A<3;++A) if(A!=Free) { ++Holds;if(!Check(Save->journey.assignments[A].duty==JourneyDuty::Hold,TEXT("both other officers keep real beacon guard duties"))) return false; }
        if(!Check(Holds==2 && Save->journey.world.cities[0].food==Camp.food && Save->journey.officerTrust==Unpaid->journey.officerTrust,TEXT("adoption grants neither free supplies nor trust"))) return false;
        CloseEpisode();if(!Check(Field->ImportSession(Bytes) && Field->Presentation().Journey.ExampleAdviser==Free,TEXT("saved council restores the same lesson and adviser"))) return false;
        const auto Bill=Save->journey.PreparationCosts(Save->preparation);
        Field->Issue(ETacticalCommand::Deploy);Field->ExportSession(Bytes);DecodeJourneySave(Bytes,*Save);
        if(!Check(Save->journey.stage==JourneyStage::Battle && Save->journey.heldTurns==0 && !Save->journey.retryDefense && Save->beforeBattle && Save->beforeBattle->retryDefense &&
            Save->realtime.orders[Free].kind==JourneyDirective::None && !Save->journey.chain.spent[Free] && Save->selected==Free && Field->Presentation().Selected==Free &&
            Save->journey.world.cities[0].food==Camp.food-Bill.TotalFood() && Save->journey.world.cities[0].gold==Camp.gold-Bill.gold,
            TEXT("normal payment and orders leave the support officer free while preserving the unpaid retry snapshot"))) return false;
        for(int A=0;A<3;++A) if(A!=Free && !Check(Save->realtime.orders[A].kind==JourneyDirective::March && Save->realtime.orders[A].arrival==JourneyArrival::Hold,TEXT("guards receive ordinary march and arrival hold orders"))) return false;
        auto Expected=std::make_unique<JourneySaveData>(*Save);
        if(!Check(Expected->realtime.Resolve(Expected->journey),TEXT("ordinary shared rules resolve the same paid deployment"))) return false;
        Field->Issue(ETacticalCommand::RealtimeStep);Field->Issue(ETacticalCommand::Skip);
        if(!Check(Field->ExportSession(Bytes) && DecodeJourneySave(Bytes,*Save) && Save->journey.turn==Expected->journey.turn && Save->journey.heldTurns==Expected->journey.heldTurns &&
            Save->journey.stage==Expected->journey.stage && Save->realtime.orders[Free].kind==JourneyDirective::None,TEXT("the real cycle gives exactly normal movement and objective progress"))) return false;
        for(int A=0;A<3;++A) if(!Check(Save->journey.world.armies[A].tile==Expected->journey.world.armies[A].tile && SameJourneyOrder(Save->realtime.orders[A],Expected->realtime.orders[A]),TEXT("every actual arrival and retained order matches the shared resolver"))) return false;
        Field->Issue(ETacticalCommand::Withdraw);Field->Issue(ETacticalCommand::Withdraw);Field->Issue(ETacticalCommand::Skip);Field->Issue(ETacticalCommand::Retry);
        Field->ExportSession(Bytes);DecodeJourneySave(Bytes,*Save);
        if(!Check(!Save->journey.retryDefense && Save->journey.world.cities[0].food==Camp.food,TEXT("a later attempt without new spent-guard evidence clears the old reference and refunds its own bill"))) return false;
    }
    CloseEpisode();Field->bLegacyJourney=Legacy;Field->ImportSession(Original);
    UE_LOG(LogTemp,Display,TEXT("GUNYU_DEFENSE_RETRY_UI: PASS (actual three-officer lessons, exact rollback, immutable council/map, explicit adoption, save/load, normal paid orders and new-attempt clearing)"));return true;
}
