#include "TacticalTrial.h"
#include "TacticalState.h"
using namespace frontline;

bool ATacticalField::RealtimePaused() const
{ return State->Realtime.paused && !State->bRealtimeStepping; }

void ATacticalField::SyncRealtime()
{
    if (IsPlaying()) return; // Keep route previews at the displayed observation until playback finishes.
    const bool Active=bRealtimeCommand && State->Journey.stage==JourneyStage::Battle;
    if (Active && !State->bRealtimeInBattle)
    {
        State->Realtime.Reset();State->bRealtimeStepping=false;
        State->bRealtimeChainRequested=false;State->bRealtimePauseAfterPlayback=false;
        State->BattleWatch.Reset();State->bWatchPending=State->bWatchPauseAfterPlayback=false;
    }
    State->bRealtimeInBattle=Active;
    State->RealtimeRoutes.Reset();
    if (!Active)
    {
        if (!IsPlaying()) State->Realtime.paused=true;
        return;
    }
    for (int Army=0;Army<3;++Army)
    {
        State->OrderReads[Army]=ReadJourneyOrder(State->Journey,State->Realtime,Army);
        const auto& Route=State->OrderReads[Army].route;
        for (int I=1;I<static_cast<int>(Route.size());++I) State->RealtimeRoutes.Add({Route[I-1],Route[I]});
    }
}

