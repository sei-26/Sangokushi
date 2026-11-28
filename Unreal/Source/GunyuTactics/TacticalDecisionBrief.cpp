#include "TacticalTrial.h"
#include "TacticalState.h"
using namespace frontline;

// A view of an already published turning point. Reading never picks orders,
// reveals hidden units or changes the action budget.
void ATacticalField::BuildDecisionBrief()
{
    auto& P=State->HUD.Decision;P={};const auto& J=State->Journey;
    if(!State->HUD.bDecisionPaused || !State->bAutoBattle || IsPlaying() || J.stage!=JourneyStage::Battle) return;
    const auto& Moment=State->DecisionPaused;P.bVisible=true;P.Kind=int(Moment.Kind);
    P.Turn=J.turn;P.Revision=J.world.revision;P.Focus=Moment.Tile;
    P.Title=TEXT("軍師、次の一手を");P.Context=Moment.Message;
    P.KeepPlan=TEXT("任せて続行：武将たちが現在の役目を優先する。");
    P.Takeover=TEXT("自分で指揮：誰を動かすか、ここで選び直せる。");
    using K=gunyudecision::EKind;
    switch(Moment.Kind)
    {
    case K::GuangzongFlank:
        P.Title=TEXT("別隊が北を狙っている");
        P.Context=TEXT("中央へ集中した隙を、黄巾の南翼が回り込む。北を支えるか、先に中央を崩すか。");
        P.KeepPlan=TEXT("続行：選んだ方針の攻めを優先。北の救援は保証されない。");
        P.Takeover=TEXT("介入：一隊を北へ返す、または目の前の敵を戦法で止める。");break;
    case K::GrandRelief:
        P.Title=TEXT("救援の旗がつながった");
        P.KeepPlan=J.HasCoalitionBattle() ? TEXT("続行：選んだ方針に沿い、帰還か共同攻勢へ進む。"):
            TEXT("続行：前陣と本隊の合流を目指す。");
        P.Takeover=J.HasCoalitionBattle() ? TEXT("介入：橋の出口を空け、帰る隊を守る配置へ。"):
            TEXT("介入：北の追撃を止め、中央へ戻す隊を決める。");break;
    case K::GrandOpening:
        P.Title=TEXT("突破した先を、どう支える？");
        P.KeepPlan=TEXT("続行：本隊と攻勢を続ける。旗を空けると合流が止まる。");
        P.Takeover=TEXT("介入：旗を守る隊と、敵本陣へ出す隊を分ける。");break;
    case K::GrandRally:
        P.Title=TEXT("本隊の旗に、次の合図を");
        P.KeepPlan=TEXT("続行：現在の役目に沿って決着を目指す。");
        P.Takeover=TEXT("介入：本隊を支え、残す戦法と本陣へ向ける戦力を決める。");break;
    case K::GrandCounter:
        P.Title=TEXT("敵の反撃をどう受ける？");
        P.KeepPlan=TEXT("続行：武将たちが反撃に対応。追撃と損耗に注意。");
        P.Takeover=TEXT("介入：森で受ける、隊を引く、戦法で足を止める。");break;
    case K::CoalitionReturned:
        P.Title=TEXT("帰還した旗を、守り切ろう");
        P.Takeover=TEXT("介入：一隊を西岸の帰還点へ。残敵の掃討は不要。");break;
    case K::EscortBlocked:
        P.Title=TEXT("護送路に敵の影");
        P.Takeover=TEXT("介入：車を守る隊を残し、別の隊で道の敵を崩す。");break;
    case K::PortOpening:
        P.Title=TEXT("開いた道へ、誰の旗を？");
        P.Takeover=TEXT("介入：突破した武将とは別の隊を、河港へ通す。");break;
    case K::BeaconLost:
        P.Title=TEXT("途切れた守りをつなぎ直せ");
        P.Takeover=TEXT("介入：守備の抜けた翼を見て、戻す武将を決める。");break;
    case K::SunJianBlocked:
        P.Title=TEXT("孫堅の行く手が塞がれた");
        P.Takeover=TEXT("介入：前進を急ぐか、一隊を孫堅の援護へ回すか。");break;
    default:break;
    }
    int Best=100000;FString Unspent;
    for(int A=0;A<3;++A) if(TacticChain::Living(J.world,A))
    {
        const auto& U=J.world.armies[A];const FString Name=JourneyText(J.world.generals[U.general].name);
        if(!J.chain.spent[A]) { if(!Unspent.IsEmpty()) Unspent+=TEXT("・");Unspent+=Name; }
        const int Score=(J.chain.spent[A] ? 100:0)+(J.chain.moved[A] ? 40:0)+(J.chain.disordered[A] ? 30:0)+
            (J.chain.concealed[A] || J.guarding[A] || U.tile==J.goal ? 20:0)+
            (StrategistJourney::OnMap(P.Focus) ? FMath::Min(20,J.world.MapDistance(U.tile,P.Focus)):0);
        if(Score<Best) { Best=Score;P.Officer=A; }
    }
    P.Ready=Unspent.IsEmpty() ? TEXT("戦法は全員使用済み。次の進行で行動が戻る。"):
        TEXT("戦法未使用：")+Unspent+TEXT("。射程と士気は別に確認。");
}

bool ATacticalField::UseDecisionBrief(const FTacticalPresentation::FDecisionBrief& Expected,bool bTakeover)
{
    const auto Pressed=Expected;Refresh();const auto Now=State->HUD.Decision;
    if(!Pressed.bVisible || !Now.bVisible || Pressed.Kind!=Now.Kind || Pressed.Turn!=Now.Turn ||
       Pressed.Revision!=Now.Revision || Pressed.Focus!=Now.Focus || Pressed.Officer!=Now.Officer) return false;
    if(!bTakeover) { Issue(ETacticalCommand::RealtimePause);return true; }
    StopAutoBattle();State->Input=ETacticalInput::Select;
    if(Now.Officer>=0)
    {
        SelectOfficer(Now.Officer);
        State->Report=JourneyText(State->Journey.world.generals[State->Journey.world.armies[Now.Officer].general].name)+
            TEXT("、次の一手を頼む！ 移動先や戦法は自分で選べる。兵力と使った行動はそのまま。");
    }
    Refresh();return true;
}
