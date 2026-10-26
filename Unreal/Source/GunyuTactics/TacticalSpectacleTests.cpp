#include "TacticalTrial.h"
#include "TacticalState.h"
#include "TacticalSpectacle.h"
#include "TacticalTacticEffects.h"
#include "TacticalCinematicStyle.h"
bool ATacticalField::SpectacleSmokeTest()
{
    auto Check=[](bool V,const TCHAR* Why) { if(!V) UE_LOG(LogTemp,Error,TEXT("GUNYU_SPECTACLE: FAIL %s"),Why);return V; };
    TArray<FVisualUnit> U;U.SetNum(6);
    for(int I=0;I<4;++I) { U[I].Id=I;U[I].Troops=1400; }
    U[1].Position=FVector(300,0,0);U[2].Position=FVector(0,300,0);U[3].Position=FVector(9000,9000,0);U[3].bEnemy=true;
    FBattleBeat B;B.Before=B.After=U;B.Actor=0;B.Target=3;B.Kind=EBattleCue::Rally;B.After[1].bInspired=true;
    const auto Support=gunyuspectacle::Read(&B,U,.55f);
    if(!Check(gunyucinema::ResultUnit(B,U)==B.Actor,TEXT("support result stays on its friendly actor instead of the selected enemy"))) return false;
    if(!Check(Support.bValid && Support.Center.Equals(FVector(100,100,0),.01) && Support.Pulse>0 && Support.Shake==0,TEXT("support frames known allies, excludes enemy focus and uses a calm impact"))) return false;
    U[3].Position=FVector(200,0,0);B.Before=B.After=U;B.After[3].Troops-=500;B.Kind=EBattleCue::AzureDragon;
    const auto Dragon=gunyuspectacle::Read(&B,U,.55f);
    B.Kind=EBattleCue::SerpentRoar;const auto Roar=gunyuspectacle::Read(&B,U,.55f);
    if(!Check(Dragon.Tilt<Roar.Tilt && Dragon.CloseDistance<Roar.CloseDistance && Dragon.Sweep!=Roar.Sweep,TEXT("blade and roar have distinct shot language"))) return false;
    if(!Check(gunyuspectacle::Read(&B,U,.47f).Pulse==0 && Roar.Pulse>0 && FMath::Abs(Roar.Shake)<=8,TEXT("impact accent is bounded and starts only at publication"))) return false;
    for(auto Kind:{EBattleCue::AzureDragon,EBattleCue::SerpentRoar})
    {
        B.Kind=Kind;int Quads=0;bool Finite=true;
        auto Draw=[&](FVector A,FVector C,FVector D,FVector E,FLinearColor Color)
        { ++Quads;Finite=Finite && !A.ContainsNaN() && !C.ContainsNaN() && !D.ContainsNaN() && !E.ContainsNaN() && A.Size()<1200 && C.Size()<1200 && FMath::IsFinite(Color.A); };
        gunyufx::Signature(B,U,.55f,Draw);
        if(!Check(Quads>60 && Quads<200 && Finite,TEXT("signature ribbons are bounded, finite local geometry"))) return false;
        Quads=0;gunyufx::Signature(B,U,.47f,Draw);
        if(!Check(Quads==0,TEXT("no signature impact geometry before the hit"))) return false;
        B.Target=4;Quads=0;gunyufx::Signature(B,U,.55f,Draw);
        if(!Check(Quads==0 && !gunyuspectacle::Read(&B,U,.55f).bValid,TEXT("hidden reserve cannot receive geometry, a shot or a lens accent"))) return false;
        B.Target=3;
    }
    B.Kind=EBattleCue::Stop;
    if(!Check(gunyucinema::ResultUnit(B,U)==B.Actor,TEXT("failure result belongs to the unsuccessful officer"))) return false;
    if(!Check(gunyuspectacle::Read(&B,U,.55f).Pulse==0 && gunyuspectacle::Read(&B,U,.55f).Shake==0,TEXT("stopped chain has no success flash or shake"))) return false;
    const int SavedMission=FirstMission,SavedPolicy=EnemyPolicy;
    const bool SavedCouncil=bStartWithCouncil,SavedRealtime=bRealtimeCommand,SavedNoPlayback=State->bNoPlayback;
    const float SavedSpeed=PlaybackSpeed;
    FirstMission=0;EnemyPolicy=0;bStartWithCouncil=false;bRealtimeCommand=false;State->bNoPlayback=false;PlaybackSpeed=1;
    Issue(ETacticalCommand::Restart);Issue(ETacticalCommand::Sample);Issue(ETacticalCommand::Execute);AdvancePresentation(2.75f);
    auto C=ReadTacticCinema();
    if(!Check(C.Relay>0 && C.NextActor==1 && C.NextOfficer==State->Units[1].Name,TEXT("current successful step hands off to the already queued officer"))) return false;
    const auto SavedKind=State->Beats[1].Kind;State->Beats[1].Kind=EBattleCue::Stop;
    const auto UnknownOutcome=ReadTacticCinema();State->Beats[1].Kind=SavedKind;
    if(!Check(UnknownOutcome.NextActor==C.NextActor && UnknownOutcome.NextOfficer==C.NextOfficer && UnknownOutcome.Relay==C.Relay,TEXT("handoff never exposes the next step's failure"))) return false;
    const int Resolved=State->Journey.world.armies[3].troops,Turn=State->Journey.turn;
    for(int N=0;N<12;++N) { ReadTacticCinema();gunyuspectacle::Read(CurrentBeat(),VisualUnits(),BeatProgress()); }
    if(!Check(State->Journey.world.armies[3].troops==Resolved && State->Journey.turn==Turn,TEXT("spectacle does not apply combat results"))) return false;
    AdvancePresentation(5.95f);C=ReadTacticCinema();
    if(!Check(CurrentBeat()->ChainStep==3 && C.NextActor==-1 && C.Relay==0,TEXT("last step invents no extra relay"))) return false;
    Issue(ETacticalCommand::Skip);
    Issue(ETacticalCommand::Restart);Issue(ETacticalCommand::Sample);EditOrder(2,-1);Issue(ETacticalCommand::Execute);AdvancePresentation(5.2f);C=ReadTacticCinema();
    if(!Check(CurrentBeat()->Kind==EBattleCue::Stop && C.Relay==0,TEXT("failed current step cannot relay onward"))) return false;
    Issue(ETacticalCommand::Skip);
    FirstMission=SavedMission;EnemyPolicy=SavedPolicy;bStartWithCouncil=SavedCouncil;bRealtimeCommand=SavedRealtime;State->bNoPlayback=SavedNoPlayback;PlaybackSpeed=SavedSpeed;Issue(ETacticalCommand::Restart);
    UE_LOG(LogTemp,Display,TEXT("GUNYU_SPECTACLE: PASS (distinct shots, allied support, published impact, bounded ribbons, hidden target, stopped chain, queued-only relay, immutable reads)"));return true;
}
