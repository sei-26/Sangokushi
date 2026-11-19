#include "TacticalTrial.h"
#include "TacticalState.h"
using namespace frontline;

bool ATacticalField::CanUseGrandApproach() const
{
    return (State->Journey.CanChooseGrandApproach() || State->Journey.CanChooseYangrenApproach()) && !IsPlaying() && !State->bAutoBattle &&
        !State->bRealtimeStepping && !State->bRealtimeChainRequested && State->Orders.empty() && !State->ChainSignal.Armed();
}
bool ATacticalField::UseGrandApproach(int Index)
{
    if(Index<0 || Index>1 || !CanUseGrandApproach()) return false;
    auto& J=State->Journey;
    const bool Chosen=J.HasYangrenStrategy() ? J.ChooseYangrenApproach(Index==0 ? YangrenApproach::Forest:YangrenApproach::Advance):
        J.ChooseGrandApproach(Index==0 ? GrandApproach::RelieveLeft:GrandApproach::CentralAssault);
    if(!Chosen) return false;
    State->Realtime.paused=true;
    State->Report=JourneyText(State->Journey.message);
    Refresh();return true;
}
void ATacticalField::BuildGrandStrategyPresentation()
{
    const auto& J=State->Journey;auto& H=State->HUD;
    if(J.HasYangrenStrategy()) { BuildYangrenStrategyPresentation();return; }
    if(!J.HasGrandStrategy()) return;
    const auto& G=J.battleReview.grand;
    const bool LeftLives=J.world.armies[6].troops>0;
    const bool Rescue=G.approach==GrandApproach::RelieveLeft && !G.relievedOn && LeftLives;
    H.bGrandOrdersPending=J.CanChooseGrandApproach();
    H.GrandChoices={{TEXT("北の仲間を救う"),TEXT("本隊は北の林道へ。中央の支援は後になる。")},
        {TEXT("中央を一気に突破する"),TEXT("本隊が兵糧と士気を使い強行軍。左翼は自力で耐える。")}};
    H.GrandOrderStatus=G.approach==GrandApproach::RelieveLeft ? TEXT("左翼救援"):G.approach==GrandApproach::CentralAssault ? TEXT("中央強行軍"):TEXT("本隊の方針を選ぼう");
    const FString Left=G.relievedOn && LeftLives ? TEXT("救援成功"):!LeftLives ? TEXT("左翼壊滅"):TEXT("北の林で交戦中");
    H.MissionStep=FString::Printf(TEXT("%s ｜ 左翼 %d兵・%s ｜ 本隊 %d兵"),*H.GrandOrderStatus,J.world.armies[6].troops,*Left,J.world.armies[5].troops);
    H.MissionAdvice=H.bGrandOrdersPending ? TEXT("北を救うか、中央を押すか"):Rescue ? TEXT("北陣を崩し、左翼へ道をつなげ"):G.openedOn ? TEXT("本隊と敵本陣を落とせ"):TEXT("中央の橋を渡り、前陣へ旗を立てろ");
    if(Rescue)
    {
        H.MissionFocusTile=J.world.armies[6].tile;H.SecondGoalTile=H.MissionFocusTile;H.SecondGoalLabel=TEXT("救援する左翼");
    }
    H.MissionDetail+=TEXT("\n本隊の方針は開戦の指揮で一度だけ選ぶ。左翼救援は本隊が北へ迂回。中央突破は本隊が橋へ進み、左翼は自力で耐える。青い矢印は本隊の進軍予定。");
    H.MissionDetail+=FString::Printf(TEXT("\n中央強行軍は前陣突破まで、本隊が移動できた指揮に追加で1マス進む。兵糧100・士気3を実消費。接敵・閉塞・士気40未満では追加移動しない。実施%d回。"),G.forcedSteps);
    H.MissionDetail+=TEXT("\n士気25未満の敵は隣接だけで前陣を阻まない。敵本陣は撃破、または士気10以下で本陣から2マス以上敗走すれば崩壊。");
    H.MissionDetail+=TEXT("\n北陣を撃破するか捕捉中に士気25以下へ崩し、生きた左翼の隣か北の林道へ主力隊が到達すると救援成功。勝利時に左翼が生還すると、救援した隊は次の酸棗で先行配置・士気＋10（上限100）。");
    auto Observed=std::make_unique<Campaign>(J.world);
    for(auto& A:Observed->armies) if(A.faction!=Observed->player) A.troops=0;
    for(const auto& C:J.intelligence[0].contacts) if(C.current) Observed->armies[C.id]=C.observed;
    int At=J.world.armies[5].tile;
    const int Target=J.GrandMainTarget();
    H.GrandAdvanceRoute.Add(At);
    for(int Step=0;Step<4;++Step)
    {
        const int Next=JourneyAdvanceStep(*Observed,At,Target,Target==Campaign::At(7,8) || Target==Campaign::At(9,8));
        if(Next==At) break;
        At=Next;H.GrandAdvanceRoute.Add(At);Observed->armies[5].tile=At;
    }
}
