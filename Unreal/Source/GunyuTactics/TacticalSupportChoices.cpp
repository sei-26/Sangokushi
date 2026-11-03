#include "TacticalTrial.h"
#include "TacticalState.h"
using namespace frontline;
void ATacticalField::BuildSupportChoices()
{
    auto& H=State->HUD;const auto& J=State->Journey;const int Actor=State->Selected;
    if(J.stage!=JourneyStage::Battle || IsPlaying() || State->bAutoBattle || State->bRealtimeStepping || State->bRealtimeChainRequested) return;
    for(const auto& O:State->Orders) if(O.army==Actor) return;
    auto Key=JourneySupportContext(J,State->Realtime,Actor);Key.push_back(bRealtimeCommand);
    if(Key!=State->SupportKey)
    { State->SupportKey=Key;State->SupportChoices=ReadJourneySupport(J,State->Realtime,Actor);++State->SupportVersion; }
    H.SupportVersion=State->SupportVersion;
    for(const auto& C:State->SupportChoices)
    {
        FSupportChoicePresentation P;P.Actor=Actor;P.Goal=C.goal;P.bEnabled=C.available;
        const FString Side=C.goal==J.goal ? TEXT("北の烽火"):TEXT("南の烽火");
        P.Title=Side+(C.cover ? TEXT("を援護"):TEXT("を守りに行く"));
        P.Outcome=C.cover ? JourneyText(J.world.generals[J.world.armies[C.guard].general].name)+TEXT("を横から支える"):TEXT("空いている烽火の守備へ");
        P.Outcome+=FString::Printf(TEXT(" / 周辺の敵%d"),C.enemies);
        P.Cost=FString::Printf(TEXT("最短%d指揮で到着・固守は%d指揮目以降"),C.arrival,C.brace);
        if(C.steps==0 && C.available) P.Cost=FString::Printf(TEXT("到着済み / 固守は%d指揮目以降"),C.brace);
        if(!bRealtimeCommand) P.Cost=C.steps>0 ? TEXT("まず1マス進む / 今の移動を使う"):TEXT("ここで固守 / 今の行動を使う");
        if(!bRealtimeCommand && !CanIssue(C.steps>0 ? ETacticalCommand::Move:ETacticalCommand::Hold))
        { P.bEnabled=false;P.Cost=TEXT("今の行動は使用済み。次の指揮で選ぼう"); }
        if(!C.available)
        {
            P.Cost=C.blocker>=0 ? JourneyText(J.world.generals[J.world.armies[C.blocker].general].name)+TEXT("が進路を塞いでいる"):TEXT("地形・捕捉中の敵で進路なし");
        }
        P.Detail=P.Title+TEXT("\n")+P.Outcome+TEXT("\n")+P.Cost;
        P.Detail+=TEXT("\n移動後は行動の回復を待って固守。到着だけでは守備成功にならない。敵の移動や味方との衝突で遅れることがある。");
        if(C.cover) P.Detail+=TEXT("\n烽火の隣で待機。守備隊の交代・兵や士気の回復は行わない。");
        if(C.uncertain) P.Detail+=TEXT("\n未確認のマスを通る仮経路。見えていない敵は数に含まない。");
        if(C.breaksConcealment) { P.Cost+=TEXT(" / 潜伏解除");P.Detail+=TEXT("\n移動・固守で潜伏が解ける。"); }
        H.SupportChoices.Add(MoveTemp(P));
    }
}
bool ATacticalField::UseSupportChoice(int Index,int ExpectedVersion)
{
    if(IsPlaying() || State->bAutoBattle || State->bRealtimeStepping || State->bRealtimeChainRequested || State->Journey.stage!=JourneyStage::Battle) return false;
    auto Key=JourneySupportContext(State->Journey,State->Realtime,State->Selected);Key.push_back(bRealtimeCommand);
    if(ExpectedVersion!=State->SupportVersion || Key!=State->SupportKey || Index<0 || Index>=int(State->SupportChoices.size()))
    { State->Report=TEXT("戦況が変わった。援護先をもう一度確認しよう。");Refresh();return false; }
    for(const auto& O:State->Orders) if(O.army==State->Selected) return false;
    const auto C=State->SupportChoices[Index];if(!C.available) return false;
    if(!bRealtimeCommand)
    {
        const auto Command=C.steps>0 ? ETacticalCommand::Move:ETacticalCommand::Hold;
        if(!CanIssue(Command)) return false;
        Issue(Command);if(C.steps>0) CommandAt(C.route[1]);return true;
    }
    if(!QueueJourneySupport(State->Journey,State->Realtime,C)) { Refresh();return false; }
    State->Input=ETacticalInput::Select;State->bConfirmEndTurn=State->bConfirmWithdrawal=false;
    State->Report=TEXT("援護へ行軍→固守を指示。停止中。再開すると1指揮に1マス進む。到着後の固守にも通常の行動を使う。");
    Refresh();return true;
}
