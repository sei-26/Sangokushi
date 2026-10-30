#include "TacticalTrial.h"
#include "TacticalState.h"

using namespace frontline;

void ATacticalField::BuildTacticsPresentation()
{
    const auto& J = State->Journey;
    auto& H = State->HUD;
    const auto Selected = State->SelectedTactics[State->Selected];
    const auto& Definition = TacticChain::Definition(Selected);
    H.SelectedTacticName = JourneyText(Definition.name);
    BuildGuardPresentation();
    BuildTacticTargets();
    BuildTacticAvailability();
    H.Hint = JourneyText(Definition.description);
    if (!Definition.targetsEnemy) H.Target = TEXT("対象: 自隊と周囲2マスの味方\n荷車を除く / 敵の指定は不要");
    for (const auto Tactic : TacticChain::Loadout(J.world, State->Selected))
    {
        const auto& Info = TacticChain::Definition(Tactic);
        FTacticOptionPresentation Option;
        Option.Id = static_cast<int>(Tactic); Option.Name = JourneyText(Info.name);
        Option.Description = JourneyText(Info.description);
        Option.bSelected = Tactic == Selected; Option.bUnique = Info.unique;
        Option.bUsed = Info.unique && J.chain.signatureUsed[State->Selected];
        Option.Cost = FString::Printf(TEXT("消費%d / 必要士気%d・兵150%s"), Info.moraleCost, FMath::Max(15, Info.moraleCost),
            Info.unique ? (Option.bUsed ? TEXT(" / 使用済") : TEXT(" / 1戦1回")) : TEXT(""));
        H.Tactics.Add(MoveTemp(Option));
    }
    if (Definition.unique && J.chain.signatureUsed[State->Selected]) H.Hint = TEXT("この固有戦法は使用済み。通常戦法を選ぼう。固有の使用回数は次の戦闘で回復する。");
    else if (J.chain.spent[State->Selected]) H.Hint = TEXT("この指揮の戦法・攻撃は使用済み。戦法の選択は変更できる。未移動なら移動も可能。");
}

void ATacticalField::SelectTactic(int Id)
{
    if (IsPlaying() || State->Journey.stage != JourneyStage::Battle) return;
    const auto Tactic = static_cast<ChainTactic>(Id);
    if (!TacticChain::CanUse(State->Journey.world, State->Selected, Tactic)) return;
    State->SelectedTactics[State->Selected] = Tactic;
    State->Input = ETacticalInput::Select;
    State->bConfirmEndTurn = State->bConfirmWithdrawal = false;
    State->Report = JourneyText(TacticChain::Name(Tactic)) + TEXT("を選択。選ぶだけでは行動・士気・固有戦法を消費しません。予約して予測を確認できます。");
    for (const auto& Order : State->Orders) if (Order.army == State->Selected)
        State->Report = JourneyText(TacticChain::Name(Tactic)) + TEXT("を選択。この武将は") + JourneyText(TacticChain::Name(Order.tactic)) + TEXT("を予約済み。予約欄でその手を取り消してから、新しい戦法を予約してください。");
    Refresh();
}


void ATacticalField::DispatchQueuedChain()
{
    auto& J=State->Journey;
    const auto Before=ObserveUnits();
    const auto WatchBefore=bRealtimeCommand ? JourneyBattleWatch::Capture(J,State->Realtime) : JourneyWatchSnapshot{};
    const int Requested=static_cast<int>(State->Orders.size());
        const auto Result = J.Execute(State->Orders);
        State->Report.Reset();
        for (int Index = 0; Index < static_cast<int>(Result.events.size()); ++Index)
        {
            const auto& Event = Result.events[Index];
            State->Report += FString::Printf(TEXT("%d %s%s: %s  "), Index + 1, *JourneyText(TacticChain::Name(Event.tactic)), Event.success ? TEXT("成功") : TEXT("停止"), *JourneyText(Event.reason));
        }
        QueueChain(Before);
        RecordBattleWatch(WatchBefore);
        State->Orders.erase(State->Orders.begin(), State->Orders.begin() + Result.completed);
        if (J.stage != JourneyStage::Battle) State->Orders.clear();
        State->Input = ETacticalInput::Select;

    if (bRealtimeCommand && Result.completed<Requested)
    {
        if (IsPlaying()) State->bRealtimePauseAfterPlayback=true;
        else State->Realtime.paused=true;
    }
}
