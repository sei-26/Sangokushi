#include "TacticalTrial.h"
#include "TacticalState.h"
using namespace frontline;
void ATacticalField::PrepareTacticAvailabilityPreview(bool bUsedSignature)
{
    FirstMission=0;EnemyPolicy=1;bStartWithCouncil=true;bRealtimeCommand=false;Issue(ETacticalCommand::Restart);
    for(int I=0;I<3;++I) Issue(ETacticalCommand::Dialogue);
    Issue(ETacticalCommand::PromiseRescue);Issue(ETacticalCommand::PlanSupplies);Issue(ETacticalCommand::Deploy);
    // A bounded QA fixture for post-combat escort guidance, never normal startup.
    State->Journey.world.armies[3].troops=State->Journey.world.armies[4].troops=0;
    State->Journey.RefreshIntelligence();
    State->Journey.chain.signatureUsed[0]=bUsedSignature;
    SelectOfficer(bUsedSignature ? 0:2);SelectTactic(int(bUsedSignature ? ChainTactic::Oath:ChainTactic::Feint));Refresh();
}
bool ATacticalField::TacticAvailabilitySmokeTest()
{
    auto Check=[](bool V,const TCHAR* Why) { if(!V) UE_LOG(LogTemp,Error,TEXT("GUNYU_TACTIC_AVAILABILITY: FAIL %s"),Why);return V; };
    const int SavedMission=FirstMission,SavedPolicy=EnemyPolicy;
    const bool SavedCouncil=bStartWithCouncil,SavedRealtime=bRealtimeCommand,SavedNoPlayback=State->bNoPlayback;
    bRealtimeCommand=false;bStartWithCouncil=true;FirstMission=0;EnemyPolicy=1;State->bNoPlayback=false;
    Issue(ETacticalCommand::Restart);for(int I=0;I<3;++I) Issue(ETacticalCommand::Dialogue);
    Issue(ETacticalCommand::PromiseRescue);Issue(ETacticalCommand::PlanSupplies);Issue(ETacticalCommand::Deploy);
    const int Dest[]={Campaign::At(9,11),Campaign::At(9,9),Campaign::At(9,10)};
    for(int I=0;I<3;++I) { SelectOfficer(I);Issue(ETacticalCommand::Move);CommandAt(Dest[I]);Issue(ETacticalCommand::Skip); }
    for(int I=0;I<3;++I) { SelectOfficer(I);SelectTactic(int(ChainTactic::Feint));Issue(ETacticalCommand::QueueTactic); }
    Issue(ETacticalCommand::Execute);Issue(ETacticalCommand::Skip);
    for(int I=0;I<3;++I)
    {
        SelectOfficer(I);
        if(!Check(!CanIssue(ETacticalCommand::QueueTactic) && Presentation().TacticAvailability.Contains(TEXT("行動済み")),TEXT("all spent officers show the actual recovery action"))) return false;
    }
    Action(EKeys::SpaceBar);Issue(ETacticalCommand::Skip);
    for(int I=0;I<3;++I)
    {
        SelectOfficer(I);
        if(!Check(CanIssue(ETacticalCommand::QueueTactic) && TacticReservationBlockReason().IsEmpty(),TEXT("public next command restores ordinary tactics for every living officer"))) return false;
    }
    SelectOfficer(0);SelectTactic(int(ChainTactic::Oath));Issue(ETacticalCommand::QueueTactic);Issue(ETacticalCommand::Execute);Issue(ETacticalCommand::Skip);
    Action(EKeys::SpaceBar);Issue(ETacticalCommand::Skip);
    if(!Check(!CanIssue(ETacticalCommand::QueueTactic) && Presentation().TacticAvailability.Contains(TEXT("通常戦法へ")),TEXT("spent signature explains switching rather than waiting another turn"))) return false;
    SelectTactic(int(ChainTactic::Feint));
    if(!Check(CanIssue(ETacticalCommand::QueueTactic),TEXT("switching from a used signature restores a valid common reservation"))) return false;
    auto& J=State->Journey;
    J.world.armies[4].tile=J.world.armies[4].target=Campaign::At(11,10);J.RefreshIntelligence();Refresh();
    if(!Check(State->Target==3 && J.intelligence[0].Find(4) && J.intelligence[0].Find(4)->current,TEXT("fixture observes a second living hostile without changing a valid target"))) return false;
    State->Orders={{2,3,ChainTactic::Feint,-1}};J.world.armies[3].troops=0;J.RefreshIntelligence();Refresh();
    if(!Check(State->Target==4 && CanIssue(ETacticalCommand::QueueTactic) && State->Orders[0].enemy==3,TEXT("new selections target the living observed reserve, while old orders keep their authorized target"))) return false;
    Issue(ETacticalCommand::Clear);const FString ObservedTarget=Presentation().Target;
    J.world.armies[4].troops=0;J.world.armies[4].tile=Campaign::At(1,1);State->Target=-1;Refresh();
    if(!Check(State->Target==4 && Presentation().Target==ObservedTarget && CanIssue(ETacticalCommand::QueueTactic),TEXT("unobserved changes in a real army never alter target choice or guidance"))) return false;
    J.RefreshIntelligence();Refresh();
    for(int I=0;I<3;++I)
    {
        SelectOfficer(I);SelectTactic(int(ChainTactic::Feint));
        if(!Check(State->Target==-1 && !CanIssue(ETacticalCommand::QueueTactic) && Presentation().TacticAvailability.Contains(TEXT("確認中の敵なし")) && Presentation().TacticAvailability.Contains(TEXT("護送")),TEXT("all attack tactics explain missing targets and the unfinished escort"))) return false;
    }
    Action(EKeys::SpaceBar);Issue(ETacticalCommand::Skip);
    if(!Check(J.stage==JourneyStage::Battle && !CanIssue(ETacticalCommand::QueueTactic) && Presentation().TacticAvailability.Contains(TEXT("護送")),TEXT("advancing does not fabricate targets when the escort still needs completion"))) return false;
    PrepareTacticAvailabilityPreview(false);SelectOfficer(0);SelectTactic(int(ChainTactic::Oath));
    if(!Check(State->Target==-1 && CanIssue(ETacticalCommand::QueueTactic) && !Presentation().bTacticUnavailable,TEXT("unused support can still be reserved without an enemy"))) return false;
    std::vector<std::uint8_t> Before,After;ExportSession(Before);Refresh();
    if(!Check(ExportSession(After) && Before==After,TEXT("stable status refresh spends nothing and preserves selection"))) return false;
    FirstMission=SavedMission;EnemyPolicy=SavedPolicy;bStartWithCouncil=SavedCouncil;bRealtimeCommand=SavedRealtime;State->bNoPlayback=SavedNoPlayback;Issue(ETacticalCommand::Restart);
    UE_LOG(LogTemp,Display,TEXT("GUNYU_TACTIC_AVAILABILITY: PASS (all spent recovery, used signature, live observed target, no hidden-state selection, preserved orders, empty-target escort, support, immutable refresh)"));return true;
}
