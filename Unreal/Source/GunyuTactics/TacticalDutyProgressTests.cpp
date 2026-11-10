#include "TacticalTrial.h"
#include "JourneySave.hpp"
#include <memory>
using namespace frontline;

bool ATacticalController::PrepareDutyProgressPreview(int Variant)
{
    if(Variant<0 || Variant>5) return false;
    CloseEpisode();Field->Issue(ETacticalCommand::Skip);
    Field->bStartWithCouncil=true;Field->bLegacyJourney=false;Field->bRealtimeCommand=true;
    Field->FirstMission=Variant==4 ? 2:1;Field->EnemyPolicy=0;Field->Issue(ETacticalCommand::Restart);
    for(int I=0;I<3;++I) Field->Action(EKeys::Enter);
    Field->Issue(ETacticalCommand::PlanExample);Field->Issue(ETacticalCommand::Deploy);
    Field->SelectOfficer(0);
    if(SelectedDutyTile()<0 || Field->Presentation().DutyProgress.bCanResume) return false;
    if(Variant==1 || Variant==2 || Variant==5)
    {
        auto Save=std::make_unique<JourneySaveData>();std::vector<std::uint8_t> Bytes;
        if(!Field->ExportSession(Bytes) || !DecodeJourneySave(Bytes,*Save)) return false;
        auto& J=Save->journey;
        const auto Path=Save->realtime.Route(J,0);
        if(Path.size()<2) return false;
        auto Position=[&](int Id,int At) { J.world.armies[Id].tile=J.world.armies[Id].target=At;J.world.armies[Id].path.clear(); };
        if(Variant==5)
        {
            // The two routes merge before Liu's assigned post. Movement and
            // conflict resolution still run through the real command clock.
            Position(0,Campaign::At(6,10));Position(2,Campaign::At(6,9));
            if(!Save->realtime.Queue(J,2,JourneyDirective::March,Campaign::At(7,10))) return false;
        }
        else
        {
            const int Id=Variant==1 ? 2:0;
            const int At=Variant==1 ? J.assignments[0].tile:Path[Path.size()-2];
            Position(Id,At);
        }
        ++J.world.revision;J.RefreshIntelligence();
        if(!EncodeJourneySave(*Save,Bytes) || !Field->ImportSession(Bytes)) return false;
        if(Variant==2)
        {
            Field->Issue(ETacticalCommand::RealtimeStep);
            if(!Field->IsPlaying() || !Field->Presentation().DutyProgress.Caption.Contains(TEXT("行軍"))) return false;
            Field->Issue(ETacticalCommand::Skip);
            if(Field->Presentation().DutyProgress.Caption!=TEXT("到着・固守待ち")) return false;
            Field->Issue(ETacticalCommand::RealtimeStep);Field->Issue(ETacticalCommand::Skip);
        }
    }
    if(Variant==3) Field->Issue(ETacticalCommand::RealtimeStop);
    FocusSelectedDuty();
    const auto& P=Field->Presentation().DutyProgress;
    if(Variant==1) return P.InspectTile==P.Tile && !P.bTraffic && P.Caption.Contains(TEXT("張飛"));
    if(Variant==2) return P.Caption==TEXT("到着・固守指示あり") && P.InspectTile<0 && !P.bCanResume;
    if(Variant==3) return P.Caption==TEXT("担当への指示なし") && P.bCanResume;
    if(Variant==4) return GoalDutyStatus(0)==P.Caption && !P.Caption.IsEmpty();
    if(Variant==5)
    {
        FocusDutyObstacle();
        return P.bTraffic && P.InspectTile==Campaign::At(7,10) && P.InspectTile!=P.Tile && P.Caption.Contains(TEXT("張飛"));
    }
    return P.Caption.Contains(TEXT("担当へ行軍")) && P.InspectTile<0;
}

