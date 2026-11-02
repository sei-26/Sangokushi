#include "TacticalTrial.h"
#include "TacticalState.h"
using namespace frontline;
void ATacticalField::BuildBattleChoices()
{
    auto& H=State->HUD;const auto& J=State->Journey;
    if(IsPlaying() || J.stage!=JourneyStage::Battle) return;
    if(State->bAutoBattle) { H.BattleChoiceReason=TEXT("皆に任せて進軍中。勝負どころで自分の指揮へ戻ろう。");return; }
    if(!State->Orders.empty() || State->ChainSignal.Armed())
    { H.BattleChoiceReason=TEXT("予約した策がある。詳しい采配で実行・見直しができる。");return; }
    const auto* Seen=J.intelligence[0].Find(State->Target);
    if(!Seen || !Seen->current || Seen->observed.troops<=0)
    { H.BattleChoiceReason=TEXT("見えている敵の札を選ぼう。その相手に打てる策を比べられる。");return; }
    H.BattleChoiceTarget=JourneyText(J.world.generals[Seen->observed.general].name);
    const auto Key=BattleChoiceContext(J,State->Target);
    if(Key!=State->BattleChoiceKey)
    { State->BattleChoiceKey=Key;State->BattleChoices=ReadJourneyBattleChoices(J,State->Target);++State->BattleChoiceVersion; }
    H.BattleChoiceVersion=State->BattleChoiceVersion;
    for(const auto& Plan:State->BattleChoices)
    {
        FTacticalPresentation::FBattleChoice Choice;Choice.Actor=Plan.orders.front().army;
        const FString Name=JourneyText(J.world.generals[J.world.armies[Choice.Actor].general].name);
        Choice.Title=H.BattleChoices.IsEmpty() ? Name+TEXT("に一手を託す"):TEXT("仲間と畳みかける");
        const bool Bait=std::any_of(Plan.orders.begin(),Plan.orders.end(),[](const ChainOrder& O) { return O.tactic==ChainTactic::Lure; });
        if(Bait) Choice.Title=TEXT("誘って、一斉に打つ");
        Choice.Outcome=Plan.damage>=Seen->observed.troops ? TEXT("この敵を撃破できる見込み"):FString::Printf(TEXT("敵兵%dを減らす見込み"),Plan.damage);
        if(Plan.disordered && !J.chain.disordered[Plan.target]) Choice.Outcome+=TEXT("・混乱");
        else if(Plan.pressured && !J.chain.pressured[Plan.target]) Choice.Outcome+=TEXT("・隙を作る");
        if(Plan.friendlyLoss>0) Choice.Outcome+=FString::Printf(TEXT(" / 味方損耗%d"),Plan.friendlyLoss);
        for(const auto& Order:Plan.orders)
        {
            if(!Choice.Detail.IsEmpty()) Choice.Detail+=TEXT(" → ");
            const auto Officer=JourneyText(J.world.generals[J.world.armies[Order.army].general].name);
            if(!Choice.Cost.IsEmpty()) Choice.Cost+=TEXT("・");Choice.Cost+=Officer;
            Choice.Detail+=Officer+TEXT("「")+JourneyText(TacticChain::Name(Order.tactic))+TEXT("」");
            if(Order.tactic==ChainTactic::Lure) Choice.Detail+=FString::Printf(TEXT("（%d,%dへ後退）"),Order.destination%Width,Order.destination/Width);
        }
        Choice.Cost+=FString::Printf(TEXT(" / 士気計%d"),Plan.moraleCost);
        Choice.Detail+=TEXT("\n参加する隊の行動と士気を使って、今すぐ発動。参加しない隊の行動は使わない。捕捉からの見込みで、未知の状況では停止することもある。");
        H.BattleChoices.Add(MoveTemp(Choice));
    }
    if(H.BattleChoices.IsEmpty())
    {
        bool Unspent=false,Ready=false;
        for(int A=0;A<3;++A) if(TacticChain::Living(J.world,A) && !J.chain.spent[A])
        { Unspent=true;Ready=Ready || (J.world.armies[A].troops>=150 && J.world.armies[A].morale>=15); }
        H.BattleChoiceReason=!Unspent ? TEXT("今の攻撃は使い切った。次の指揮へ進もう。"):!Ready ? TEXT("兵・士気が足りない。次の指揮や補給を考えよう。"):
            TEXT("今の位置から届く策はない。部隊を寄せるか、皆に任せて進めよう。");
        if(J.Definition().kind==JourneyObjective::HoldPositions && Unspent && Ready)
            H.BattleChoiceReason=TEXT("烽火の守備役は残す。別の隊で援護するか、守備を固めよう。");
    }
}
bool ATacticalField::UseBattleChoice(int Index,int ExpectedVersion)
{
    if(IsPlaying() || State->Journey.stage!=JourneyStage::Battle || State->bAutoBattle || !State->Orders.empty() || State->ChainSignal.Armed()) return false;
    if(ExpectedVersion!=State->BattleChoiceVersion || Index<0 || Index>=int(State->BattleChoices.size()) || BattleChoiceContext(State->Journey,State->Target)!=State->BattleChoiceKey)
    { State->Report=TEXT("戦況が変わった。今の提案を確認してから、もう一度選ぼう。");Refresh();return false; }
    const auto Plan=State->BattleChoices[Index];
    const auto Check=State->Journey.Preview(Plan.orders);
    if(Check.completed!=int(Plan.orders.size())) { Refresh();return false; }
    State->Selected=Plan.orders.front().army;State->Orders=Plan.orders;
    State->Input=ETacticalInput::Select;State->bConfirmEndTurn=State->bConfirmWithdrawal=false;
    // A direct decision plays immediately even when the planning clock is paused.
    // Preserve standing movement and all normal action budgets; finish paused again.
    const bool WasPaused=bRealtimeCommand && State->Realtime.paused;
    if(WasPaused) { State->bRealtimeStepping=true;State->bRealtimePauseAfterPlayback=true; }
    DispatchQueuedChain();
    if(!IsPlaying() && WasPaused) { State->bRealtimeStepping=false;State->bRealtimePauseAfterPlayback=false;State->Realtime.paused=true; }
    Refresh();return true;
}
