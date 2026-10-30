#include "TacticalTrial.h"
#include "TacticalState.h"
#include "TacticalTargetVisual.h"
#include "JourneySave.hpp"
using namespace frontline;
void ATacticalField::PrepareTacticTargetPreview(int Variant)
{
    bLegacyJourney=false;bStartWithCouncil=false;FirstMission=0;EnemyPolicy=0;bRealtimeCommand=false;Issue(ETacticalCommand::Restart);
    if(Variant==1)
    {
        State->Orders={{0,3,ChainTactic::Lure,Campaign::At(8,11)}};SelectOfficer(1);
    }
    else
    {
        SelectOfficer(0);Issue(ETacticalCommand::Move);CommandAt(Campaign::At(9,11));Issue(ETacticalCommand::Skip);
        SelectOfficer(2);Issue(ETacticalCommand::Move);CommandAt(Campaign::At(9,10));Issue(ETacticalCommand::Skip);
        SelectTactic(int(ChainTactic::Feint));
        if(Variant==2)
        {
            State->Journey.world.armies[4].tile=State->Journey.world.armies[4].target=Campaign::At(9,9);
            State->Journey.RefreshIntelligence();Refresh();
        }
    }
}
bool ATacticalField::TacticTargetSmokeTest()
{
    auto Check=[](bool V,const TCHAR* Why) { if(!V) UE_LOG(LogTemp,Error,TEXT("GUNYU_TACTIC_TARGETS: FAIL %s"),Why);return V; };
    const int SavedMission=FirstMission,SavedPolicy=EnemyPolicy;
    const bool SavedCouncil=bStartWithCouncil,SavedRealtime=bRealtimeCommand,SavedLegacy=bLegacyJourney,SavedNoPlayback=State->bNoPlayback;
    State->bNoPlayback=false;
    auto Bytes=[this]() { std::vector<std::uint8_t> B;ExportSession(B);return B; };
    auto Target=[this](int Id) { return ReadTacticTarget(Id); };
    PrepareTacticTargetPreview(0);const auto Solo=Target(3);
    if(!Check(Solo.bReady && Solo.Step==1 && Solo.ComboPercent==100 && Solo.Damage>0 && Presentation().ReadyTacticTargets==1,TEXT("solo candidate uses actual 100 percent preview"))) return false;
    const auto Stable=Bytes();BuildPresentation();RenderEffects();
    if(!Check(Bytes()==Stable,TEXT("target read and ground rendering leave serialized gameplay unchanged"))) return false;
    Issue(ETacticalCommand::QueueTactic);Issue(ETacticalCommand::Execute);
    if(!Check(IsPlaying() && !gunyutargetui::Find(Presentation(),3) && !SelectNextTacticTarget(),TEXT("playback hides planning markers and blocks target focus"))) return false;
    if(!Check(State->Journey.lastChain.completed==1 && State->Journey.lastChain.events[0].damage==Solo.Damage,TEXT("actual solo resolution equals the observed forecast in a fully observed encounter"))) return false;
    Issue(ETacticalCommand::Skip);
    if(!Check(Presentation().ReadyTacticTargets==0 && Presentation().TacticAvailability.Contains(TEXT("行動済み")),TEXT("actual spending removes ready targets"))) return false;
    // Current distance is insufficient, but the already reserved lure creates contact.
    PrepareTacticTargetPreview(1);const auto Follow=Target(3);const auto Prefix=State->Orders;
    State->Orders.clear();Refresh();
    if(!Check(!Target(3).bReady && Presentation().bTacticUnavailable,TEXT("Guan cannot claim solo range before the lure"))) return false;
    State->Orders=Prefix;Refresh();
    if(!Check(Follow.bReady && Follow.Step==2 && Follow.ComboPercent==125 && Presentation().TacticAvailability.Contains(TEXT("125％")),TEXT("map and command panel both accept the post-lure second step"))) return false;
    Issue(ETacticalCommand::QueueTactic);SelectOfficer(2);const auto Finish=Target(3);
    if(!Check(Finish.bReady && Finish.Step==3 && Finish.ComboPercent==150,TEXT("third actor gets the real 150 percent continuation"))) return false;
    Issue(ETacticalCommand::QueueTactic);Issue(ETacticalCommand::Execute);Issue(ETacticalCommand::Skip);
    if(!Check(State->Journey.lastChain.completed==3 && State->Journey.lastChain.events[1].damage==Follow.Damage && State->Journey.lastChain.events[2].damage==Finish.Damage,TEXT("normal three-step execution matches target previews including displacement"))) return false;
    PrepareTacticTargetPreview(1);State->Orders[0].destination=-1;Refresh();
    if(!Check(!Target(3).bReady && Target(3).Detail.Contains(TEXT("第1手が停止")) && Target(3).Badge==TEXT("先行手停止"),TEXT("broken predecessor never produces a false ready successor"))) return false;
    PrepareTacticTargetPreview(1);State->Orders.clear();SelectOfficer(0);SelectTactic(int(ChainTactic::Lure));
    const int Choices=Target(3).RetreatChoices;Issue(ETacticalCommand::QueueTactic);
    if(!Check(Choices>0 && State->Input==ETacticalInput::Lure && State->Reachable.Num()==Choices && !Presentation().bTacticTargetOverlay && Presentation().TacticTargetSummary.IsEmpty() && !SelectNextTacticTarget(),TEXT("lure hints use exactly the real observed legal retreat cells and cannot interrupt destination input"))) return false;
    Issue(ETacticalCommand::Cancel);State->Journey.world.armies[3].stance=battle::Stance::Guard;State->Journey.RefreshIntelligence();Refresh();
    if(!Check(!Target(3).bReady && Presentation().TacticAvailability.Contains(TEXT("固守")),TEXT("guarded enemy is not marked as a lure target"))) return false;
    State->Journey.world.armies[3].stance=battle::Stance::Balanced;State->Journey.RefreshIntelligence();
    State->Journey.chain.moved[0]=true;Refresh();
    if(!Check(!Target(3).bReady && Target(3).Detail.Contains(TEXT("移動済み")),TEXT("lure needs an unused move"))) return false;
    PrepareTacticTargetPreview(0);State->Journey.world.armies[2].morale=0;Refresh();
    if(!Check(!Target(3).bReady && Target(3).Detail.Contains(TEXT("士気")),TEXT("morale cost is not bypassed by a range marker"))) return false;
    State->Journey.world.armies[2].morale=90;State->Journey.world.armies[2].troops=149;Refresh();
    if(!Check(!Target(3).bReady,TEXT("minimum troops still applies"))) return false;
    PrepareTacticTargetPreview(0);SelectTactic(int(ChainTactic::SerpentRoar));State->Journey.chain.signatureUsed[2]=true;Refresh();
    if(!Check(!Target(3).bReady && Target(3).Detail.Contains(TEXT("使用済み")),TEXT("used signature never becomes a ready target"))) return false;
    SelectTactic(int(ChainTactic::Feint));
    State->Journey.world.armies[3].morale=50;State->Journey.RefreshIntelligence();
    const int Push=TacticChain::PushTile(State->Journey.world,2,3);
    State->Journey.world.tiles[Push].terrain=Terrain::River;SelectTactic(int(ChainTactic::Breakthrough));
    if(!Check(!Target(3).bReady && Target(3).Detail.Contains(TEXT("塞がれて")),TEXT("real push blockage prevents a false ready breakthrough"))) return false;
    // Viewing stale/hidden actual armies must not modify any target read.
    PrepareTacticTargetPreview(0);const auto Known=Target(3);const auto Summary=Presentation().TacticTargetSummary;
    State->Journey.world.armies[3].tile=Campaign::At(1,1);State->Journey.world.armies[3].troops=0;
    State->Journey.world.armies[4].tile=Campaign::At(10,11);State->Journey.world.armies[4].morale=1;Refresh();
    if(!Check(Target(3).Detail==Known.Detail && Target(3).Tile==Known.Tile && Target(3).Damage==Known.Damage && Presentation().TacticTargetSummary==Summary && Target(4).Army<0,TEXT("unchanged observations completely isolate target hints from hidden actual positions, casualties and morale"))) return false;
    // A focus changes only the authorized selected contact and pause, preserving all orders/signals.
    PrepareTacticTargetPreview(2);bRealtimeCommand=true;Refresh();
    State->Orders={{0,-1,ChainTactic::Oath,-1}};State->Realtime.Queue(State->Journey,1,JourneyDirective::Hold);
    if(!Check(State->ChainSignal.Arm(State->Journey,State->Orders),TEXT("existing support reservation has an armed signal"))) return false;
    State->Realtime.paused=false;State->Realtime.Advance(2);Refresh();
    if(!Check(Presentation().ReadyTacticTargets==2,TEXT("two observed enemies are legal second-step targets"))) return false;
    auto Expected=std::make_unique<JourneySaveData>();
    if(!Check(DecodeJourneySave(Bytes(),*Expected),TEXT("exact state snapshot decodes"))) return false;
    Expected->target=4;Expected->realtime.paused=true;std::vector<std::uint8_t> ExpectedBytes;EncodeJourneySave(*Expected,ExpectedBytes);
    if(!Check(SelectNextTacticTarget() && State->Target==4 && Bytes()==ExpectedBytes && State->ChainSignal.Armed(),TEXT("focus cycles to the other ready contact and only changes target/pause"))) return false;
    const auto Chosen=Bytes();const auto Detail=Target(4).Detail;
    if(!Check(ImportSession(Chosen) && State->Target==4 && Target(4).Detail==Detail && Presentation().ReadyTacticTargets==2,TEXT("load regenerates the same target predictions without storing new save fields"))) return false;
    if(!Check(SelectNextTacticTarget() && State->Target==3,TEXT("focus cycles back, never auto-reserves or executes"))) return false;
    for(auto& Seen:State->Journey.intelligence[0].contacts) Seen.current=false;
    if(!Check(!SelectNextTacticTarget(),TEXT("stale cached candidates are revalidated against current observations"))) return false;
    Refresh();
    if(!Check(Presentation().TacticTargets.IsEmpty() && !SelectNextTacticTarget(),TEXT("no contacts yields no fake candidate"))) return false;
    Issue(ETacticalCommand::Clear);SelectOfficer(0);SelectTactic(int(ChainTactic::Oath));
    if(!Check(Presentation().TacticTargets.IsEmpty() && !Presentation().bTacticTargetOverlay && CanIssue(ETacticalCommand::QueueTactic),TEXT("support requires no enemy and clears old offensive markers"))) return false;
    FirstMission=SavedMission;EnemyPolicy=SavedPolicy;bStartWithCouncil=SavedCouncil;bRealtimeCommand=SavedRealtime;bLegacyJourney=SavedLegacy;State->bNoPlayback=SavedNoPlayback;Issue(ETacticalCommand::Restart);
    UE_LOG(LogTemp,Display,TEXT("GUNYU_TACTIC_TARGETS: PASS (solo and 125/150 percent continuations, actual execution, legal lure retreats, resource and push restrictions, exact focus conservation, observation privacy, playback lock and save/load)"));return true;
}
