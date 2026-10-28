#include "TacticalTrial.h"
#include "JourneySave.hpp"
#include <memory>
bool ATacticalController::PlanningDutySmokeTest()
{
    using namespace frontline;
    auto Check=[](bool V,const TCHAR* Why) { if(!V) UE_LOG(LogTemp,Error,TEXT("GUNYU_PLANNING_DUTY: FAIL %s"),Why);return V; };
    CloseEpisode();auto Save=std::make_unique<JourneySaveData>();Save->journey.Reset();Save->journey.Meet();Save->journey.mission=2;Save->preparation=1;Save->realtimeMode=true;
    std::vector<std::uint8_t> Bytes;
    if(!Check(EncodeJourneySave(*Save,Bytes) && Field->ImportSession(Bytes),TEXT("council can load"))) return false;
    OpenEpisode();OpenNovelPlan();TogglePlanningMap();bPlanningDuties=true;
    PlanningOfficer=0;SelectPlanningTile(Campaign::At(7,6));SetPlanningDuty(1);
    PlanningOfficer=1;SelectPlanningTile(Campaign::At(7,5));SetPlanningDuty(2);
    PlanningOfficer=2;SelectPlanningTile(Campaign::At(8,9));SetPlanningDuty(1);
    if(!Check(Field->ExportSession(Bytes) && DecodeJourneySave(Bytes,*Save),TEXT("assigned council exports"))) return false;
    const auto Camp=Save->journey.world.cities[0];
    if(!Check(Save->journey.assignments[0].tile==Campaign::At(7,6) && Save->journey.assignments[1].duty==JourneyDuty::Ambush && !Save->journey.assignments[2].completed && Save->journey.officerTrust[0]==0 && Camp.gold==900,
        TEXT("map actions set actual duties without paying or inventing performance"))) return false;
    if(!Check(!Field->AssignPlanningDuty(2,Campaign::At(7,6),1) && !Field->AssignPlanningDuty(2,Campaign::At(9,7),1),TEXT("duplicate or river targets are rejected"))) return false;
    HandleEpisodeKey(EKeys::D);
    if(!Check(!bEpisodeOpen && Field->RealtimePaused() && Field->ExportSession(Bytes) && DecodeJourneySave(Bytes,*Save),TEXT("explicit deployment starts paused"))) return false;
    if(!Check(Save->realtime.orders[0].destination==Campaign::At(7,6) && Save->realtime.orders[1].arrival==JourneyArrival::Ambush && Save->realtime.orders[2].arrival==JourneyArrival::Hold && !Save->journey.assignments[2].completed,
        TEXT("three real march/arrival orders survive presentation refresh"))) return false;
    if(!Check(Save->beforeBattle && Save->beforeBattle->assignments[2].tile==Campaign::At(8,9) && Save->beforeBattle->world.cities[0].food==Camp.food,TEXT("retry snapshot preserves duties and unpaid camp"))) return false;
    Field->SelectOfficer(0);Field->Issue(ETacticalCommand::RealtimeStop);
    Field->ExportSession(Bytes);DecodeJourneySave(Bytes,*Save);
    if(!Check(Save->realtime.orders[0].kind==JourneyDirective::None && !Save->journey.assignments[0].completed && Save->journey.turn==1,TEXT("cancel keeps honest pending status and consumes no command cycle"))) return false;
    if(!Check(Field->ImportSession(Bytes) && Field->ExportSession(Bytes) && DecodeJourneySave(Bytes,*Save) && Save->realtime.orders[0].kind==JourneyDirective::None,TEXT("load cannot reissue an overridden council order"))) return false;
    Field->Issue(ETacticalCommand::Withdraw);Field->Issue(ETacticalCommand::Withdraw);Field->Issue(ETacticalCommand::Skip);
    Field->Issue(ETacticalCommand::Retry);Field->ExportSession(Bytes);DecodeJourneySave(Bytes,*Save);
    if(!Check(Save->journey.stage==JourneyStage::Preparation && Save->journey.assignments[1].duty==JourneyDuty::Ambush && !Save->journey.assignments[1].completed && Save->journey.world.cities[0].gold==Camp.gold,TEXT("retry restores the council choices and full preparation cost"))) return false;
    UE_LOG(LogTemp,Display,TEXT("GUNYU_PLANNING_DUTY: PASS (actual controller assignments, real deployment orders, paused start, cancellation, save/load and retry finance)"));return true;
}
