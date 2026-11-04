#include "TacticalTrial.h"
#include "TacticalState.h"
#include "TacticalContributionQA.h"
#include "TacticalSupportQA.h"
#include "JourneyDefense.hpp"
using namespace frontline;
bool ATacticalField::DefenseSmokeTest()
{
    auto Check=[](bool V,const TCHAR* Why) { if(!V) UE_LOG(LogTemp,Error,TEXT("GUNYU_DEFENSE: FAIL %s"),Why);return V; };
    Issue(ETacticalCommand::Skip);std::vector<std::uint8_t> Original,Prepared,Natural,After;
    const bool Legacy=bLegacyJourney,Playback=State->bNoPlayback;const float Speed=PlaybackSpeed,Volume=EffectsVolume;
    if(!Check(ExportSession(Original) && PrepareCooperationCouncilPreview(this,0),TEXT("actual prior victory leads to the ordinary unpaid council"))) return false;
    State->bNoPlayback=false;PlaybackSpeed=1;EffectsVolume=0;
    Issue(ETacticalCommand::PlanExample);Issue(ETacticalCommand::PlanAmbush);Issue(ETacticalCommand::Deploy);
    if(!Check(PrepareDefensePreview(this,0),TEXT("paid deployment plus authored contact boundary executes real feint and guard disrupt"))) return false;
    const auto& J=State->Journey;const int Goal=J.goal,South=J.Definition().secondGoal,Turn=J.turn;
    const auto& Chain=J.lastChain;
    if(!Check(Chain.completed==2 && Chain.events.size()==2 && Chain.events[0].tactic==ChainTactic::Feint && Chain.events[1].tactic==ChainTactic::Disrupt &&
        Chain.events[1].comboPercent==125 && J.chain.disordered[3] && J.chain.spent[0] && J.chain.spent[1] && !J.guarding[1] && !J.chain.spent[2],TEXT("the guard actually spends its action in a two-officer chain that disorders the adjacent enemy"))) return false;
    if(!Check(State->HUD.Defenses.Num()==2 && State->HUD.Defenses[0].Current.Contains(TEXT("固守待ち")) && State->HUD.Defenses[0].Last.Contains(TEXT("報告なし")) &&
        ReadJourneyDefenseCheck(J,Goal).reason==JourneyDefenseReason::Unchecked && ExportSession(Prepared),TEXT("current guard cost is visible without inventing the next cycle's outcome"))) return false;
    const auto Orders=State->Realtime.orders;const auto Trust=J.officerTrust;const auto Camp=J.world.cities[0];
    for(int I=0;I<3;++I) { BuildDefensePresentation();CaptureMissionProgress();ReadMissionResponse();Refresh(); }
    if(!Check(ExportSession(After) && Prepared==After && J.officerTrust==Trust && J.world.cities[0].food==Camp.food && J.world.cities[0].gold==Camp.gold,TEXT("repeated defense reads spend no orders resources morale or trust"))) return false;
    const FString LastBefore=State->HUD.Defenses[0].Last;Issue(ETacticalCommand::RealtimeStep);
    if(!Check(IsPlaying() && CurrentBeat()->Kind==EBattleCue::Enemy && ReadMissionResponse().Line.IsEmpty() && State->HUD.Defenses[0].Last==LastBefore,TEXT("the actual cycle resolves once while HUD and dialogue withhold its future defense result"))) return false;
    const auto* Beat=CurrentBeat();const auto& Checks=Beat->MissionAfter.DefenseChecks;
    if(!Check(Checks.Num()==2 && Checks[0].Goal==Goal && Checks[1].Goal==South && Checks[0].Turn==Turn && Checks[0].Reason==int(JourneyDefenseReason::GuardSpent) &&
        !Checks[0].bSecured && !Checks[0].bGuarded && Checks[0].bAttackSpent && Checks[1].bSecured && Beat->MissionAfter.Held==0,TEXT("same instant records a failed spent guard and the independently secured opposite wing"))) return false;
    AdvancePresentation(20);
    const auto Actual=ReadJourneyDefenseCheck(State->Journey,Goal),Now=ReadJourneyDefense(State->Journey,Goal);
    if(!Check(!IsPlaying() && RealtimePaused() && State->Journey.turn==Turn+1 && Actual.reason==JourneyDefenseReason::GuardSpent && Now.reason==JourneyDefenseReason::NeedsGuard &&
        State->HUD.Defenses[0].Last.Contains(TEXT("×")) && State->HUD.Defenses[0].Last.Contains(TEXT("固守なし")) && State->HUD.Defenses[0].Current.Contains(TEXT("固守指示あり")),TEXT("natural completion separates previous failed defense from the now recovered action and retained hold order"))) return false;
    if(!Check(ReadMissionResponse().Actor==1 && !ReadMissionResponse().Line.IsEmpty() && ReadMissionResponse().Caption.Contains(TEXT("北")) && ReadMissionResponse().Caption.Contains(TEXT("維持できず")) &&
        State->Journey.world.armies[1].tile==Goal && ExportSession(Natural),TEXT("the actual stationary guard reports only its local consequence after playback"))) return false;
    for(int A=0;A<3;++A) if(!Check(SameJourneyOrder(Orders[A],State->Realtime.orders[A]),TEXT("the defense report preserves every retained standing order"))) return false;
    for(int I=0;I<3;++I) { ReadMissionResponse();BuildDefensePresentation();Refresh(); }AdvancePresentation(.5f);
    if(!Check(ExportSession(After) && After==Natural,TEXT("reading the actual result and its timer change no persistent result"))) return false;
    if(!Check(ImportSession(Natural) && ReadMissionResponse().Line.IsEmpty() && State->HUD.Defenses[0].Last.Contains(TEXT("報告なし")) &&
        ReadJourneyDefenseCheck(State->Journey,Goal).reason==JourneyDefenseReason::Unchecked && ExportSession(After) && After==Natural,TEXT("load clears transient checked results and speech while preserving the exact saved battle"))) return false;
    if(!Check(ImportSession(Prepared),TEXT("pre-cycle save restores the same spent guard and retained hold orders"))) return false;
    Issue(ETacticalCommand::RealtimeStep);Issue(ETacticalCommand::Skip);
    if(!Check(!IsPlaying() && ReadMissionResponse().Line.IsEmpty() && State->HUD.Defenses[0].Last.Contains(TEXT("固守なし")) && ExportSession(After) && After==Natural,TEXT("skip suppresses speech but keeps the exact actual defense and save outcome"))) return false;
    Issue(ETacticalCommand::RealtimeStep);
    if(!Check(IsPlaying() && ReadMissionResponse().Line.IsEmpty(),TEXT("recovery remains private until the next real cycle finishes"))) return false;
    AdvancePresentation(20);
    const auto Recovered=ReadJourneyDefenseCheck(State->Journey,Goal),Other=ReadJourneyDefenseCheck(State->Journey,South);
    if(!Check(!IsPlaying() && Recovered.secured && Recovered.guarded && Other.secured && State->Journey.heldTurns==1 &&
        State->HUD.Defenses[0].bLastSecured && State->HUD.Defenses[1].bLastSecured && !ReadMissionResponse().Caption.Contains(TEXT("固守なし")),TEXT("retained hold acts after normal recovery and the next actual two-wing check advances maintained progress"))) return false;
    Issue(ETacticalCommand::Skip);bLegacyJourney=Legacy;ImportSession(Original);State->bNoPlayback=Playback;PlaybackSpeed=Speed;EffectsVolume=Volume;
    UE_LOG(LogTemp,Display,TEXT("GUNYU_DEFENSE: PASS (paid deployment, real guard attack and next-cycle consequence, separate live readiness, actual retained-hold recovery, immutable reads, load clearing and skip equality)"));return true;
}
