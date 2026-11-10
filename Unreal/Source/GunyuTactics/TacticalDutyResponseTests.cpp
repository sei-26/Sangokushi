#include "TacticalTrial.h"
#include "TacticalMissionResponse.h"
#include "JourneySave.hpp"
#include <memory>
using namespace frontline;
namespace
{
    bool IsDutyReport(const ATacticalField* Field)
    { return Field->ReadMissionResponse().Caption.StartsWith(TEXT("軍議の担当 / ")); }
}

bool ATacticalController::PrepareDutyResponseCase(int Variant)
{
    if(Variant==0) return PrepareDutyProgressPreview(0);
    if(Variant==2 && !PrepareDutyProgressPreview(4)) return false;
    if(Variant==1)
    {
        CloseEpisode();Field->Issue(ETacticalCommand::Skip);
        Field->bStartWithCouncil=true;Field->bLegacyJourney=false;Field->bRealtimeCommand=false;
        Field->FirstMission=1;Field->EnemyPolicy=0;Field->Issue(ETacticalCommand::Restart);
        for(int I=0;I<3;++I) Field->Action(EKeys::Enter);
        Field->Issue(ETacticalCommand::PlanExample);Field->Issue(ETacticalCommand::PlanSupplies);Field->Issue(ETacticalCommand::Deploy);
        Field->SelectOfficer(1);
    }
    auto Save=std::make_unique<JourneySaveData>();std::vector<std::uint8_t> Bytes;
    if(!Field->ExportSession(Bytes) || !DecodeJourneySave(Bytes,*Save)) return false;
    auto& J=Save->journey;
    // Set the test formation after real payment. Only actual actions may record a completed duty.
    for(int Id:Variant==1 ? TArray<int>{1}:TArray<int>{0,2})
    {
        if(J.assignments[Id].completed || J.assignments[Id].duty==JourneyDuty::None) return false;
        J.world.armies[Id].tile=J.world.armies[Id].target=J.assignments[Id].tile;
        J.world.armies[Id].path.clear();
    }
    ++J.world.revision;J.RefreshIntelligence();
    if(!EncodeJourneySave(*Save,Bytes) || !Field->ImportSession(Bytes)) return false;
    FocusSelectedDuty();return true;
}

bool ATacticalController::PrepareDutyResponseCapture(int Variant)
{
    if(Variant<0 || Variant>3 || !PrepareDutyResponseCase(Variant==3 ? 0:Variant)) return false;
    if(Variant==0)
    { Field->Issue(ETacticalCommand::RealtimeStep);Field->Issue(ETacticalCommand::Skip); }
    Field->Issue(Variant==1 ? ETacticalCommand::PrepareAmbush:ETacticalCommand::RealtimeStep);
    if(!Field->IsPlaying()) return false;
    Field->AdvancePresentation(20);
    const auto Reply=Field->ReadMissionResponse();
    if(Variant==2) return Reply.Caption.Contains(TEXT("両翼を維持"));
    if(Variant==3) return !IsDutyReport(Field) && Field->Presentation().DutyProgress.Caption==TEXT("到着・固守待ち");
    return IsDutyReport(Field) && Reply.Actor==(Variant==0 ? 0:1);
}

bool ATacticalController::DutyResponseSmokeTest()
{
    auto Check=[](bool Ok,const TCHAR* Reason)
    { if(!Ok) UE_LOG(LogTemp,Error,TEXT("GUNYU_DUTY_RESPONSE: FAIL %s"),Reason);return Ok; };
    CloseEpisode();Field->Issue(ETacticalCommand::Skip);
    const bool Council=Field->bStartWithCouncil,Legacy=Field->bLegacyJourney;
    const int Mission=Field->FirstMission,Policy=Field->EnemyPolicy,Preset=CameraPreset;
    const FVector OldFocus=Focus;const float OldDistance=Distance;
    std::vector<std::uint8_t> Original,Arrived,Natural,After;
    auto Save=std::make_unique<JourneySaveData>();
    if(!Check(Field->ExportSession(Original) && PrepareDutyResponseCase(0),TEXT("ordinary deployment starts the original assigned march"))) return false;
    Field->Issue(ETacticalCommand::RealtimeStep);Field->AdvancePresentation(20);
    if(!Check(!IsDutyReport(Field) && Field->Presentation().DutyProgress.Caption==TEXT("到着・固守待ち") &&
        Field->ExportSession(Arrived) && DecodeJourneySave(Arrived,*Save) && !Save->journey.assignments[0].completed && Save->journey.assignments[1].completed,
        TEXT("mere arrival and already prepared opening ambush produce no duty report"))) return false;
    Field->Issue(ETacticalCommand::RealtimeStep);
    if(!Check(Field->IsPlaying() && !IsDutyReport(Field),TEXT("the actual guard result remains private during replay"))) return false;
    Field->AdvancePresentation(20);
    if(!Check(IsDutyReport(Field) && Field->ReadMissionResponse().Actor==0 && Field->ReadMissionResponse().Caption.Contains(TEXT("固守")) &&
        Field->ExportSession(Natural),TEXT("Liu reports only after the first hold is performed and shown"))) return false;
    if(!Check(Field->ImportSession(Arrived),TEXT("reload the state before the first hold"))) return false;
    Field->Issue(ETacticalCommand::RealtimeStep);Field->Issue(ETacticalCommand::Skip);
    if(!Check(!IsDutyReport(Field) && Field->ExportSession(After) && After==Natural,TEXT("skipping has the same save and suppresses the short report"))) return false;
    if(!Check(Field->ImportSession(Natural) && !IsDutyReport(Field),TEXT("reload preserves completion without replaying the report"))) return false;
    Field->Issue(ETacticalCommand::RealtimeStep);Field->AdvancePresentation(20);
    if(!Check(!IsDutyReport(Field),TEXT("standing hold does not repeat a completed report"))) return false;
    if(!Check(PrepareDutyResponseCapture(1) && Field->ReadMissionResponse().Caption.Contains(TEXT("伏兵")),
        TEXT("manual ambush at the assigned forest reports Guan after the real action"))) return false;
    if(!Check(PrepareDutyResponseCapture(2) && Field->ExportSession(After) && DecodeJourneySave(After,*Save) &&
        Save->journey.assignments[0].completed && Save->journey.assignments[2].completed,
        TEXT("actual first holds and first beacon progress share a cycle, with objective progress taking priority"))) return false;

    Field->bStartWithCouncil=Council;Field->bLegacyJourney=Legacy;Field->FirstMission=Mission;Field->EnemyPolicy=Policy;
    if(!Check(Field->ImportSession(Original),TEXT("restore caller session"))) return false;
    Focus=OldFocus;Distance=OldDistance;CameraPreset=Preset;
    UE_LOG(LogTemp,Display,TEXT("GUNYU_DUTY_RESPONSE: PASS (actual hold and manual ambush, mere arrival, no preview or repeat, skip/load equality, objective priority)"));
    return true;
}
