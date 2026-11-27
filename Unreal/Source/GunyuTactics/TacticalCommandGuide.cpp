#include "TacticalTrial.h"
#include "TacticalState.h"
using namespace frontline;

// This is a view of the current command state. It never advances a cycle,
// picks a target, replaces a reservation, or predicts unseen enemies.
void ATacticalField::BuildCommandGuide()
{
    auto& H=State->HUD;const auto& J=State->Journey;
    if(J.stage!=JourneyStage::Battle || IsPlaying()) return;
    auto& Text=H.CommandGuide;
    if(State->bAutoBattle)
    {
        Text=H.bDecisionPaused ? TEXT("節目で停止中。自分で指揮するか、Spaceで武将たちに任せて再開。"):
            RealtimePaused() ? TEXT("武将たちに任せたまま停止中。Spaceで再開できる。"):
            TEXT("武将たちが指揮中。いつでも「自分で指揮する」で采配を引き取れる。");return;
    }
    if(State->Input==ETacticalInput::Move)
    { Text=bRealtimeCommand ? TEXT("地図で行軍先を選ぶ → Spaceで進める。Escで取消。"):
        TEXT("水色の隣接マスを選ぶ。Escで移動先の指定を取消。");return; }
    if(State->Input==ETacticalInput::Lure)
    { Text=TEXT("水色の後退先を選んで誘導を予約 → Cで発動。Escで取消。");return; }
    if(State->Input==ETacticalInput::Position)
    { Text=TEXT("番号のマスを比較し、採用する位置を確定。比較だけでは動かない。");return; }
    if(State->ChainSignal.Armed())
    { Text=TEXT("連鎖は合図待ち。変更・解除は「詳しい采配」から。");return; }
    if(!State->Orders.empty())
    { Text=State->Orders.size()==1 ? TEXT("一隊の戦法も発動できる。Cで実行、取消なら予約を解除。"):
        TEXT("戦法を予約中。Cで連鎖を実行、順番は「詳しい采配」で確認。");return; }
    if(State->bRealtimeStepping || State->bRealtimeChainRequested)
    { Text=TEXT("命令を実行中。完了を待って、次の一手を選ぼう。");return; }
    if(H.OpeningOrder.Action==EOpeningOrderAction::Solo)
    { Text=TEXT("最初は右の「この一手を託す」から。消費を読み、一隊で戦法を試せる。");return; }
    if(H.bGuardOrder)
    { Text=TEXT("固守中。戦法を使うには「固守解除」から。行動済みなら周期を進めよう。");return; }
    if(J.lastChain.completed==1 && !H.BattleChoices.IsEmpty())
    {
        bool Ready=false;for(int I=0;I<3;++I) Ready|=TacticChain::Living(J.world,I) && !J.chain.spent[I];
        if(Ready) { Text=TEXT("未行動の仲間が残っている。右の単独・連鎖を比べて、次を選ぼう。");return; }
    }
    if(TacticChain::Living(J.world,State->Selected) && J.chain.spent[State->Selected])
    {
        bool Ready=false;for(int I=0;I<3;++I) Ready|=TacticChain::Living(J.world,I) && !J.chain.spent[I];
        Text=Ready ? TEXT("この武将の戦法は行動済み。下の未行動の仲間を選んで指揮できる。"):
            bRealtimeCommand ? TEXT("全員の戦法が行動済み。「1周期進める / .」で双方の進行と次の行動へ。"):
            TEXT("全員の戦法が行動済み。Spaceで指揮を終え、次の行動へ。");return;
    }
    if(!H.BattleChoices.IsEmpty())
    {
        Text=J.HasGrandStrategy() && !J.battleReview.grand.relievedOn && TacticChain::Living(J.world,6) ?
            TEXT("北へ回す仲間を残すか、今の敵に連鎖を使うか。右の策は選ぶと発動。"):
            TEXT("右の策は選ぶと発動する。単独で仕掛けるか、仲間とつなぐかを選ぼう。");return;
    }
    if(H.MissionMarch.bVisible)
    { Text=bRealtimeCommand ? TEXT("目標へ一隊の行軍を指示できる。移動はSpaceで再開してから。"):TEXT("目標と敵の位置を見て、移動できる一歩を選ぼう。");return; }
    Text=bRealtimeCommand ? TEXT("下で武将を選び、地図で相手を選ぶ。Spaceで戦いを進められる。"):
        TEXT("下で武将を選び、地図で相手を選ぶ。Spaceで指揮を終える。");
}
