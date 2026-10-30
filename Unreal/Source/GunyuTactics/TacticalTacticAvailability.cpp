#include "TacticalTrial.h"
#include "TacticalState.h"
using namespace frontline;

FString ATacticalField::TacticReservationBlockReason() const
{
    if(IsPlaying()) return TEXT("行動を再生中。演出を終えると命令できます。");
    const auto& J=State->Journey;const int Id=State->Selected;
    if(J.stage!=JourneyStage::Battle) return TEXT("戦闘中に戦法を予約できます。");
    if(Id<0 || Id>=3 || !TacticChain::Living(J.world,Id)) return TEXT("生存している味方武将を選んでください。");
    if(J.chain.spent[Id] && J.guarding[Id])
    {
        if(!bRealtimeCommand) return TEXT("固守で行動済み。Spaceで次の指揮へ。");
        return HasGuardOrder() ? TEXT("固守で行動済み。継続を解除し『1周期』で次の指揮へ。"):
            TEXT("固守で行動済み。停止中の『1周期』で次の指揮へ。");
    }
    if(J.chain.spent[Id]) return bRealtimeCommand ? TEXT("この指揮は行動済み。一時停止→「1周期」で回復。") : TEXT("この指揮は行動済み。Spaceで次の指揮へ。");
    const auto Tactic=State->SelectedTactics[Id];
    if(!TacticChain::CanUse(J.world,Id,Tactic)) return TEXT("この武将が使える戦法を選んでください。");
    for(int I=0;I<int(State->Orders.size());++I)
        if(State->Orders[I].army==Id) return FString::Printf(TEXT("第%d手に予約済み。変更は予約欄の×から。"),I+1);
    if(State->Orders.size()>=3) return TEXT("予約は最大3手。不要な手を×で外してください。");
    const auto& Definition=TacticChain::Definition(Tactic);
    if(Definition.unique && J.chain.signatureUsed[Id]) return TEXT("固有戦法は使用済み。戦法▼から通常戦法へ切替。");
    if(Tactic==ChainTactic::Lure && J.chain.moved[Id]) return TEXT("移動済みで誘導できません。別の戦法か次の指揮へ。");
    if(!Definition.targetsEnemy) return FString();
    const auto* Contact=J.intelligence[0].Find(State->Target);
    if(!Contact || !Contact->current || Contact->observed.troops<=0)
        return J.HasConvoy() ? TEXT("確認中の敵なし。護送路を空け、指揮を進めよう。") : TEXT("確認中の敵なし。見えている敵を指定・索敵しよう。");
    return FString();
}
void ATacticalField::BuildTacticAvailability()
{
    auto& H=State->HUD;H.TacticAvailability=TacticReservationBlockReason();
    H.bTacticUnavailable=!H.TacticAvailability.IsEmpty();
    if(H.bTacticUnavailable) return;
    const auto& J=State->Journey;const auto Tactic=State->SelectedTactics[State->Selected];
    if(Tactic==ChainTactic::Lure)
    { H.TacticAvailability=TEXT("予約から後退先を指定。固守する敵は誘えません。");return; }
    auto Orders=State->Orders;
    Orders.push_back({State->Selected,TacticChain::Definition(Tactic).targetsEnemy ? State->Target:-1,Tactic,-1});
    const auto Preview=J.Preview(Orders);
    if(Preview.completed==int(Orders.size())) H.TacticAvailability=Orders.size()==1 ? TEXT("単独で発動できます。予約 → 発動 / C。") : TEXT("連鎖に追加できます。予約欄で順番を確認。");
    else
    {
        H.bTacticUnavailable=true;
        H.TacticAvailability=Preview.events.empty() ? TEXT("予約欄で成立条件を確認してください。") : JourneyText(Preview.events.back().reason);
    }
}
void ATacticalField::RetargetObservedEnemy()
{
    if(IsPlaying() || State->Journey.stage!=JourneyStage::Battle) return;
    const auto& Known=State->Journey.intelligence[0];const auto* Current=Known.Find(State->Target);
    if(Current && Current->current && Current->observed.troops>0) return;
    // Only target a live observed contact. Hidden army slots never authorize selection.
    State->Target=-1;
    for(const auto& Contact:Known.contacts)
        if(Contact.current && Contact.observed.troops>0 && State->Journey.world.Hostile(State->Journey.world.player,Contact.observed.faction))
        { State->Target=Contact.id;break; }
}
