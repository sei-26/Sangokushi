#include "TacticalTrial.h"
#include "TacticalState.h"
#include "TacticalChainReview.h"
bool ATacticalField::ComboSmokeTest()
{
    auto Check=[](bool V,const TCHAR* Why) { if(!V) UE_LOG(LogTemp,Error,TEXT("GUNYU_COMBO: FAIL %s"),Why);return V; };
    const int SavedMission=FirstMission,SavedPolicy=EnemyPolicy;
    const bool SavedCouncil=bStartWithCouncil,SavedRealtime=bRealtimeCommand,SavedPlayback=State->bNoPlayback;
    const float SavedSpeed=PlaybackSpeed;
    FirstMission=0;EnemyPolicy=0;bStartWithCouncil=false;bRealtimeCommand=false;State->bNoPlayback=false;PlaybackSpeed=1;
    Issue(ETacticalCommand::Restart);
    SelectOfficer(0);Issue(ETacticalCommand::Move);CommandAt(frontline::Campaign::At(9,11));Issue(ETacticalCommand::Skip);
    SelectOfficer(2);Issue(ETacticalCommand::Move);CommandAt(frontline::Campaign::At(9,10));Issue(ETacticalCommand::Skip);
    SelectOfficer(2);SelectTactic(int(frontline::ChainTactic::Breakthrough));Issue(ETacticalCommand::QueueTactic);
    if(!Check(Presentation().Predicted==1 && Presentation().Orders[0].Combo.Contains(TEXT("100％")),TEXT("single unprepared breakthrough has a 100% forecast"))) return false;
    Issue(ETacticalCommand::Execute);
    if(!Check(CurrentBeat() && CurrentBeat()->Combo==1 && CurrentBeat()->ComboPercent==100 && ReadTacticCinema().Primary.IsEmpty(),TEXT("solo replay starts without revealing its outcome"))) return false;
    AdvancePresentation(1.6f);auto C=ReadTacticCinema();
    if(!Check(C.Chain.Contains(TEXT("1コンボ")) && C.Chain.Contains(TEXT("100％")) && C.Primary==TEXT("−417"),TEXT("single impact reports its actual normal damage and combo"))) return false;
    Issue(ETacticalCommand::Skip);
    Issue(ETacticalCommand::Restart);Issue(ETacticalCommand::Sample);
    if(!Check(Presentation().Orders[1].Combo.Contains(TEXT("125％")) && Presentation().Orders[2].Combo.Contains(TEXT("150％")),TEXT("planner shows successive combo multipliers"))) return false;
    Issue(ETacticalCommand::Execute);AdvancePresentation(6.2f);C=ReadTacticCinema();
    if(!Check(CurrentBeat()->ChainStep==3 && C.Primary.IsEmpty() && !C.Chain.Contains(TEXT("3コンボ")),TEXT("third-hand windup does not claim its future success"))) return false;
    AdvancePresentation(1.4f);C=ReadTacticCinema();
    if(!Check(C.Chain.Contains(TEXT("3コンボ")) && C.Chain.Contains(TEXT("150％")) && State->Units[3].Loss==781,TEXT("finisher publishes 150% damage once at impact"))) return false;
    Issue(ETacticalCommand::Skip);const auto Review=ReadChainReview();
    if(!Check(Review.Damage==1334 && Review.Rows[2].Role.Contains(TEXT("150％")),TEXT("review uses persisted real combo facts"))) return false;
    std::vector<std::uint8_t> Bytes;
    if(!Check(ExportSession(Bytes) && ImportSession(Bytes) && ReadChainReview().Rows[2].Role==Review.Rows[2].Role,TEXT("save/load preserves combo attribution"))) return false;
    Issue(ETacticalCommand::Restart);Issue(ETacticalCommand::Sample);
    SelectOfficer(2);Issue(ETacticalCommand::Move);CommandAt(frontline::Campaign::At(7,10));Issue(ETacticalCommand::Skip);EditOrder(2,-1);
    Issue(ETacticalCommand::Execute);Issue(ETacticalCommand::Skip);
    if(!Check(State->Journey.lastChain.completed==1 && State->Journey.lastChain.events.size()==2 && State->Journey.lastChain.events[1].combo==0 && ReadChainReview().bStopped,TEXT("public out-of-range second hand stops without a fabricated second combo"))) return false;
    FirstMission=SavedMission;EnemyPolicy=SavedPolicy;bStartWithCouncil=SavedCouncil;bRealtimeCommand=SavedRealtime;State->bNoPlayback=SavedPlayback;PlaybackSpeed=SavedSpeed;Issue(ETacticalCommand::Restart);
    UE_LOG(LogTemp,Display,TEXT("GUNYU_COMBO: PASS (solo UI, predicted multipliers, current-step impact, actual damage, review, save/load)"));return true;
}
