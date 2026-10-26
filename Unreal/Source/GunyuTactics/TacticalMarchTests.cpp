#include "TacticalTrial.h"
#include "TacticalState.h"
using namespace frontline;

void ATacticalField::PrepareMarchPreview()
{
    bRealtimeCommand=true;bStartWithCouncil=false;FirstMission=1;EnemyPolicy=1;Issue(ETacticalCommand::Restart);
    SelectOfficer(1);CommandAt(Campaign::At(6,9));AppendWaypointAt(Campaign::At(6,7));AppendWaypointAt(Campaign::At(8,7));
    Issue(ETacticalCommand::RealtimeArrival);Issue(ETacticalCommand::RealtimeArrival);
}
bool ATacticalField::MarchPlanSmokeTest()
{
    const bool SavedMode=bRealtimeCommand,SavedCouncil=bStartWithCouncil,SavedPlayback=State->bNoPlayback;
    const int SavedMission=FirstMission,SavedPolicy=EnemyPolicy;
    auto Check=[](bool Value,const TCHAR* Reason)
    { if (!Value) UE_LOG(LogTemp,Error,TEXT("GUNYU_MARCH_PLAN: FAIL: %s"),Reason);return Value; };
    State->bNoPlayback=true;PrepareMarchPreview();
    const int Revision=State->Journey.world.revision;
    if (!Check(Presentation().MarchPoints.Num()==3 && State->Realtime.orders[1].arrival==JourneyArrival::Ambush && RealtimePaused(),
        TEXT("public append and arrival controls build the three-point forest plan"))) return false;
    AdvanceRealtime(20);
    if (!Check(State->Journey.world.revision==Revision,TEXT("planning consumes no battle time or actions"))) return false;
    Issue(ETacticalCommand::RealtimeUndoWaypoint);
    if (!Check(Presentation().MarchPoints.Num()==2 && State->Realtime.orders[1].arrival==JourneyArrival::Wait,TEXT("undo changes only the plan and resets the arrival instruction"))) return false;

    FirstMission=0;EnemyPolicy=0;Issue(ETacticalCommand::Restart);Issue(ETacticalCommand::Sample);
    SelectOfficer(0);CommandAt(Campaign::At(9,11));AppendWaypointAt(Campaign::At(9,10));Issue(ETacticalCommand::RealtimeChainSignal);
    Issue(ETacticalCommand::RealtimePause);AdvanceRealtime(.1f);
    if (!Check(State->ChainSignal.Armed() && State->SignalRead.completed==3 && State->Journey.lastChain.completed==0 && !Presentation().bChainSignalReady,
        TEXT("a feasible chain waits for the commanded march rather than firing at its starting cell"))) return false;
    Issue(ETacticalCommand::RealtimePause);Issue(ETacticalCommand::RealtimeStep);
    if (!Check(State->Journey.world.armies[0].tile==Campaign::At(9,11) && Presentation().MarchPoints.Num()==1,TEXT("first checkpoint advances without teleporting to the final destination"))) return false;
    Issue(ETacticalCommand::RealtimeStep);
    if (!Check(State->Journey.world.armies[0].tile==Campaign::At(9,10) && Presentation().MarchPoints.IsEmpty(),TEXT("second checkpoint completes the march"))) return false;
    Issue(ETacticalCommand::RealtimePause);AdvanceRealtime(.1f);
    if (!Check(State->Journey.lastChain.completed==3 && !State->ChainSignal.Armed() && RealtimePaused(),TEXT("the same reserved chain releases after the full march"))) return false;

    FirstMission=1;Issue(ETacticalCommand::Restart);State->bNoPlayback=false;
    SelectOfficer(0);CommandAt(Campaign::At(9,12));AppendWaypointAt(Campaign::At(8,12));
    const auto DisplayedRoutes=State->RealtimeRoutes;
    Issue(ETacticalCommand::RealtimeStep);AdvancePresentation(.5f);
    const auto Plan=State->Realtime.orders[0];
    if (!Check(IsPlaying() && State->RealtimeRoutes==DisplayedRoutes && Presentation().MarchPoints.Num()==2 && !CanIssue(ETacticalCommand::RealtimeArrival) && !CanIssue(ETacticalCommand::RealtimeUndoWaypoint),
        TEXT("plan editing is unavailable during unresolved presentation"))) return false;
    AppendWaypointAt(Campaign::At(7,12));
    if (!Check(SameJourneyOrder(Plan,State->Realtime.orders[0]),TEXT("mid-playback append cannot change the hidden resolved route"))) return false;
    AdvancePresentation(10);Issue(ETacticalCommand::RealtimeMode);
    if (!Check(Presentation().MarchPoints.IsEmpty(),TEXT("manual mode clears march UI and orders"))) return false;
    bRealtimeCommand=SavedMode;bStartWithCouncil=SavedCouncil;FirstMission=SavedMission;EnemyPolicy=SavedPolicy;
    State->bNoPlayback=SavedPlayback;Issue(ETacticalCommand::Restart);
    UE_LOG(LogTemp,Display,TEXT("GUNYU_MARCH_PLAN: PASS (public waypoint edits, arrival, full-march chain gate, real movement, presentation lock, reset)"));
    return true;
}
