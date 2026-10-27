#include "TacticalTrial.h"
#include "TacticalState.h"
#include "TacticalTacticEffects.h"
bool ATacticalField::TacticCinemaSmokeTest()
{
    const int SavedMission=FirstMission,SavedPolicy=EnemyPolicy;
    const bool SavedCouncil=bStartWithCouncil,SavedPlayback=State->bNoPlayback,SavedRealtime=bRealtimeCommand;
    const float SavedVolume=EffectsVolume,SavedSpeed=PlaybackSpeed;
    auto Check=[](bool V,const TCHAR* Why) { if(!V) UE_LOG(LogTemp,Error,TEXT("GUNYU_TACTIC_CINEMA: FAIL %s"),Why);return V; };
    FirstMission=0;EnemyPolicy=0;bStartWithCouncil=false;State->bNoPlayback=false;bRealtimeCommand=false;EffectsVolume=0;PlaybackSpeed=1;
    Issue(ETacticalCommand::Restart);Issue(ETacticalCommand::Sample);Issue(ETacticalCommand::Execute);
    const int Resolved=State->Journey.world.armies[3].troops,ResolvedTurn=State->Journey.turn;
    AdvancePresentation(.6f);auto C=ReadTacticCinema();
    if(!Check(C.bVisible && !C.bResolved && C.Primary.IsEmpty() && C.Intro>0 && CurrentBeat()->ChainStep==1,TEXT("intro names actor and step, never future damage"))) return false;
    AdvancePresentation(1.1f);C=ReadTacticCinema();
    if(!Check(C.bResolved && C.Primary==TEXT("敵を引き出した") && C.Secondary.Contains(TEXT("60")),TEXT("lure shows actual bait loss and movement"))) return false;
    AdvancePresentation(1.3f);C=ReadTacticCinema();
    if(!Check(CurrentBeat()->Kind==EBattleCue::Ambush && C.Primary.IsEmpty() && !CueReason().Contains(TEXT("553")) && State->Units[3].Troops==1800,TEXT("next-step intro cannot reveal its later casualties"))) return false;
    AdvancePresentation(1.6f);C=ReadTacticCinema();
    if(!Check(C.Primary==TEXT("−553") && C.Secondary.Contains(TEXT("混乱")) && CurrentBeat()->ChainStep==2,TEXT("actual ambush damage and disorder at impact"))) return false;
    int Quads=0;bool Finite=true;
    auto Draw=[&](FVector A,FVector B,FVector D,FVector E,FLinearColor Color) { ++Quads;Finite=Finite && !A.ContainsNaN() && !B.ContainsNaN() && !D.ContainsNaN() && !E.ContainsNaN() && FMath::IsFinite(Color.R) && FMath::IsFinite(Color.G) && FMath::IsFinite(Color.B) && FMath::IsFinite(Color.A); };
    gunyufx::Tactic(*CurrentBeat(),State->Units,BeatProgress(),Draw);
    if(!Check(Quads>0 && Quads<250 && Finite,TEXT("bounded finite independent-quad tactic geometry"))) return false;
    auto Hidden=*CurrentBeat();Hidden.Kind=EBattleCue::AzureDragon;Hidden.Target=4;Quads=0;
    gunyufx::Tactic(Hidden,State->Units,.6f,Draw);
    if(!Check(Quads==0,TEXT("no attack geometry at an unobserved reserve"))) return false;
    for(int I=0;I<12;++I) ReadTacticCinema();
    if(!Check(State->Journey.world.armies[3].troops==Resolved && State->Journey.turn==ResolvedTurn,TEXT("reading cinematic feedback never applies effects again"))) return false;
    Issue(ETacticalCommand::Skip);
    if(!Check(!ReadTacticCinema().bVisible && State->Units[3].Troops==Resolved,TEXT("skip clears cinematic feedback without replaying damage"))) return false;
    Issue(ETacticalCommand::Restart);Issue(ETacticalCommand::Sample);State->Journey.world.armies[2].morale=5;EditOrder(2,-1);Issue(ETacticalCommand::Execute);AdvancePresentation(4.5f);C=ReadTacticCinema();
    if(!Check(CurrentBeat()->Kind==EBattleCue::Stop && C.Primary==TEXT("連鎖停止") && !C.bHit,TEXT("failed step has no successful impact stamp"))) return false;
    Quads=0;gunyufx::Tactic(*CurrentBeat(),State->Units,BeatProgress(),Draw);
    if(!Check(Quads==0,TEXT("stopped chain has no fabricated attack effects"))) return false;
    Issue(ETacticalCommand::Skip);
    Issue(ETacticalCommand::Restart);
    State->Journey.world.armies[1].morale=State->Journey.world.armies[2].morale=100;
    SelectOfficer(0);SelectTactic(static_cast<int>(frontline::ChainTactic::Oath));Issue(ETacticalCommand::QueueTactic);Issue(ETacticalCommand::Execute);AdvancePresentation(2.f);C=ReadTacticCinema();
    if(!Check(C.Primary==TEXT("鼓舞") && C.Secondary.Contains(TEXT("3隊")) && C.Secondary.Contains(TEXT("+0")),TEXT("capped morale still reports actual inspiration, without invented healing"))) return false;
    Quads=0;gunyufx::Tactic(*CurrentBeat(),State->Units,BeatProgress(),Draw);
    if(!Check(Quads==24,TEXT("both capped but inspired allies receive support light"))) return false;
    Issue(ETacticalCommand::Skip);
    FirstMission=SavedMission;EnemyPolicy=SavedPolicy;bStartWithCouncil=SavedCouncil;State->bNoPlayback=SavedPlayback;bRealtimeCommand=SavedRealtime;EffectsVolume=SavedVolume;PlaybackSpeed=SavedSpeed;
    Issue(ETacticalCommand::Restart);
    UE_LOG(LogTemp,Display,TEXT("GUNYU_TACTIC_CINEMA: PASS (intro/impact timing, actual step effects, stopped chain, skip, hidden target, bounded geometry)"));return true;
}
