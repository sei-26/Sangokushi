#include "TacticalTrial.h"
#include "TacticalState.h"
#include "TacticalChainReview.h"
bool ATacticalField::ChainReviewSmokeTest()
{
    auto Check=[](bool V,const TCHAR* Why) { if(!V) UE_LOG(LogTemp,Error,TEXT("GUNYU_CHAIN_REVIEW: FAIL %s"),Why);return V; };
    using T=frontline::ChainTactic;frontline::ChainResult Result;
    TArray<FString> Names{TEXT("劉備"),TEXT("関羽"),TEXT("張飛"),TEXT("先鋒"),TEXT(""),TEXT("")};
    frontline::ChainEvent Lure;Lure.army=0;Lure.enemy=3;Lure.tactic=T::Lure;Lure.success=true;Lure.from=10;Lure.to=11;
    auto Ambush=Lure;Ambush.army=1;Ambush.tactic=T::Ambush;Ambush.from=Ambush.to=11;Ambush.damage=413;
    Ambush.changes.push_back({3,11,1387,65,false,true,false,false});
    auto Push=Ambush;Push.army=2;Push.tactic=T::Breakthrough;Push.to=12;Push.damage=556;
    Result.events={Lure,Ambush,Push};Result.completed=3;
    auto R=gunyureview::Build(Result,Names);
    if(!Check(R.bAvailable && R.Completed==3 && R.Damage==969 && R.Rows[1].Link.Contains(TEXT("第1手")) && R.Rows[2].Link.Contains(TEXT("第2手")),TEXT("actual damage and same-target setup links are explicit"))) return false;
    Result.events[1].success=false;Result.events[1].reason=U"敵の隊列が整っている。";
    R=gunyureview::Build(Result,Names);
    if(!Check(R.Completed==1 && R.Damage==0 && R.bStopped && R.Rows.Num()==2 && R.Rows[1].Reason.Contains(TEXT("整っている")),TEXT("stop reason survives; later unexecuted damage is never counted"))) return false;
    Result.prediction=true;
    if(!Check(!gunyureview::Build(Result,Names).bAvailable,TEXT("prediction is not an execution record"))) return false;
    const int SavedMission=FirstMission,SavedPolicy=EnemyPolicy;
    const bool SavedCouncil=bStartWithCouncil,SavedRealtime=bRealtimeCommand,SavedNoPlayback=State->bNoPlayback;
    const float SavedSpeed=PlaybackSpeed;
    FirstMission=0;EnemyPolicy=0;bStartWithCouncil=false;bRealtimeCommand=false;State->bNoPlayback=false;PlaybackSpeed=1;
    Issue(ETacticalCommand::Restart);
    if(!Check(!HasChainReview(),TEXT("new battle has no invented record"))) return false;
    Issue(ETacticalCommand::Sample);Issue(ETacticalCommand::Execute);
    if(!Check(!HasChainReview() && ReadChainReview().Rows.IsEmpty(),TEXT("in-flight replay cannot reveal the resolved future"))) return false;
    Issue(ETacticalCommand::Skip);R=ReadChainReview();
    if(!Check(R.Completed==3 && R.Damage==969 && R.Rows.Num()==3,TEXT("public chain yields its completed real review"))) return false;
    std::vector<std::uint8_t> Before,After;
    if(!Check(ExportSession(Before),TEXT("stable completed session"))) return false;
    for(int N=0;N<12;++N) ReadChainReview();
    if(!Check(ExportSession(After) && Before==After,TEXT("review does not spend resources or change plans"))) return false;
    State->Journey.lastChain={};
    if(!Check(!HasChainReview() && ImportSession(Before),TEXT("saved session restores its chain events"))) return false;
    const auto Loaded=ReadChainReview();
    if(!Check(Loaded.Summary==R.Summary && Loaded.Rows[2].Link==R.Rows[2].Link && Loaded.Rows[1].Reason==R.Rows[1].Reason,TEXT("review is rebuilt after load without a new save field"))) return false;
    const auto Real=State->Journey.lastChain;auto Unknown=Real.events[0];Unknown.enemy=4;Unknown.success=false;Unknown.reason=U"対象を確認できない。";
    State->Journey.lastChain.events={Unknown};const auto Hidden=ReadChainReview();State->Journey.lastChain=Real;
    const auto Secret=JourneyText(State->Journey.world.generals[State->Journey.world.armies[4].general].name);
    if(!Check(Hidden.Rows.Num()==1 && Hidden.Rows[0].Subject==TEXT("対象の敵") && !Hidden.Rows[0].Subject.Contains(Secret),TEXT("unknown reserve's actual name is never inspected for the UI"))) return false;
    Issue(ETacticalCommand::Withdraw);Issue(ETacticalCommand::Withdraw);Issue(ETacticalCommand::Skip);
    if(!Check(Presentation().Journey.bDebrief && HasChainReview(),TEXT("debrief can review the last chain"))) return false;
    Issue(ETacticalCommand::Retry);
    if(!Check(!HasChainReview(),TEXT("retry preparation does not masquerade as the previous battle"))) return false;
    FirstMission=0;bStartWithCouncil=false;Issue(ETacticalCommand::Restart);
    State->Journey.world.armies[1].morale=State->Journey.world.armies[2].morale=100;
    SelectOfficer(0);SelectTactic(int(T::Oath));Issue(ETacticalCommand::QueueTactic);Issue(ETacticalCommand::Execute);Issue(ETacticalCommand::Skip);R=ReadChainReview();
    if(!Check(R.Completed==1 && R.Damage==0 && R.Rows[0].Subject==TEXT("周囲の味方") && R.Rows[0].Outcome.Contains(TEXT("2隊")) && !R.Rows[0].Reason.Contains(TEXT("18")),TEXT("capped support records actual state changes without invented healing"))) return false;
    bStartWithCouncil=true;Issue(ETacticalCommand::Restart);
    for(int I=0;I<3;++I) Issue(ETacticalCommand::Dialogue);
    Issue(ETacticalCommand::PromiseRoad);Issue(ETacticalCommand::PlanSupplies);Issue(ETacticalCommand::Deploy);
    const int Dest[]={frontline::Campaign::At(9,11),frontline::Campaign::At(9,9),frontline::Campaign::At(9,10)};
    for(int I=0;I<3;++I)
    {
        SelectOfficer(I);Issue(ETacticalCommand::Move);CommandAt(Dest[I]);Issue(ETacticalCommand::Skip);
        if(!Check(State->Journey.world.armies[I].tile==Dest[I],TEXT("signature fixture reaches its public planned positions"))) return false;
    }
    const int Actors[]={0,2,1};const T Tactics[]={T::Oath,T::SerpentRoar,T::AzureDragon};
    for(int I=0;I<3;++I) { SelectOfficer(Actors[I]);SelectTactic(int(Tactics[I]));Issue(ETacticalCommand::QueueTactic); }
    Issue(ETacticalCommand::Execute);Issue(ETacticalCommand::Skip);R=ReadChainReview();
    if(!Check(R.Completed==3 && R.Damage==1022 && R.Rows[1].Link.Contains(TEXT("第1手")) && R.Rows[2].Link.Contains(TEXT("第1手")) && R.Rows[2].Link.Contains(TEXT("第2手")),TEXT("actual signature chain records both prior disruption and consumed inspiration"))) return false;
    FirstMission=SavedMission;EnemyPolicy=SavedPolicy;bStartWithCouncil=SavedCouncil;bRealtimeCommand=SavedRealtime;State->bNoPlayback=SavedNoPlayback;PlaybackSpeed=SavedSpeed;Issue(ETacticalCommand::Restart);
    UE_LOG(LogTemp,Display,TEXT("GUNYU_CHAIN_REVIEW: PASS (causal links, actual damage, stop, hidden names, playback guard, immutable reads, save reconstruction, debrief/retry, capped support)"));return true;
}
