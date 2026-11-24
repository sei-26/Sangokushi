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
    const auto& Main=J.world.armies[5];
    const auto* North=J.intelligence[0].Find(8);const auto* HQ=J.intelligence[0].Find(GrandCommander);
    const bool LeftLives=J.world.armies[6].troops>0;
    const bool Rescue=G.approach==GrandApproach::RelieveLeft && !G.relievedOn && LeftLives;
    const bool NorthBroken=J.world.armies[8].troops<=0 || (North && North->current && North->observed.morale<=25);
    const bool HQBroken=J.world.armies[GrandCommander].troops<=0 || (HQ && HQ->current && HQ->observed.morale<=10 &&
        J.world.MapDistance(HQ->observed.tile,Campaign::At(13,6))>=2);
    H.bGrandOrdersPending=J.CanChooseGrandApproach();
    H.GrandChoices={{TEXT("北の仲間を救う"),TEXT("本隊は北の林道へ。北陣を崩し、主力も一隊届ける。中央の支援は後になる。")},
        {TEXT("中央を一気に突破する"),TEXT("中央橋から前陣へ。追加で進めた指揮だけ本隊が兵糧100・士気3を使う。")}};
    const FString Approach=G.approach==GrandApproach::RelieveLeft ? TEXT("左翼救援"):G.approach==GrandApproach::CentralAssault ? TEXT("中央強行軍"):TEXT("方針未定");
    const FString Left=G.relievedOn && LeftLives ? TEXT("救援済・生存"):!LeftLives ? (G.relievedOn ? TEXT("救援後に壊滅"):TEXT("壊滅")):TEXT("救援前");
    H.MissionStep=FString::Printf(TEXT("%s ｜ 左翼 %d兵・%s ｜ 前陣：%s ｜ 本隊合流：%s"),*Approach,J.world.armies[6].troops,*Left,
        G.openedOn ? TEXT("旗あり"):TEXT("未"),G.ralliedOn ? TEXT("済"):TEXT("未"));
    H.GoalLabel=G.openedOn ? TEXT("維持する前陣"):TEXT("突破する前陣");
    H.Crossings[0].Tile=Campaign::At(8,8);H.Crossings[0].Caption=TEXT("中央橋 ／ 前陣へ");
    H.Crossings[1].Tile=Campaign::At(8,11);H.Crossings[1].Caption=TEXT("南渡し ／ 側面へ");
    H.SecondGoalTile=LeftLives && !G.ralliedOn ? J.world.armies[6].tile:Campaign::At(13,6);
    H.SecondGoalLabel=LeftLives && !G.ralliedOn ? (G.relievedOn ? TEXT("左翼・救援済"):TEXT("北の左翼")):TEXT("黄巾軍の本陣");
    H.MissionFocusTile=J.goal;
    if(H.bGrandOrdersPending)
    { H.MissionAdvice=TEXT("北を救うか、中央を押すか");H.GrandOrderStatus=TEXT("救援には主力も一隊。中央突破は左翼が自力で耐える"); }
    else if(HQBroken)
    {
        H.MissionAdvice=G.ralliedOn ? TEXT("本陣は崩れた。前陣を確保し直そう"):TEXT("本陣は崩れた。前陣を守り、本隊を迎えよう");
        H.GrandOrderStatus=TEXT("前陣の安全な確保と本隊の合流で決着。残敵掃討は不要");
        if(!G.ralliedOn) H.MissionFocusTile=Main.tile;
    }
    else if(Rescue)
    {
        H.MissionAdvice=NorthBroken ? TEXT("北陣は崩れた。主力を左翼へ届けよう"):TEXT("北陣を崩し、左翼へ道をつなげ");
        H.GrandOrderStatus=TEXT("北の林道へ主力を一隊。本隊だけの到着では救援にならない");
        H.MissionFocusTile=!NorthBroken && North && North->current ? North->observed.tile:J.world.armies[6].tile;
    }
    else if(G.openedOn && !G.ralliedOn)
    {
        H.MissionAdvice=TEXT("前陣を守り、本隊を通して本陣へ");H.GrandOrderStatus=TEXT("前陣占領で終わらない。皇甫嵩の行く手を空けよう");H.MissionFocusTile=Main.tile;
    }
    else if(G.ralliedOn)
    {
        H.MissionAdvice=TEXT("前陣を残し、本隊と敵本陣を崩そう");H.GrandOrderStatus=TEXT("本隊合流済。主力の戦法と連鎖を本陣へ");
        H.MissionFocusTile=HQ && HQ->current ? HQ->observed.tile:Campaign::At(13,6);
    }
    else
    {
        H.MissionAdvice=G.relievedOn && LeftLives ? TEXT("北の救援は届いた。中央の前陣へ旗を"):TEXT("中央橋か南渡しから、前陣へ旗を立てよう");
        H.GrandOrderStatus=!LeftLives ? TEXT("左翼は壊滅。前陣を開き、本隊と本陣へ"):
            G.relievedOn ? TEXT("救援した左翼の生還を支え、前陣の攻めへ戻ろう"):TEXT("本隊の通る中央橋と出口を空けよう。左翼の援護は任意");
    }
    H.MissionDetail=TEXT("金の旗の前陣を安全に確保し、本隊と合流して敵本陣を崩す。前陣を取るだけでは勝利にならず、最後まで主力の旗を残す。青緑の旗は北で耐える左翼。中央橋と南渡しは別の進路で、北の救援路は川の上流を回る。");
    H.MissionDetail+=TEXT("\n方針は開戦の指揮で一度だけ選ぶ。左翼救援は本隊が北へ迂回し、中央の支援は遅れる。中央突破は本隊が中央橋へ進み、左翼は自力で耐える。どちらでも主力の一隊を北へ回して救援できる。");
    H.MissionDetail+=FString::Printf(TEXT("\n中央強行軍は前陣突破まで、追加で1マス進めた指揮だけ兵糧100・士気3を使う。接敵・閉塞・兵糧不足・士気40未満・混乱では追加移動しない。実施%d回。青い線は捕捉情報に基づく数歩分の移動予定で、到着や追加行軍の保証ではない。"),G.forcedSteps);
    H.MissionDetail+=TEXT("\n士気25未満の敵は隣接だけで前陣を阻まない。敵本陣は撃破、または士気10以下で本陣から2マス以上敗走すれば崩壊。");
    H.MissionDetail+=TEXT("\n北陣を撃破するか捕捉中に士気25以下へ崩し、生きた左翼の隣、または左翼が北の林付近にいる間に北の林道へ主力隊が到達すると救援成功。本隊の到着だけでは成立しない。勝利時に左翼が生還すると、救援した隊は次の酸棗で先行配置・士気＋10（上限100）。救援しても、その後に壊滅すれば生還にはならない。");
    H.GrandRouteLegend=H.bGrandOrdersPending ? TEXT("本隊の方針を選ぼう"):J.chain.disordered[5] ? TEXT("本隊は混乱で行軍できない"):TEXT("青：皇甫嵩本隊の移動予定");
    if(H.bGrandOrdersPending || J.chain.disordered[5] || Main.troops<=0) return;
    const auto Plans=J.PlanGrandAllies();
    const auto Plan=std::find_if(Plans.begin(),Plans.end(),[](const JourneyEnemyPlan& P){return P.army==5;});
    if(Plan==Plans.end() || Plan->next==Main.tile) { H.GrandRouteLegend=TEXT("本隊は待機・交戦中。前路と橋の出口を確認");return; }
    auto Observed=std::make_unique<Campaign>(J.world);
    for(auto& A:Observed->armies) if(A.faction!=Observed->player) A.troops=0;
    for(const auto& C:J.intelligence[0].contacts) if(C.current) Observed->armies[C.id]=C.observed;
    int At=Main.tile;
    const int Target=Plan->target;
    H.GrandAdvanceRoute.Add(At);
    for(int Step=0;Step<4;++Step)
    {
        const int Next=Step==0 ? Plan->next:JourneyAdvanceStep(*Observed,At,Target,Target==Campaign::At(7,8) || Target==Campaign::At(9,8));
        if(Next==At) break;
        At=Next;H.GrandAdvanceRoute.Add(At);Observed->armies[5].tile=At;
    }
}
