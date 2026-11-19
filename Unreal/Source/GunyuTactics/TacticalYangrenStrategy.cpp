#include "TacticalTrial.h"
#include "TacticalState.h"
using namespace frontline;

void ATacticalField::BuildYangrenStrategyPresentation()
{
    const auto& J=State->Journey;auto& H=State->HUD;const auto& Y=J.battleReview.grand.yangren;
    const auto* cavalry=J.intelligence[0].Find(8);
    H.bGrandOrdersPending=J.CanChooseYangrenApproach();
    H.bYangrenCounter=Y.counterOn>0 && !Y.repelledOn;
    H.GrandChoices={{TEXT("森で先鋒の騎兵を崩す"),TEXT("孫堅は合図を待つ。森で呂布を崩してから合流。")},
        {TEXT("孫堅と一緒に押し込む"),TEXT("孫堅が兵糧と士気を使い前進。合流後の反撃に備える。")}};
    H.GrandOrderStatus=Y.approach==YangrenApproach::Forest ? TEXT("森で騎兵を崩す"):Y.approach==YangrenApproach::Advance ? TEXT("孫堅と突破"):TEXT("孫堅と、どう戦う？");
    const TCHAR* phase=Y.repelledOn ? TEXT("反撃を撃退"):H.bYangrenCounter ? TEXT("呂布の反撃中"):
        J.battleReview.grand.ralliedOn ? TEXT("本隊合流"):TEXT("南道から合流へ");
    H.MissionStep=FString::Printf(TEXT("%s ｜ 孫堅 %d兵 ｜ %s"),*H.GrandOrderStatus,J.world.armies[5].troops,phase);
    H.MissionAdvice=H.bYangrenCounter ? TEXT("呂布の騎兵反撃！ 孫堅を孤立させるな"):
        J.battleReview.grand.openedOn ? TEXT("孫堅と合流し、反撃に備えろ"):TEXT("森で騎兵を止め、辻へ旗を立てろ");
    H.MissionFocusTile=H.bYangrenCounter && cavalry && cavalry->current ? cavalry->observed.tile:J.goal;
    H.SecondGoalTile=J.world.armies[5].tile;H.SecondGoalLabel=TEXT("守り抜く孫堅本隊");
    H.MissionDetail=TEXT("辻を確保して孫堅と合流すると、呂布の親衛騎兵が反撃する。孫堅を生かし、反撃隊を撃破するか捕捉中に士気25以下へ崩して決着。本陣や残敵の全滅は不要。");
    H.MissionDetail+=TEXT("\n森で崩す方針では、先鋒の撃破・戦意低下・戦法での崩しを待って孫堅が前進。一緒に押す方針では、兵糧100・士気3を払って追加行軍できる。守った本隊の兵と騎兵への対処は、戦後と洛陽の任務選択に残る。");
    const bool waiting=Y.approach==YangrenApproach::Forest && !J.YangrenFirstChargeBroken();
    if(!waiting && !J.battleReview.grand.ralliedOn)
    {
        auto observed=std::make_unique<Campaign>(J.world);
        for(auto& A:observed->armies) if(A.faction!=0) A.troops=0;
        for(const auto& C:J.intelligence[0].contacts) if(C.current) observed->armies[C.id]=C.observed;
        int at=J.world.armies[5].tile;H.GrandAdvanceRoute.Add(at);
        for(int step=0;step<4;++step)
        {
            const int next=JourneyAdvanceStep(*observed,at,YangrenRally,true);
            if(next==at) break;at=next;H.GrandAdvanceRoute.Add(at);observed->armies[5].tile=at;
        }
    }
}