bool ATacticalField::IsRealtimeAction(ETacticalCommand Command) const
{
    if (Command==ETacticalCommand::RealtimeMode || Command==ETacticalCommand::RealtimePause ||
        Command==ETacticalCommand::RealtimeSpeed || Command==ETacticalCommand::RealtimeStep || Command==ETacticalCommand::RealtimeStop ||
        Command==ETacticalCommand::RealtimeChainSignal || Command==ETacticalCommand::RealtimeAlertPause ||
        Command==ETacticalCommand::RealtimeUndoWaypoint || Command==ETacticalCommand::RealtimeArrival) return true;
    return bRealtimeCommand && State->Journey.stage==JourneyStage::Battle &&
        (Command==ETacticalCommand::Move || Command==ETacticalCommand::Strike || Command==ETacticalCommand::Hold ||
         Command==ETacticalCommand::PrepareAmbush || Command==ETacticalCommand::Execute || Command==ETacticalCommand::EndTurn);
}
bool ATacticalField::CanRealtimeIssue(ETacticalCommand Command) const
{
    const bool Battle=State->Journey.stage==JourneyStage::Battle;
    if (Command==ETacticalCommand::RealtimeMode) return Battle && !IsPlaying();
    if (!bRealtimeCommand || (!Battle && !IsPlaying())) return false;
    if (Command==ETacticalCommand::RealtimeUndoWaypoint || Command==ETacticalCommand::RealtimeArrival)
        return Battle && !IsPlaying() && State->Realtime.orders[State->Selected].kind==JourneyDirective::March;
    if (Command==ETacticalCommand::RealtimeAlertPause) return true;
    if (Command==ETacticalCommand::RealtimeChainSignal) return Battle && (State->ChainSignal.Armed() || (!IsPlaying() && !State->Orders.empty()));
    if (Command==ETacticalCommand::RealtimePause || Command==ETacticalCommand::RealtimeSpeed) return true;
    if (Command==ETacticalCommand::RealtimeStep) return Battle && RealtimePaused() && !IsPlaying();
    if (Command==ETacticalCommand::Execute) return !IsPlaying() && !State->Orders.empty();
    if (Command==ETacticalCommand::EndTurn) return false;
    const int Id=State->Selected;
    if (!Battle || Id<0 || Id>=3 || !TacticChain::Living(State->Journey.world,Id)) return false;
    if (Command==ETacticalCommand::RealtimeStop) return State->Realtime.orders[Id].kind!=JourneyDirective::None;
    if (Command==ETacticalCommand::PrepareAmbush) return State->Journey.world.generals[State->Journey.world.armies[Id].general].tactic==Tactic::Ambush;
    if (Command==ETacticalCommand::Strike)
    {
        const auto* Seen=State->Journey.intelligence[0].Find(State->Target);
        return Seen && Seen->current && Seen->observed.troops>0;
    }
    return Command==ETacticalCommand::Move || Command==ETacticalCommand::Hold;
}
void ATacticalField::IssueRealtime(ETacticalCommand Command)
{
    auto& RT=State->Realtime;
    switch (Command)
    {
    case ETacticalCommand::RealtimeMode:
        bRealtimeCommand=!bRealtimeCommand;State->bRealtimeInBattle=false;RT.Reset();
        State->ChainSignal.Cancel();State->SignalRead={};
        State->BattleWatch.Reset();State->bWatchPending=State->bWatchPauseAfterPlayback=false;
        State->bRealtimeChainRequested=false;State->Input=ETacticalInput::Select;
        State->Report=bRealtimeCommand ? TEXT("戦術ポーズ付きの同時進行。停止中に行軍・攻撃・固守を指示し、Spaceで作戦開始。")
            : TEXT("手動指揮に戻した。継続行軍の指示は解除。戦法の予約と戦況は引き継ぐ。");
        break;
    case ETacticalCommand::RealtimePause:
        if (State->bRealtimeStepping) { State->bRealtimeStepping=false;RT.paused=true; }
        else RT.paused=!RT.paused;
        break;
    case ETacticalCommand::RealtimeSpeed:
        RT.speed=RT.speed<.75 ? 1. : RT.speed<1.5 ? 2. : .5;break;
    case ETacticalCommand::RealtimeAlertPause:
        bPauseOnBattleAlert=!bPauseOnBattleAlert;
        State->Report=bPauseOnBattleAlert ? TEXT("危険時停止をON。新たな捕捉・損耗・護送車の被害・停滞を報告し、演出後に停止する。") :
            TEXT("危険時停止をOFF。報告は記録し、作戦を続ける。合図の発動後・失効時の停止は維持する。");break;
    case ETacticalCommand::RealtimeStep:
        RunRealtimeCycle(true);return;
    case ETacticalCommand::RealtimeChainSignal:
        IssueChainSignal();return;
    case ETacticalCommand::RealtimeUndoWaypoint:
    case ETacticalCommand::RealtimeArrival:
        IssueMarchPlan(Command);return;
    case ETacticalCommand::RealtimeStop:
        RT.Cancel(State->Selected);State->Report=TEXT("選択部隊の継続指示を解除。今の動きが終わった位置で待機する。");break;
    case ETacticalCommand::Move:
        RT.paused=true;State->bRealtimeStepping=false;State->Input=ETacticalInput::Move;
        State->Report=TEXT("指揮停止中。遠くの目的地も指定できる。川は橋を使い、部隊は進路を見直しながら行軍する。");break;
    case ETacticalCommand::Execute:
        State->ChainSignal.Cancel();State->SignalRead={};
        State->bRealtimeChainRequested=true;
        State->Report=RealtimePaused() ? TEXT("連鎖を発動待ちにした。Spaceで再開すると発動。") : TEXT("現在の動きが終わり次第、予約した連鎖を発動する。");break;
    default:
    {
        const auto Kind=Command==ETacticalCommand::Hold ? JourneyDirective::Hold :
            Command==ETacticalCommand::PrepareAmbush ? JourneyDirective::Ambush : JourneyDirective::Attack;
        if (RT.Queue(State->Journey,State->Selected,Kind,State->Target))
            State->Report=Kind==JourneyDirective::Hold ? TEXT("固守を継続。移動を止め、次の更新から防御を優先する。") :
                Kind==JourneyDirective::Ambush ? TEXT("伏兵準備を指示。森林と敵の接近状況を見て準備を試みる。") : TEXT("見えている敵へ接近して攻撃を継続。対象を見失うと再捕捉を待つ。");
        State->Input=ETacticalInput::Select;break;
    }
    }
    State->bConfirmEndTurn=false;State->bConfirmWithdrawal=false;Refresh();
}
void ATacticalField::RunRealtimeCycle(bool bSingleStep)
{
    if (IsPlaying() || State->Journey.stage!=JourneyStage::Battle) return;
    const auto Before=ObserveUnits();
    const auto WatchBefore=JourneyBattleWatch::Capture(State->Journey,State->Realtime);
    if (!State->Realtime.Resolve(State->Journey,State->ChainSignal.ReservedAttacks())) return;
    State->Realtime.Consume(bSingleStep);
    State->bRealtimeStepping=bSingleStep;
    const auto After=ObserveUnits();
    int Moved=0,Waiting=0;
    for (int Id=0;Id<3;++Id)
    {
        if (Before[Id].Id<0 || After[Id].Id<0) continue;
        if (FVector::DistSquared2D(Before[Id].Position,After[Id].Position)>1) ++Moved;
        else if (State->Realtime.orders[Id].kind==JourneyDirective::March) ++Waiting;
    }
    State->Report=FString::Printf(TEXT("味方の行軍 %d / 進路待ち %d。交戦・補給・護送を更新。 "),Moved,Waiting)+JourneyText(State->Journey.message);
    QueueAction(EBattleCue::Enemy,-1,-1,Before);
    RecordBattleWatch(WatchBefore);
    if (IsPlaying()) State->Beats[0].Title=TEXT("全軍進行 / 移動と交戦");
    else State->bRealtimeStepping=false;
    Refresh();
}
void ATacticalField::AdvanceRealtime(float Delta)
{
    if (!bRealtimeCommand) return;
    if (State->Journey.stage!=JourneyStage::Battle)
    { if (!IsPlaying()) State->Realtime.paused=true;UpdateRealtimePresentation();return; }
    State->Realtime.Advance(Delta);
    if (!RealtimePaused() && !IsPlaying())
    {
        if (State->bRealtimeChainRequested)
        {
            State->bRealtimeChainRequested=false;
            if (!State->Orders.empty()) { DispatchQueuedChain();Refresh(); }
        }
        else if (DispatchReadySignal()) { }
        else if (State->Realtime.Due()) RunRealtimeCycle(false);
    }
    UpdateRealtimePresentation();
}
void ATacticalField::UpdateRealtimePresentation()
{
    auto& H=State->HUD;const auto& RT=State->Realtime;
    H.bRealtime=bRealtimeCommand;H.bRealtimePaused=RealtimePaused();
    H.bAlertPause=bPauseOnBattleAlert;
    H.bChainSignalArmed=State->ChainSignal.Armed();
    H.bChainSignalReady=H.bChainSignalArmed && !IsPlaying() && State->SignalRead.state==ChainSignalState::Ready && !SignalHasPendingMarch();
    if (!H.bChainSignalArmed) H.ChainSignalLabel=TEXT("予約を組んで合図を設定。\n全手成立で一度だけ発動。");
    else if (IsPlaying()) H.ChainSignalLabel=TEXT("合図待ち / 動きを確認中\n連鎖の判定は演出完了後。");
    else if (State->SignalRead.state==ChainSignalState::Ready && SignalHasPendingMarch()) H.ChainSignalLabel=TEXT("成立見込み / 行軍待ち\n到着後に全手を再判定。");
    else if (H.bChainSignalReady) H.ChainSignalLabel=H.bRealtimePaused ? TEXT("全手成立 / 再開で発動\nBで解除・Cで手動指揮。"): TEXT("全手成立 / 合図を送る");
    else H.ChainSignalLabel=FString::Printf(TEXT("%d/%d手成立 / %d手目を待つ\n未成立の条件は右の予約欄。"),State->SignalRead.completed,State->SignalRead.total,State->SignalRead.completed+1);
    H.ClockProgress=FMath::Clamp(static_cast<float>(RT.elapsed/JourneyRealtime::Period),0.f,1.f);
    H.SpeedLabel=FString::Printf(TEXT("%.1f×"),RT.speed);
    H.ClockLabel=State->bRealtimeStepping ? TEXT("１周期を進行中") :
        H.bRealtimePaused ? TEXT("指揮停止中 / 命令受付") :
        FString::Printf(TEXT("進行中 / 次 %.1f秒"),FMath::Max(0.,JourneyRealtime::Period-RT.elapsed));
    if (!bRealtimeCommand) return;
    H.Selected=State->Selected;
    for (int Id=0;Id<H.Officers.Num();++Id) H.Officers[Id].bSelected=Id==State->Selected;
    const auto Kind=RT.orders[State->Selected].kind;
    H.StandingOrderLabel=Kind==JourneyDirective::March ? TEXT("行軍 / 到着で停止") : Kind==JourneyDirective::Attack ? TEXT("追撃 / 接近して攻撃") :
        Kind==JourneyDirective::Hold ? TEXT("固守を継続") : Kind==JourneyDirective::Ambush ? TEXT("伏兵準備を継続") : TEXT("継続指示なし / 待機");
    if (Kind==JourneyDirective::Attack)
    {
        const auto* Contact=State->Journey.intelligence[0].Find(RT.orders[State->Selected].target);
        if (!Contact || !Contact->current) H.StandingOrderLabel=TEXT("見失った / 再捕捉待ち");
        else if (H.bChainSignalArmed && State->ChainSignal.ReservedAttacks()[State->Selected]) H.StandingOrderLabel=TEXT("追撃 / 追加攻撃は温存");
    }
    if (!IsPlaying())
    {
        const auto& Read=State->OrderReads[State->Selected];
        if (Read.status==JourneyRouteStatus::Moving) H.StandingOrderLabel=FString::Printf(TEXT("%s / 残り%dマス%s"),Kind==JourneyDirective::Attack ? TEXT("追撃") : TEXT("行軍"),static_cast<int>(Read.route.size())-1,Read.complete ? TEXT("") : TEXT("以上"));
        else if (Read.status==JourneyRouteStatus::NoRoute) H.StandingOrderLabel=TEXT("進路なし / 配置を見直す");
        else if (Read.status==JourneyRouteStatus::Engaging && !(H.bChainSignalArmed && State->ChainSignal.ReservedAttacks()[State->Selected])) H.StandingOrderLabel=TEXT("攻撃圏内 / 交戦を継続");
    }
    if (State->Input==ETacticalInput::Move) H.Hint=TEXT("停止中。行軍の目的地をクリック。敵を右クリックで追撃。");
    else if (IsPlaying()) H.Hint=TEXT("行軍先・固守は次の更新から。経由地・連鎖の編集は演出完了後。");
    if (H.bBattle) H.NextLabel=H.bRealtimePaused ? TEXT("作戦を再開 / Space") : TEXT("戦術ポーズ / Space");
}
