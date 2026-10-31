#include "TacticalTrial.h"
#include "TacticalState.h"
using namespace frontline;
FReliefApproachRead ATacticalField::ReadReliefApproach(int Officer) const
{
    FReliefApproachRead R;const auto& J=State->Journey;const int Source=ReliefSource();
    if(IsPlaying() || State->Input!=ETacticalInput::Position || Source<0 || Officer<0 || Officer>=3 || Officer==State->Selected || !TacticChain::Living(J.world,Officer)) return R;
    const auto Plan=State->Realtime.ReliefApproach(J,Officer,Source);using S=JourneyApproachState;
    if(Plan.state==S::Unavailable || Plan.state==S::Adjacent) return R;
    R.bVisible=true;R.Officer=Officer;R.Staging=Plan.staging;R.Blocker=Plan.blocker;R.BlockedTile=Plan.blockedTile;
    R.bBlocked=Plan.state==S::AllyBlocked;for(int T:Plan.route) R.Route.Add(T);
    const FString Name=JourneyText(J.world.generals[J.world.armies[Officer].general].name);
    R.ConfirmLabel=Name+(State->Realtime.orders[Officer].kind==JourneyDirective::None ? TEXT("を待機先へ行軍→固守"):TEXT("の前指示を待機・固守へ置換"));
    if(Plan.state==S::Clear)
    {
        R.Summary=FString::Printf(TEXT("仮経路%dマス → 待機(%d,%d)。"),int(Plan.route.size())-1,Plan.staging%Width,Plan.staging/Width);
        R.Summary+=TEXT("\n隣接外。到着と移動権回復を待つ。");
        R.bCanQueue=bRealtimeCommand && !State->bRealtimeStepping && !State->bRealtimeChainRequested;
        for(const auto& O:State->Orders) if(O.army==Officer) R.bCanQueue=false;
    }
    else if(R.bBlocked)
    {
        R.Summary=JourneyText(J.world.generals[J.world.armies[Plan.blocker].general].name)+TEXT("が仮進路を塞いでいる。\n道を空けるか、別の後任を選ぶ。");
        R.ConfirmLabel=TEXT("進路を空けてから行軍指示");
    }
    else { R.Summary=TEXT("地形・捕捉中の敵で仮経路なし。\n守備隊を残し、別の後任を検討。");R.ConfirmLabel=TEXT("待機先への行軍不可"); }
    const int Tile=J.world.armies[Officer].tile;const auto& Duty=J.assignments[Officer];
    if((J.Definition().kind==JourneyObjective::HoldPositions && (Tile==J.goal || Tile==J.Definition().secondGoal)) || (Duty.duty==JourneyDuty::Hold && Duty.tile==Tile))
        R.Warning=TEXT("今の守備地点を空ける。代役を確認。");
    else if(Duty.duty==JourneyDuty::Hold && Duty.tile!=Plan.staging && Duty.tile!=Source)
        R.Warning=TEXT("軍議の守備担当と違う待機先。代役を確認。");
    if(J.world.armies[Officer].troops<150 || J.world.armies[Officer].morale<25) R.Warning+=TEXT(" 兵150・士気25が不足。");
    return R;
}
void ATacticalField::BuildReliefApproach()
{
    auto& R=State->HUD.Position.Relief;if(!R.bVisible) return;
    R.Approach=ReadReliefApproach(R.Incoming);
    if(R.Approach.bVisible)
    {
        const auto* O=R.Officers.FindByPredicate([&R](const auto& In) { return In.Army==R.Incoming; });
        if(O && !O->bReserved) R.Decision=R.Approach.Summary;
        R.Cost=bRealtimeCommand ? TEXT("後任だけの指示・経由地を置換。到着後固守。\n守備隊・他隊・予約・合図は保持。"):
            TEXT("仮進路の閲覧のみ。後任を選び、通常の1歩ずつ移動する。");
    }
}
bool ATacticalField::CanQueueReliefApproach() const
{
    const auto& Shown=State->HUD.Position.Relief.Approach;const auto Current=ReadReliefApproach(State->ComparedRelief);
    return Shown.bVisible && Current.bCanQueue && Shown.Officer==Current.Officer && Shown.Staging==Current.Staging;
}
bool ATacticalField::QueueReliefApproach()
{
    if(!CanQueueReliefApproach()) return false;
    const auto R=ReadReliefApproach(State->ComparedRelief);auto Orders=State->Realtime;
    if(!Orders.Queue(State->Journey,R.Officer,JourneyDirective::March,R.Staging) || !Orders.SetArrival(State->Journey,R.Officer,JourneyArrival::Hold)) return false;
    Orders.paused=true;State->Realtime=std::move(Orders);State->Input=ETacticalInput::Select;
    State->bConfirmEndTurn=State->bConfirmWithdrawal=false;
    State->Report=TEXT("後任だけの指示・経由地を、待機先への行軍と到着後固守へ置換して停止。守備隊・他隊・予約・合図は保持。実移動と交代は別。再開後は敵味方も動き、経路を再判定します。");
    Refresh();return true;
}
