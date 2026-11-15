#include "TacticalTrial.h"
#include "JourneySave.hpp"
#include <memory>

bool ATacticalController::OpeningOrderSmokeTest()
{
    using namespace frontline;
    auto Check=[](bool Valid,const TCHAR* Reason)
    {
        if(!Valid) UE_LOG(LogTemp,Error,TEXT("GUNYU_OPENING_ORDER: FAIL %s"),Reason);
        return Valid;
    };
    TGuardValue<bool> Legacy(Field->bLegacyJourney,false);
    TGuardValue<bool> Detail(bDetailedBattle,false);
    std::unique_ptr<JourneySaveData> Save;
    std::vector<std::uint8_t> Bytes;
    auto OpenCouncil=[&](int Mission,bool Realtime)
    {
        CloseEpisode();
        Save=std::make_unique<JourneySaveData>();
        Save->journey.ResetCampaign();
        Save->journey.Meet();
        Save->journey.mission=Mission;
        Save->realtimeMode=Realtime;
        if(!EncodeJourneySave(*Save,Bytes) || !Field->ImportSession(Bytes)) return false;
        OpenJourneyEntry();
        return CouncilSceneVisible();
    };
    auto Record=[&]
    {
        return Field->ExportSession(Bytes) && DecodeJourneySave(Bytes,*Save);
    };
    if(!Check(OpenCouncil(0,false),TEXT("unpaid escort council"))) return false;
    Field->Issue(ETacticalCommand::PromiseRescue);
    Field->Issue(ETacticalCommand::RouteSouthernWood);
    OpenCouncilVoice(2);
    HandleEpisodeKey(EKeys::M);
    const int Bend=PlanningTile;
    HandleEpisodeKey(EKeys::D);
    if(!Check(ShowOpeningOrder() && Field->Presentation().OpeningOrder.Officer==2 &&
        Field->Presentation().OpeningOrder.Tile==Bend && Field->Presentation().OpeningOrder.Place.Contains(TEXT("曲がり角")),
        TEXT("deploy carries the southern role marker out of the actual council map"))) return false;
    if(!Check(Record(),TEXT("first escort save"))) return false;
    const int Start=Save->journey.world.armies[2].tile;
    OpenFirstCommand();
    if(!Check(Record() && Save->selected==2 && Save->journey.world.armies[2].tile==Start &&
        Save->journey.turn==1 && Field->Presentation().Hint.Contains(TEXT("移動候補")),TEXT("manual preview selects the officer and allows a normal adjacent move without moving the army"))) return false;
    Field->Issue(ETacticalCommand::EndTurn);
    Field->Issue(ETacticalCommand::Skip);
    if(!Check(!ShowOpeningOrder(),TEXT("real turn progression dismisses the initial guidance"))) return false;

    if(!Check(OpenCouncil(1,true),TEXT("unpaid river-port council"))) return false;
    HandleEpisodeKey(EKeys::D);
    if(!Check(ShowOpeningOrder() && Field->Presentation().OpeningOrder.Officer==2 &&
        Field->Presentation().OpeningOrder.Tile==Campaign::At(11,11),TEXT("freely deployed Zhang Fei is cued to the actual bridge"))) return false;
    OpenFirstCommand();
    if(!Check(Record() && Save->selected==2 && Save->realtime.paused &&
        Save->realtime.orders[2].kind==JourneyDirective::None,TEXT("opening the march input leaves time paused and does not queue an order"))) return false;
    const int Approach=Campaign::At(10,11);
    Field->ClickTile(Approach);
    if(!Check(Record() && Save->realtime.orders[2].destination==Approach && Save->realtime.paused &&
        Field->Presentation().OpeningOrder.Tile==Approach && Field->Presentation().OpeningOrder.Action==EOpeningOrderAction::Inspect,
        TEXT("the normal map command queues the selected approach, and the cue shows the actual order"))) return false;
    Field->Issue(ETacticalCommand::RealtimePause);
    Field->Issue(ETacticalCommand::RealtimePause);
    if(!Check(!ShowOpeningOrder(),TEXT("resuming then pausing cannot resurrect the introduction"))) return false;
    if(!Check(Record() && Field->ImportSession(Bytes) && !ShowOpeningOrder(),TEXT("loading a battle never replays the opening guidance"))) return false;

    if(!Check(OpenCouncil(1,true),TEXT("river-port council with delegated duties"))) return false;
    Field->Issue(ETacticalCommand::PlanExample);
    OpenCouncilVoice(0);
    DeployNovelPlan();
    if(!Check(ShowOpeningOrder() && Record() && Field->Presentation().OpeningOrder.Action==EOpeningOrderAction::Inspect &&
        Field->Presentation().OpeningOrder.Tile==Save->journey.assignments[0].tile,TEXT("confirmed duty takes precedence over the authored suggestion"))) return false;
    const auto DutyOrder=Save->realtime.orders[0];
    OpenFirstCommand();
    if(!Check(Record() && Save->selected==0 && Save->realtime.orders[0].kind==DutyOrder.kind &&
        Save->realtime.orders[0].destination==DutyOrder.destination && Save->realtime.orders[0].arrival==DutyOrder.arrival,
        TEXT("inspecting the duty preserves the paid deployment and the already queued arrival instruction"))) return false;

    if(!Check(OpenCouncil(2,true),TEXT("unpaid defense council"))) return false;
    OpenCouncilVoice(2);
    DeployNovelPlan();
    if(!Check(ShowOpeningOrder() && Field->Presentation().OpeningOrder.Action==EOpeningOrderAction::Hold &&
        Field->Presentation().OpeningOrder.Tile==MissionDefinition(2).secondGoal,
        TEXT("the officer on the southern beacon can give a hold order immediately"))) return false;
    OpenFirstCommand();
    if(!Check(Record() && Save->realtime.orders[2].kind==JourneyDirective::Hold && Save->realtime.paused &&
        Save->journey.turn==1 && !ShowOpeningOrder(),TEXT("the explicit hold button queues the real guard without advancing time"))) return false;
    UE_LOG(LogTemp,Display,TEXT("GUNYU_OPENING_ORDER: PASS (council handoff, manual and realtime move, paid duties, guard, dismissal and reload)"));
    return true;
}
