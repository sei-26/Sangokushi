#include "TacticalTrial.h"
#include "TacticalState.h"
#include "TacticalActivity.h"
#include "JourneyArchive.hpp"

bool ATacticalField::ActivityReadSmokeTest()
{
    auto Check=[](bool V,const TCHAR* Why) { if(!V) UE_LOG(LogTemp,Error,TEXT("GUNYU_ACTIVITY: FAIL %s"),Why);return V; };
    TArray<FVisualUnit> U;U.SetNum(6);
    U[0].Id=0;U[0].Troops=1400;U[0].Name=TEXT("actor");
    U[1].Id=1;U[1].Troops=1400;U[1].bConcealed=true;U[1].Position=FVector(0,200,0);
    U[3].Id=3;U[3].Troops=1800;U[3].bEnemy=true;U[3].Position=FVector(200,0,0);
    FBattleBeat B;B.Actor=0;B.Target=3;B.Kind=EBattleCue::Strike;B.Before=B.After=U;B.After[3].Troops=1200;
    auto R=gunyuactivity::Read(0,U,&B,.25f);
    if(!Check(R.bActor && R.bActive && R.Partner==3 && R.Caption.Contains(TEXT("構え")),TEXT("acting officer and observed recipient are paired"))) return false;
    if(!Check(!gunyuactivity::Read(1,U,&B,.25f).bActive && gunyuactivity::Read(1,U,&B,.25f).Caption==TEXT("潜伏中"),TEXT("uninvolved officers retain their own state"))) return false;
    if(!Check(!gunyuactivity::Read(3,U,&B,.47f).bImpact && !gunyuactivity::Read(3,U,&B,.47f).Caption.Contains(TEXT("被弾")),TEXT("windup does not reveal later casualties"))) return false;
    U[3].Troops=1200;U[3].Loss=600;U[3].bDisordered=true;
    if(!Check(gunyuactivity::Read(3,U,&B,.55f).bImpact && gunyuactivity::Read(3,U,&B,.55f).Caption==TEXT("被弾・混乱"),TEXT("published casualty/state produces the victim role"))) return false;
    auto Marks=gunyuactivity::Markers(U,&B,.55f);
    if(!Check(Marks.Num()>2 && Marks.Num()<16,TEXT("actual action has a small bounded set of directional marks"))) return false;
    for(const auto& Mark:Marks) for(auto P:Mark.Points)
        if(!Check(!P.ContainsNaN() && P.Size()<1000,TEXT("markers stay finite and near displayed participants"))) return false;
    B.Kind=EBattleCue::Stop;
    if(!Check(gunyuactivity::Read(0,U,&B,.55f).bStopped && gunyuactivity::Markers(U,&B,.55f).IsEmpty(),TEXT("stopped chain has a stop label and no attack arrows"))) return false;
    B.Kind=EBattleCue::Rally;
    if(!Check(!gunyuactivity::Read(3,U,&B,.2f).bActive && gunyuactivity::Read(0,U,&B,.2f).Partner==-1,TEXT("support never marks an enemy as an attack recipient"))) return false;
    B.Kind=EBattleCue::Strike;B.Target=4;B.After[4].Id=4;B.After[4].Position=FVector(9000,9000,0);
    if(!Check(!gunyuactivity::Read(4,U,&B,.3f).bActive && !gunyuactivity::Read(0,U,&B,.3f).bActive && gunyuactivity::Markers(U,&B,.3f).IsEmpty(),TEXT("future hidden reserve has no role, name, progress bar or mark"))) return false;
    const int SavedMission=FirstMission,SavedPolicy=EnemyPolicy;
    const bool SavedCouncil=bStartWithCouncil,SavedRealtime=bRealtimeCommand,SavedNoPlayback=State->bNoPlayback;
    const float SavedSpeed=PlaybackSpeed;
    FirstMission=0;EnemyPolicy=0;bStartWithCouncil=false;bRealtimeCommand=false;State->bNoPlayback=false;PlaybackSpeed=1;
    Issue(ETacticalCommand::Restart);Issue(ETacticalCommand::Sample);Issue(ETacticalCommand::Execute);AdvancePresentation(.9f);
    if(!Check(ReadUnitActivity(0).Caption==TEXT("誘導・後退") && ReadUnitActivity(1).Caption==TEXT("潜伏中"),TEXT("real lure distinguishes its actor from the concealed next officer"))) return false;
    AdvancePresentation(2.1f);
    if(!Check(CurrentBeat()->Kind==EBattleCue::Ambush && ReadUnitActivity(1).Caption.Contains(TEXT("構え")) && State->Units[3].Troops==1800,TEXT("next step starts without borrowing the later damage"))) return false;
    AdvancePresentation(1.5f);
    if(!Check(State->Units[3].Loss==413 && ReadUnitActivity(3).Caption==TEXT("被弾・混乱") && State->HUD.Officers[1].Status.Contains(TEXT("伏兵")),TEXT("map and roster describe the current ambush, not the later breakthrough"))) return false;
    // The player save API intentionally rejects an in-flight replay. Snapshot
    // simulation copies in memory here without changing that production guard.
    auto Snapshot=[&]()
    {
        std::vector<std::uint8_t> Bytes;
        auto Journey=std::make_unique<frontline::StrategistJourney>(State->Journey);
        auto Realtime=State->Realtime;auto Signal=State->ChainSignal;auto Orders=State->Orders;
        frontline::JourneyArchive Write{false,Bytes};Write(*Journey,Realtime,Signal,Orders);return Bytes;
    };
    const auto Before=Snapshot();
    for(int N=0;N<12;++N) { for(int I=0;I<6;++I) ReadUnitActivity(I);gunyuactivity::Markers(VisualUnits(),CurrentBeat(),BeatProgress()); }
    if(!Check(!Before.empty() && Before==Snapshot(),TEXT("activity reads consume no command, resources, time or damage"))) return false;
    Issue(ETacticalCommand::Skip);Issue(ETacticalCommand::EndTurn);
    if(!Check(CurrentBeat() && CurrentBeat()->Kind==EBattleCue::Enemy,TEXT("public command produces an enemy beat"))) return false;
    State->Beats[State->BeatIndex].Reason=TEXT("future-result-9137");AdvancePresentation(.1f);
    if(!Check(!CueReason().Contains(TEXT("9137")),TEXT("generic enemy report stays hidden before impact too"))) return false;
    AdvancePresentation(1.8f);
    if(!Check(CueReason().Contains(TEXT("9137")),TEXT("resolved generic report appears at its published point"))) return false;
    Issue(ETacticalCommand::Skip);
    FirstMission=SavedMission;EnemyPolicy=SavedPolicy;bStartWithCouncil=SavedCouncil;bRealtimeCommand=SavedRealtime;State->bNoPlayback=SavedNoPlayback;PlaybackSpeed=SavedSpeed;Issue(ETacticalCommand::Restart);
    UE_LOG(LogTemp,Display,TEXT("GUNYU_ACTIVITY: PASS (current actor/recipient, public damage, inactive allies, support, stop, hidden reserve, bounded marks, immutable reads, generic report timing)"));return true;
}
