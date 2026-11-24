#include "TacticalTrial.h"
#include "TacticalState.h"
#include "TacticalCrossingLandscape.h"
#include "JourneyCoalitionBattle.hpp"
using namespace frontline;

void ATacticalField::BuildCoalitionBattlePresentation()
{
    const auto& J=State->Journey;auto& H=State->HUD;const auto& G=J.battleReview.grand;const auto& C=G.coalition;
    const bool retreat=J.coalitionPlan==CoalitionPlan::Protect;
    const bool caoHome=CoalitionReturnReached(J.world,5),reliefHome=CoalitionReturnReached(J.world,6);
    const int returned=int(caoHome)+int(reliefHome);
    H.GrandOrderStatus=retreat ? FString::Printf(TEXT("救援 %d/1 ｜ 西岸への帰還 %d/2隊"),C.rescuedOn ? 1:0,returned):
        G.openedOn ? TEXT("合流済み ｜ 合流点を維持し、本陣へ"):TEXT("共同攻勢 ｜ まず曹操との合流点へ");
    H.GoalLabel=retreat ? TEXT("西岸の帰還点"):G.openedOn ? TEXT("合流点を維持"):TEXT("曹操との合流点");
    H.CoalitionAllyStatus[0]=retreat && caoHome ? TEXT("西岸へ到着"):retreat && C.rescuedOn ? TEXT("西岸へ帰還中"):
        G.openedOn ? TEXT("共同攻勢"):TEXT("対岸の先行隊");
    H.CoalitionAllyStatus[1]=J.world.armies[6].troops<=0 ? TEXT("壊滅"):retreat && reliefHome ? TEXT("西岸へ到着"):
        C.rescuedOn ? (retreat ? TEXT("西岸へ帰還中"):TEXT("救援済み・共闘")):TEXT("救援を待つ");
    H.MissionStep=FString::Printf(TEXT("曹操 %d兵・%s ｜ 援軍 %d兵・%s"),J.world.armies[5].troops,*H.CoalitionAllyStatus[0],J.world.armies[6].troops,*H.CoalitionAllyStatus[1]);
    H.MissionAdvice=retreat ? (!C.rescuedOn ? TEXT("南の包囲を崩し、張邈の援軍へ届かせろ"):
        returned<2 ? TEXT("南橋の西へ、曹操と援軍を通せ"):TEXT("西岸の帰還点を確保しよう")):
        (G.openedOn ? TEXT("合流点を維持し、徐栄軍の本陣を崩せ"):TEXT("橋を渡れ！ 曹操の前陣へ旗を並べろ"));
    H.SecondGoalTile=J.world.armies[6].troops>0 ? J.world.armies[6].tile:-1;
    H.SecondGoalLabel=TEXT("張邈援軍・")+FString(retreat && reliefHome ? TEXT("到着"):
        C.rescuedOn ? (retreat ? TEXT("帰還中"):TEXT("合流済")):TEXT("救援前"));
    H.MissionFocusTile=retreat ? (!C.rescuedOn || !reliefHome ? J.world.armies[6].tile:!caoHome ? J.world.armies[5].tile:J.goal):J.goal;
    if(!retreat && G.openedOn) if(const auto* E=J.intelligence[0].Find(GrandCommander);E && E->current) H.MissionFocusTile=E->observed.tile;
    const auto Bridges=gunyucrossing::Bridges(J);
    for(int I=0;I<2 && Bridges.IsValidIndex(I);++I)
    {
        H.Crossings[I].Tile=Bridges[I];
        H.Crossings[I].Caption=I==0 ? (retreat ? TEXT("北橋 ／ 渡河点"):TEXT("北橋 ／ 攻勢の道")):
            retreat ? TEXT("南橋 ／ 援軍の帰路"):TEXT("南橋 ／ 援軍へ");
    }
    H.MissionDetail=retreat ? TEXT("南の包囲隊を撃破するか捕捉中に士気25以下へ崩し、主力一隊が生きた援軍の隣へ到着すると救援。援軍を先に南橋へ通し、曹操を続ける。二隊が西岸の帰還点から2マス以内へ戻り、主力一隊が帰還点を安全に確保すれば決着。本陣の掃討は不要。"):
        TEXT("対岸の合流点を主力一隊で確保し、曹操と合流する。合流点を維持し、徐栄軍の本陣を撃破するか捕捉中に士気25以下へ崩せば決着。南の包囲を崩して援軍へ実際に届けば、救った兵も次の物語へ残る。");
    H.MissionDetail+=TEXT("\n青い矢印は曹操、金の矢印は援軍の移動予定。予定は味方と捕捉中の敵から読むもので、到着を保証しない。南橋の西の出口を空け、主力は一つ先の帰還点で支えよう。");
    H.MissionDetail+=TEXT("\n曹操の壊滅は敗北。救出方針では張邈の援軍の壊滅も敗北となる。実際に救援して生還した兵の案内は、次の陽人で孫堅の先行配置を支える。史実では曹操が徐栄に敗れた会戦を背景にしたIF。");
    const bool moving=retreat ? C.rescuedOn>0:G.openedOn>0;
    H.GrandRouteLegend=TEXT("味方は待機・交戦中");
    if(!moving) return;
    const auto Plans=J.PlanCoalitionAllies();
    for(const auto& P:Plans)
    {
        if(P.army!=5 && P.army!=6) continue;
        if(P.army==6 && !C.rescuedOn) continue;
        auto& Route=P.army==5 ? H.GrandAdvanceRoute:H.ReliefAdvanceRoute;
        Route.Add(P.from);if(P.next==P.from) continue;
        Route.Add(P.next); // First step is exactly the public allied plan, including move order.
        auto observed=std::make_unique<Campaign>(J.world);
        for(auto& A:observed->armies) if(A.faction!=0) A.troops=0;
        for(const auto& E:J.intelligence[0].contacts) if(E.current) observed->armies[E.id]=E.observed;
        for(const auto& Earlier:Plans) if(Earlier.army<P.army) observed->armies[Earlier.army].tile=Earlier.next;
        int at=P.next;observed->armies[P.army].tile=at;
        for(int step=1;step<4;++step)
        {
            const int next=JourneyAdvanceStep(*observed,at,P.target,P.occupyTarget);if(next==at) break;
            at=next;Route.Add(at);observed->armies[P.army].tile=at;
        }
    }
    if(H.GrandAdvanceRoute.Num()>1 || H.ReliefAdvanceRoute.Num()>1)
        H.GrandRouteLegend=TEXT("青：曹操 ｜ 金：援軍の移動予定");
}
