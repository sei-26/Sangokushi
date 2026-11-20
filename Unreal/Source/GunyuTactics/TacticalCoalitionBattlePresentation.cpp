#include "TacticalTrial.h"
#include "TacticalState.h"
using namespace frontline;

void ATacticalField::BuildCoalitionBattlePresentation()
{
    const auto& J=State->Journey;auto& H=State->HUD;const auto& G=J.battleReview.grand;const auto& C=G.coalition;
    const bool retreat=J.coalitionPlan==CoalitionPlan::Protect;
    H.GrandOrderStatus=retreat ? TEXT("援軍救出・全軍帰還"):TEXT("曹操との共同攻勢");
    const TCHAR* phase=C.returnedOn ? TEXT("西岸へ帰還"):C.rescuedOn ? TEXT("援軍へ合流"):J.world.armies[6].troops>0 ? TEXT("南で援軍が孤立"):TEXT("援軍は壊滅");
    H.MissionStep=FString::Printf(TEXT("曹操 %d兵 ｜ 援軍 %d兵・%s"),J.world.armies[5].troops,J.world.armies[6].troops,phase);
    H.MissionAdvice=retreat ? (C.rescuedOn ? TEXT("南橋の西へ、曹操と援軍を通せ"):TEXT("南の包囲を崩し、張邈の援軍へ届かせろ")):
        (G.openedOn ? TEXT("曹操と徐栄軍の本陣を崩せ"):TEXT("橋を渡れ！ 曹操の前陣へ旗を並べろ"));
    H.SecondGoalTile=J.world.armies[6].tile;H.SecondGoalLabel=C.rescuedOn ? TEXT("生還させる張邈の援軍"):TEXT("救援する張邈の援軍");
    H.MissionFocusTile=retreat && !C.rescuedOn ? J.world.armies[6].tile:J.goal;
    if(!retreat && G.openedOn) if(const auto* E=J.intelligence[0].Find(GrandCommander);E && E->current) H.MissionFocusTile=E->observed.tile;
    H.MissionDetail=retreat ? TEXT("南の包囲隊を撃破するか捕捉中に士気25以下へ崩し、主力一隊が生きた援軍の隣へ到着すると救援。曹操と援軍が南橋を渡り、西の目標から2マス以内へ戻るまで守る。主力一隊が西の目標を確保すると決着。本陣の掃討は不要。"):
        TEXT("対岸の前陣を主力一隊で確保し、曹操と合流する。徐栄軍の本陣を撃破するか、捕捉中に士気25以下へ崩せば決着。南の包囲を崩して援軍へ実際に届けば、救った兵も次の物語へ残る。");
    H.MissionDetail+=TEXT("\n曹操の壊滅は敗北。救出方針では張邈の援軍の壊滅も敗北となる。実際に救援して生還した兵の案内は、次の陽人で孫堅の先行配置を支える。史実では曹操が徐栄に敗れた会戦を背景にしたIF。");
    const bool moving=retreat ? C.rescuedOn>0:G.openedOn>0;
    if(!moving) return;
    auto observed=std::make_unique<Campaign>(J.world);
    for(auto& A:observed->armies) if(A.faction!=0) A.troops=0;
    for(const auto& E:J.intelligence[0].contacts) if(E.current) observed->armies[E.id]=E.observed;
    int target=J.world.armies[5].tile;bool occupy=false;
    for(const auto& P:J.PlanCoalitionAllies()) if(P.army==5) { target=P.target;occupy=P.occupyTarget; }
    int at=J.world.armies[5].tile;
    H.GrandAdvanceRoute.Add(at);
    for(int step=0;step<4;++step)
    {
        const int next=JourneyAdvanceStep(*observed,at,target,occupy);if(next==at) break;
        at=next;H.GrandAdvanceRoute.Add(at);observed->armies[5].tile=at;
    }
}