bool ATacticalController::DutyProgressSmokeTest()
{
    auto Check=[](bool Valid,const TCHAR* Why)
    { if(!Valid) UE_LOG(LogTemp,Error,TEXT("GUNYU_DUTY_PROGRESS: FAIL %s"),Why);return Valid; };
    CloseEpisode();Field->Issue(ETacticalCommand::Skip);
    const bool Council=Field->bStartWithCouncil,Legacy=Field->bLegacyJourney;
    const int Mission=Field->FirstMission,Policy=Field->EnemyPolicy,Preset=CameraPreset;
    const auto OldFocus=Focus;const float OldDistance=Distance;
    std::vector<std::uint8_t> Original,Before,After;
    if(!Check(Field->ExportSession(Original),TEXT("save the caller session"))) return false;
    for(int Variant:{0,1,2,3,4})
        if(!Check(PrepareDutyProgressPreview(Variant),*FString::Printf(TEXT("actual deployment and duty state variant %d"),Variant))) return false;
    if(!Check(PrepareDutyProgressPreview(1) && Field->ExportSession(Before),TEXT("occupied destination and observed obstacle"))) return false;
    const int Blocker=Field->Presentation().DutyProgress.InspectTile;
    FocusDutyObstacle();
    if(!Check(Focus.Equals(Field->SurfaceLocation(Blocker)) && Field->ExportSession(After) && Before==After &&
       !Field->CanIssue(ETacticalCommand::ResumeDuty),TEXT("looking at the obstruction is read-only while a march is active"))) return false;

    if(!Check(PrepareDutyProgressPreview(3) && Field->ExportSession(Before),TEXT("explicit cancellation leaves a resumable duty"))) return false;
    auto A=std::make_unique<JourneySaveData>(),B=std::make_unique<JourneySaveData>();
    if(!Check(DecodeJourneySave(Before,*A) && Field->CanIssue(ETacticalCommand::ResumeDuty),TEXT("stable stopped assignment"))) return false;
    Field->Issue(ETacticalCommand::ResumeDuty);
    if(!Check(Field->ExportSession(After) && DecodeJourneySave(After,*B) &&
        B->realtime.orders[0].kind==JourneyDirective::March && B->realtime.orders[0].destination==A->journey.assignments[0].tile &&
        B->realtime.orders[0].arrival==JourneyArrival::Hold && B->realtime.paused &&
        B->journey.turn==A->journey.turn && B->journey.chain.spent==A->journey.chain.spent &&
        B->journey.world.cities[0].food==A->journey.world.cities[0].food &&
        SameJourneyOrder(A->realtime.orders[1],B->realtime.orders[1]) && SameJourneyOrder(A->realtime.orders[2],B->realtime.orders[2]) &&
        !Field->CanIssue(ETacticalCommand::ResumeDuty),
        TEXT("restoring the original route and arrival action queues one order without spending a turn, action or another officer's order"))) return false;

    if(!Check(PrepareDutyProgressPreview(5) && Field->ExportSession(Before) && DecodeJourneySave(Before,*A),
        TEXT("two legal friendly routes point at the same next tile"))) return false;
    const int Conflict=Field->Presentation().DutyProgress.InspectTile;
    Field->Issue(ETacticalCommand::RealtimeStep);Field->Issue(ETacticalCommand::Skip);
    if(!Check(Field->ExportSession(After) && DecodeJourneySave(After,*B) &&
        B->journey.world.armies[0].tile==A->journey.world.armies[0].tile && B->journey.world.armies[2].tile==A->journey.world.armies[2].tile,
        TEXT("the predicted conflict matches the real simultaneous wait"))) return false;
    Field->SelectOfficer(2);Field->Issue(ETacticalCommand::RealtimeStop);Field->SelectOfficer(0);
    if(!Check(!Field->Presentation().DutyProgress.bTraffic,TEXT("stopping the other route clears the overlap notice"))) return false;
    Field->Issue(ETacticalCommand::RealtimeStep);Field->Issue(ETacticalCommand::Skip);
    if(!Check(Field->ExportSession(After) && DecodeJourneySave(After,*B) && B->journey.world.armies[0].tile==Conflict,
        TEXT("the original duty order progresses after the player clears the overlap"))) return false;
    Field->bStartWithCouncil=Council;Field->bLegacyJourney=Legacy;Field->FirstMission=Mission;Field->EnemyPolicy=Policy;
    if(!Check(Field->ImportSession(Original),TEXT("restore original session"))) return false;
    Focus=OldFocus;Distance=OldDistance;CameraPreset=Preset;
    UE_LOG(LogTemp,Display,TEXT("GUNYU_DUTY_PROGRESS: PASS (real march, observed blocker, arrival, reissued order, same-tile wait and release, objective plate and save equality)"));
    return true;
}
