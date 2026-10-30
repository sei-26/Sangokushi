#include "TacticalTrial.h"
#include "TacticalState.h"
using namespace frontline;

bool ATacticalField::HasGuardOrder() const
{
    const auto& J=State->Journey;const int Id=State->Selected;
    if(!bRealtimeCommand || J.stage!=JourneyStage::Battle || Id<0 || Id>=3 || !TacticChain::Living(J.world,Id)) return false;
    const auto& Order=State->Realtime.orders[Id];
    return Order.kind==JourneyDirective::Hold || (Order.kind==JourneyDirective::March && Order.arrival==JourneyArrival::Hold);
}
void ATacticalField::ReleaseGuardOrder()
{
    if(IsPlaying() || !HasGuardOrder()) return;
    const int Id=State->Selected;
    State->Realtime.Cancel(Id);State->Realtime.paused=true;State->bRealtimeStepping=false;
    State->Input=ETacticalInput::Select;State->bConfirmEndTurn=State->bConfirmWithdrawal=false;
    State->Report=State->Journey.chain.spent[Id] ? TEXT("固守継続を解除し停止。行動済みは次の指揮まで続く。『1周期』では敵味方も進む。"):
        TEXT("固守継続を解除して停止。行軍中なら行軍も終了。射程・士気・対象を見て戦法を予約しよう。");
    Refresh(); // No world action, observation, clock tick, reservation or signature reset.
}
void ATacticalField::BuildGuardPresentation()
{
    auto& H=State->HUD;const auto& J=State->Journey;const int Id=State->Selected;
    H.GuardButtonLabel=TEXT("固守");H.bGuardOrder=HasGuardOrder();
    if(J.stage!=JourneyStage::Battle || Id<0 || Id>=3 || !TacticChain::Living(J.world,Id)) return;
    H.GuardHint=TEXT("固守は戦法・攻撃の行動権を使う。解除は次の固守指示を止める操作で、使った行動権・移動・固有戦法の回数は戻らない。1周期では敵味方の行動・交戦・任務も進む。");
    const bool ActualGuard=J.guarding[Id];
    if(!H.bGuardOrder && !ActualGuard) return;
    const bool March=H.bGuardOrder && State->Realtime.orders[Id].kind==JourneyDirective::March;
    H.GuardButtonLabel=H.bGuardOrder ? (March ? TEXT("行軍・固守解除"):TEXT("固守を解除")):TEXT("固守済み");
    FString Consequence=TEXT("継続解除は次の固守を止める操作。");
    if(ActualGuard)
    {
        H.GuardContext=bRealtimeCommand ? TEXT("固守で行動済み。1周期で次の指揮へ。"):
            TEXT("固守で行動済み。Spaceで次の指揮へ。");
        Consequence=TEXT("今の行動権は解除では戻らない。");
    }
    else if(March) { H.GuardContext=TEXT("到着後に固守する行軍指示。");Consequence=TEXT("解除すると行軍も停止する。"); }
    else if(J.chain.spent[Id]) H.GuardContext=TEXT("行動済み。回復は次の指揮。");
    else H.GuardContext=TEXT("固守継続／行動権は未使用。");
    if(March) H.GuardHint+=TEXT(" 行軍と経由地も取り消し、今の位置に留まる。");
    const int Tile=J.world.armies[Id].tile;
    if(J.Definition().kind==JourneyObjective::HoldPositions && (Tile==J.goal || Tile==J.Definition().secondGoal))
    {
        Consequence=TEXT("烽火の守備。切替時は代役を確認。");
        H.GuardHint+=TEXT(" 烽火の守備担当。敵が隣接する時に固守や兵・士気の条件を満たさないと維持が切れる。解除だけで拠点を離れるわけではない。");
    }
    else if(J.HasConvoy()) H.GuardHint+=TEXT(" 攻める前に、車の護衛と退路を守る隊を確認しよう。");
    H.GuardContext+=TEXT("\n")+Consequence;
}
