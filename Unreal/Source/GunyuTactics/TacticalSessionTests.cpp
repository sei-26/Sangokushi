#include "TacticalTrial.h"
#include "TacticalState.h"
#include "TacticalSaveStorage.h"
#include "HAL/FileManager.h"
#include "Misc/FileHelper.h"
#include "Misc/Parse.h"
#include "Misc/CommandLine.h"
using namespace frontline;

bool ATacticalField::SaveSessionSmokeTest()
{
    const bool SavedMode=bRealtimeCommand,SavedCouncil=bStartWithCouncil,SavedPlayback=State->bNoPlayback;
    const int SavedMission=FirstMission,SavedPolicy=EnemyPolicy;
    auto Check=[](bool Value,const TCHAR* Detail) { if(!Value) UE_LOG(LogTemp,Error,TEXT("GUNYU_SESSION_SAVE: FAIL %s"),Detail);return Value; };
    PrepareOpeningPreview(false);State->bNoPlayback=true;bRealtimeCommand=true;
    Issue(ETacticalCommand::PlanSupplies);Issue(ETacticalCommand::TalkZhang);
    const auto Camp=State->Journey.world.cities[0];std::vector<std::uint8_t> Council,Battle,Review;
    if(!Check(ExportSession(Council) && gunyusave::Write(0,Council),TEXT("save actual council to disk"))) return false;
    if(FParse::Param(FCommandLine::Get(),TEXT("GunyuCheckpointQA")))
    {
        CheckpointJourney(true);std::vector<std::uint8_t> Auto;bool Backup=false;
        if(!Check(gunyusave::Read(3,Auto,Backup) && Auto==Council,TEXT("automatic council snapshot includes current choices"))) return false;
    }
    Issue(ETacticalCommand::Deploy);SelectOfficer(0);CommandAt(Campaign::At(8,12));AppendWaypointAt(Campaign::At(9,12));
    State->Realtime.SetArrival(State->Journey,0,JourneyArrival::Hold);State->Realtime.elapsed=4.5;
    State->Orders={{0,3,ChainTactic::Feint,-1},{2,3,ChainTactic::Breakthrough,-1}};
    Issue(ETacticalCommand::RealtimeChainSignal);
    const auto Troops=State->Journey.world.armies[0].troops;const auto Visible=State->Journey.intelligence[0].visible;
    if(!Check(ExportSession(Battle) && gunyusave::Write(1,Battle),TEXT("save battle with itinerary and signal"))) return false;
    State->Journey.Withdraw();Refresh();
    if(!Check(ExportSession(Review) && gunyusave::Write(2,Review),TEXT("save settled debrief"))) return false;
    if(!Check(ImportSession(Battle) && RealtimePaused() && State->Realtime.elapsed==4.5 && State->ChainSignal.Armed() &&
        State->Realtime.orders[0].waypoints.size()==1 && State->Realtime.orders[0].arrival==JourneyArrival::Hold &&
        State->Journey.world.armies[0].troops==Troops && State->Journey.intelligence[0].visible==Visible &&
        State->Orders.size()==2 && Presentation().bRealtime && Presentation().MarchPoints.Num()==2,
        TEXT("native HUD and signal/march/observation restore without resetting the battle"))) return false;
    if(!Check(ImportSession(Review) && CanIssue(ETacticalCommand::Retry),TEXT("loaded debrief keeps retry snapshot"))) return false;
    const auto Settled=State->Journey.world.cities[0];
    if(!Check(ImportSession(Review) && State->Journey.world.cities[0].food==Settled.food && State->Journey.world.cities[0].troops==Settled.troops,
        TEXT("reloading debrief never settles troops or food twice"))) return false;
    Issue(ETacticalCommand::Retry);
    if(!Check(State->Journey.world.cities[0].gold==Camp.gold && State->Journey.world.cities[0].food==Camp.food && State->Interlocutor==2,
        TEXT("retry after loading restores actual pre-deployment resources and conversation"))) return false;
    // Write two versions, corrupt the primary, and recover the previous valid file.
    if(!Check(gunyusave::Write(0,Battle),TEXT("backup before replacement"))) return false;
    TArray<uint8> Broken;Broken.Add(0);FFileHelper::SaveArrayToFile(Broken,*gunyusave::SlotPath(0));
    std::vector<std::uint8_t> Recovered;bool bBackup=false;
    if(!Check(gunyusave::Read(0,Recovered,bBackup) && bBackup && Recovered==Council && ImportSession(Recovered),
        TEXT("corrupt primary falls back to previous valid council"))) return false;
    auto Bad=Battle;Bad.back()^=1;const int Gold=State->Journey.world.cities[0].gold;
    if(!Check(!ImportSession(Bad) && State->Journey.world.cities[0].gold==Gold,TEXT("failed load leaves live session intact"))) return false;
    if(!Check(gunyusave::Write(0,Council) && gunyusave::Write(3,Review),TEXT("QA slots rebuilt for UI review"))) return false;
    Issue(ETacticalCommand::Deploy);State->bNoPlayback=false;
    Issue(ETacticalCommand::RealtimeMode);SelectOfficer(0);Issue(ETacticalCommand::Hold);
    if(!Check(IsPlaying() && !ExportSession(Bad) && !ImportSession(Council),TEXT("in-flight playback cannot save or load resolved future facts"))) return false;
    Issue(ETacticalCommand::Skip);
    bRealtimeCommand=SavedMode;bStartWithCouncil=SavedCouncil;FirstMission=SavedMission;EnemyPolicy=SavedPolicy;
    State->bNoPlayback=SavedPlayback;Issue(ETacticalCommand::Restart);
    UE_LOG(LogTemp,Display,TEXT("GUNYU_SESSION_SAVE: PASS (disk slots, paused exact restore, observed HUD, itinerary/signal, retry finance, no double settlement, backup recovery, playback guard)"));
    return true;
}
