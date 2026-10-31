#include "TacticalTrial.h"
#include "TacticalState.h"
using namespace frontline;
FString ATacticalField::ReliefDirective(int Officer) const
{
    if(!bRealtimeCommand) return TEXT("手動指揮");
    const auto& O=State->Realtime.orders[Officer];
    if(O.kind==JourneyDirective::Hold) return TEXT("固守を継続");
    if(O.kind==JourneyDirective::Ambush) return TEXT("伏兵の準備");
    if(O.kind==JourneyDirective::Attack) return TEXT("追撃・攻撃");
    if(O.kind!=JourneyDirective::March) return TEXT("継続指示なし");
    FString Text=FString::Printf(TEXT("行軍(%d,%d)"),O.destination%Width,O.destination/Width);
    if(!O.waypoints.empty()) Text+=FString::Printf(TEXT("・経由%d"),int(O.waypoints.size()));
    return Text+(O.arrival==JourneyArrival::Hold ? TEXT("→固守"):O.arrival==JourneyArrival::Ambush ? TEXT("→伏兵"):TEXT("→待機"));
}
void ATacticalField::FillReliefReadiness(FReliefOfficerRead& Read) const
{
    const auto& J=State->Journey;const int Id=Read.Army,Source=ReliefSource();
    const auto& A=J.world.armies[Id];Read.bAdjacent=J.world.MapDistance(A.tile,Source)==1;Read.bMoveReady=!J.chain.moved[Id];
    for(const auto& O:State->Orders) if(O.army==Id) Read.bReserved=true;
    Read.Status=Read.bAdjacent ? (Read.bMoveReady ? TEXT("隣接・移動可"):TEXT("隣接・移動済")):TEXT("まだ隣接外");
    Read.Readiness=(Read.bAdjacent ? FString(TEXT("隣接済")):FString(TEXT("隣接外")))+TEXT(" / ")+(Read.bMoveReady ? TEXT("移動未使用"):TEXT("移動済"));
    Read.Readiness+=FString::Printf(TEXT("\n兵%d・士気%d / "),A.troops,A.morale);
    Read.Readiness+=J.chain.spent[Id] ? TEXT("戦法・攻撃は使用済"):TEXT("戦法・攻撃は未使用");
    Read.Directive=ReliefDirective(Id);
}
void ATacticalField::BuildReliefReadiness()
{
    auto& R=State->HUD.Position.Relief;const auto& J=State->Journey;const int Out=State->Selected;
    if(!R.bVisible) return;
    // Current guard is also read from the player's contacts; never reveal an unseen actual enemy.
    auto View=std::make_unique<StrategistJourney>(J);
    for(int I=0;I<int(View->world.armies.size());++I) if(J.world.Hostile(J.world.player,View->world.armies[I].faction))
    { const auto* Seen=J.intelligence[0].Find(I);View->world.armies[I]=Seen && Seen->current ? Seen->observed:Army{}; }
    R.CurrentGuard=JourneyText(J.world.generals[J.world.armies[Out].general].name)+TEXT(" / 守備：")+
        (View->PositionSecured(R.Source) ? TEXT("条件充足（捕捉情報）"):TEXT("条件不足（捕捉情報）"));
    R.CurrentGuard+=TEXT("\n今の指示：")+ReliefDirective(Out);
    R.bCanStopDeparture=CanStopReliefDeparture();
    const auto& OutOrder=State->Realtime.orders[Out];
    if(R.bCanStopDeparture && (OutOrder.kind==JourneyDirective::Attack || OutOrder.destination!=R.Source || !OutOrder.waypoints.empty()))
        R.CurrentGuard+=TEXT("\nこの指示で再開すると、離れる可能性あり。");
    const auto* In=R.Officers.FindByPredicate([&R](const auto& O) { return O.Army==R.Incoming; });
    R.Decision=TEXT("後任を選び、準備を確認しよう。");if(!In) return;
    R.IncomingReadiness=In->Name+TEXT(" / ")+(In->Readiness.IsEmpty() ? FString(TEXT("戦闘不能")):In->Readiness+TEXT("\n今の指示：")+In->Directive);
    if(!TacticChain::Living(J.world,In->Army)) R.Decision=TEXT("この後任は戦闘不能。別の隊を選ぶ。");
    else if(In->bReserved) R.Decision=TEXT("後任は戦法予約中。先に予約を見直す。");
    else if(!In->bAdjacent) R.Decision=TEXT("まだ隣接外。後任を隣へ動かして待機させる。");
    else if(J.chain.moved[Out]) R.Decision=TEXT("守備隊は移動済。次の指揮で再確認。");
    else if(!In->bMoveReady) R.Decision=bRealtimeCommand ? TEXT("後任は移動済。1周期後に再確認。敵味方も進む。"):TEXT("後任は移動済。Space後に再確認。敵味方も進む。");
    else if(!StrategistJourney::OnMap(State->ComparedPosition)) R.Decision=TEXT("隣接と移動権は準備済。前進先の番号を選ぶ。");
    else if(!R.bCanIssue) R.Decision=TEXT("この前進先では交代不可。移動先と条件を見直す。");
    else if(bRealtimeCommand) R.Decision=TEXT("交代は最短2周期・空白1周期。\n連続保持を優先するなら見送る。");
    else R.Decision=R.bSecured ? TEXT("交代可・守備の維持見込み。次の更新で確認。"):TEXT("交代可・守備条件に注意。先に条件を整える。");
}
bool ATacticalField::CanStopReliefDeparture() const
{
    if(IsPlaying() || !bRealtimeCommand || State->Input!=ETacticalInput::Position || State->bRealtimeStepping || State->bRealtimeChainRequested || ReliefSource()<0) return false;
    const auto Kind=State->Realtime.orders[State->Selected].kind;
    return Kind==JourneyDirective::March || Kind==JourneyDirective::Attack;
}
void ATacticalField::StopReliefDeparture()
{
    if(!CanStopReliefDeparture()) return;
    State->Realtime.Cancel(State->Selected);State->Realtime.paused=true;State->bRealtimeStepping=false;
    State->Input=ETacticalInput::Select;State->bConfirmEndTurn=State->bConfirmWithdrawal=false;
    State->Report=TEXT("守備隊の行軍・追撃だけを中止して停止。経由地も解除。後任・他隊・戦法予約・合図は保持。固守は自動で行わず、使った移動や行動も戻りません。");
    Refresh();
}
