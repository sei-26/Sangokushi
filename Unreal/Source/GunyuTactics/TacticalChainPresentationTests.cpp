#include "TacticalTrial.h"
#include "TacticalState.h"
#include "TacticalChainVisual.h"
using namespace frontline;
bool ATacticalField::ChainPresentationSmokeTest()
{
    const int SavedMission=FirstMission,SavedPolicy=EnemyPolicy;
    const bool SavedCouncil=bStartWithCouncil,SavedNoPlayback=State->bNoPlayback,SavedRealtime=bRealtimeCommand;
    auto Check=[](bool Value,const TCHAR* Why) { if(!Value) UE_LOG(LogTemp,Error,TEXT("GUNYU_CHAIN_PRESENTATION: FAIL %s"),Why);return Value; };
    FirstMission=0;EnemyPolicy=0;bStartWithCouncil=false;bRealtimeCommand=false;State->bNoPlayback=true;
    Issue(ETacticalCommand::Restart);Issue(ETacticalCommand::Sample);
    const auto& O=Presentation().Orders;
    if(!Check(O.Num()==3 && Presentation().Predicted==3,TEXT("full chain forecast"))) return false;
    for(int I=0;I<3;++I)
        if(!Check(O[I].Actor==I && O[I].Target==3 && O[I].bEvaluated && O[I].bSuccess && !O[I].ActorName.IsEmpty() && !O[I].TacticName.IsEmpty(),TEXT("cards have actual actor, observed target and evaluated forecast"))) return false;
    const int EnemyTroops=State->Journey.world.armies[3].troops;
    EditOrder(2,-1);
    const auto& Failed=Presentation().Orders;
    if(!Check(Presentation().Predicted==1 && Failed[0].bSuccess && Failed[1].bEvaluated && !Failed[1].bSuccess && !Failed[2].bEvaluated && Failed[1].Actor==2 && Failed[2].Actor==1,TEXT("reorder distinguishes success, first stop and unevaluated successor"))) return false;
    if(!Check(gunyuchainui::Status(Failed[1])!=gunyuchainui::Status(Failed[2]) && State->Journey.world.armies[3].troops==EnemyTroops && !State->Journey.chain.spent[0],TEXT("forecast consumes no morale, action or enemy soldiers"))) return false;
    std::vector<std::uint8_t> Before,After;
    if(!ExportSession(Before)) return false;
    Refresh();
    if(!Check(ExportSession(After) && Before==After,TEXT("presentation rebuild is save-state neutral"))) return false;
    Issue(ETacticalCommand::Clear);SelectOfficer(0);SelectTactic(static_cast<int>(ChainTactic::Oath));Issue(ETacticalCommand::QueueTactic);
    if(!Check(Presentation().Orders.Num()==1 && Presentation().Orders[0].Target==-1,TEXT("support never gains a fabricated hostile target"))) return false;
    FirstMission=SavedMission;EnemyPolicy=SavedPolicy;bStartWithCouncil=SavedCouncil;State->bNoPlayback=SavedNoPlayback;bRealtimeCommand=SavedRealtime;
    Issue(ETacticalCommand::Restart);
    UE_LOG(LogTemp,Display,TEXT("GUNYU_CHAIN_PRESENTATION: PASS (real actor/target, evaluated vs held, reorder, immutable preview, support target)"));
    return true;
}
