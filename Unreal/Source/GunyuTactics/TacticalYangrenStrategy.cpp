#include "TacticalTrial.h"
#include "TacticalState.h"
using namespace frontline;

void ATacticalField::BuildYangrenStrategyPresentation()
{
    const auto& J=State->Journey;auto& H=State->HUD;const auto& G=J.battleReview.grand;const auto& Y=G.yangren;
    const auto& Main=J.world.armies[5];const auto* First=J.intelligence[0].Find(3);const auto* Cavalry=J.intelligence[0].Find(8);
    const bool Waiting=Y.approach==YangrenApproach::Forest && !J.YangrenFirstChargeBroken();
    const bool Arrived=J.SunJianAtRally();
    const bool Contact=J.intelligence[0].EnemyNear(J.world,Main.tile,1),Disordered=J.chain.disordered[5];
    H.bGrandOrdersPending=J.CanChooseYangrenApproach();
    H.bYangrenCounter=Y.counterOn>0 && !Y.repelledOn;
    H.GrandChoices={{TEXT("森で先鋒の騎兵を崩す"),TEXT("孫堅は合図を待つ。先鋒の撃破・戦意低下・戦法での崩しで前進。")},
        {TEXT("孫堅と一緒に押し込む"),TEXT("南道を空けて合流。追加で進めた指揮だけ兵糧100・士気3を使う。")}};
    H.GoalLabel=TEXT("義勇軍が守る辻");
    H.SecondGoalTile=Main.tile;
    H.SecondGoalLabel=TEXT("孫堅・")+FString(G.ralliedOn ? TEXT("合流済"):Arrived ? TEXT("辻を待つ"):H.bGrandOrdersPending ? TEXT("方針待ち"):Waiting ? TEXT("合図待ち"):
        Disordered ? TEXT("混乱"):Contact ? TEXT("接敵中"):TEXT("進軍中"));
    H.MissionStep=FString::Printf(TEXT("孫堅 %d兵 ｜ 辻に旗：%s ｜ 合流：%s ｜ %s"),Main.troops,G.openedOn ? TEXT("済"):TEXT("未"),
        G.ralliedOn ? TEXT("済"):TEXT("未"),Y.repelledOn ? TEXT("反撃を撃退"):H.bYangrenCounter ? TEXT("親衛騎兵の反撃中"):TEXT("反撃に備える"));
    H.MissionFocusTile=J.goal;
    if(Y.repelledOn)
    { H.MissionAdvice=TEXT("反撃隊は退いた。辻を確保し直そう");H.GrandOrderStatus=TEXT("孫堅を生かし、辻を確保して決着へ"); }
    else if(H.bYangrenCounter)
    {
        H.MissionAdvice=TEXT("呂布の騎兵反撃！ 孫堅と辻を守れ");
        H.GrandOrderStatus=TEXT("反撃隊を崩す ｜ 敵本陣の掃討は不要");
        H.MissionFocusTile=Cavalry && Cavalry->current ? Cavalry->observed.tile:Main.tile;
    }
    else if(H.bGrandOrdersPending)
    { H.MissionAdvice=TEXT("まず、孫堅との戦い方を選ぼう");H.GrandOrderStatus=TEXT("開戦の二案：森で待つ／一緒に押す"); }
    else if(Arrived && !G.openedOn)
    { H.MissionAdvice=TEXT("孫堅は到着。義勇軍で辻を確保しよう");H.GrandOrderStatus=TEXT("辻に旗を立て、合流後の反撃に備える"); }
    else if(Waiting)
    {
        H.MissionAdvice=TEXT("先鋒を森で崩し、孫堅に合図を");H.GrandOrderStatus=TEXT("森の策：先鋒を崩すまで本隊は待機");
        H.MissionFocusTile=First && First->current ? First->observed.tile:Campaign::At(8,7);
    }
    else if(G.openedOn && !G.ralliedOn)
    {
        H.MissionAdvice=TEXT("辻を守り、孫堅との合流後の反撃に備えろ");H.GrandOrderStatus=TEXT("南の合流点を空け、本隊を通そう");H.MissionFocusTile=Main.tile;
    }
    else
    {
        H.MissionAdvice=TEXT("南道を空け、孫堅と辻へ進もう");
        H.GrandOrderStatus=Y.approach==YangrenApproach::Advance ? TEXT("共同進軍：義勇軍が辻、本隊は南の合流点へ"):
            J.YangrenFirstChargeBroken() ? TEXT("先鋒を崩した。孫堅を辻の南で迎えよう"):TEXT("南道を空け、孫堅との合流へ");
        if(!G.ralliedOn && (Contact || Disordered)) H.MissionFocusTile=Main.tile;
    }
    H.MissionDetail=TEXT("義勇軍は青緑の旗の辻を安全に確保し、孫堅が着く南隣の橙の合流点は空ける。辻に旗を立てて孫堅と合流すると、呂布の親衛騎兵が反撃する。孫堅を生かし、反撃隊を撃破するか捕捉中に士気25以下へ崩し、辻を安全に確保して決着。本陣や残敵の全滅は不要。");
    H.MissionDetail+=TEXT("\n森の策では先鋒の撃破・戦意低下・戦法での崩しが合図。一緒に押す策では、追加で進めた指揮だけ孫堅が兵糧100・士気3を使う。接敵や混乱は本隊の行軍を止める。青い線は合流までの移動予定で、到着の保証ではない。");
    H.MissionDetail+=TEXT("\n目標へ / U：森の策では捕捉中の先鋒、合流待ちは孫堅、反撃中は捕捉中の反撃隊を見る。反撃隊を見失ったら孫堅を見る。反撃後に辻が空いていれば、辻へ戻る。守った兵と対処は戦後・洛陽の作戦に残る。");
    H.GrandRouteLegend=G.ralliedOn ? TEXT("合流済み。孫堅と辻を守ろう"):Arrived ? TEXT("本隊は合流点へ到着。義勇軍は辻へ"):H.bGrandOrdersPending ? TEXT("孫堅は方針の決定待ち"):Waiting ? TEXT("孫堅は先鋒を崩す合図を待つ"):
        Disordered ? TEXT("孫堅は混乱で行軍できない"):Contact ? TEXT("孫堅は接敵で足止め中"):TEXT("青：孫堅の合流までの移動予定");
    if(H.bGrandOrdersPending || Waiting || G.ralliedOn || Disordered || Contact || Arrived) return;
    auto Observed=std::make_unique<Campaign>(J.world);
    for(auto& A:Observed->armies) if(A.faction!=0) A.troops=0;
    for(const auto& C:J.intelligence[0].contacts) if(C.current) Observed->armies[C.id]=C.observed;
    int At=Main.tile;H.GrandAdvanceRoute.Add(At);
    for(int Step=0;Step<4;++Step)
    {
        const int Next=JourneyAdvanceStep(*Observed,At,YangrenRally,true,J.goal);
        if(Next==At) break;At=Next;H.GrandAdvanceRoute.Add(At);Observed->armies[5].tile=At;
    }
    if(H.GrandAdvanceRoute.Num()<2) H.GrandRouteLegend=TEXT("孫堅の前路を確認し、合流点を空けよう");
}
