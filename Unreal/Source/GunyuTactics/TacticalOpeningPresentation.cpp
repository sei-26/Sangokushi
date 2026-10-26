#include "TacticalTrial.h"
#include "TacticalState.h"
using namespace frontline;

void ATacticalField::BuildOpeningPresentation()
{
    const auto& J=State->Journey;auto& P=State->HUD.Journey;
    if (J.history[0].completed && J.history[0].rescued)
        P.History+=TEXT(" 初戦の護送経路：")+JourneyText(ConvoyPlan(J.history[0].route).name);
    if (P.bPreparation && J.IsRescue())
    {
        P.bChooseRoute=true;P.bSouthernRoute=J.convoyRoute==JourneyConvoyRoute::SouthernWood;
        P.ConvoyRouteInfo=JourneyText(ConvoyPlan(J.convoyRoute).description);
        P.Outcome=FString::Printf(TEXT("最優先：季安の避難車を出口まで護送 / 全軍更新%d回まで\n%s・%d区間。先鋒を倒しても、車が届くまで任務は続く。"),J.TurnBudget(),*JourneyText(ConvoyPlan(J.convoyRoute).name),static_cast<int>(J.ConvoyPath().size())-1);
        if (P.bSouthernRoute)
        {
            const TCHAR* Lines[]={
                TEXT("季安が、南の林を通れる者を見つけてきた。案内料に120、道のりは一つ長くなる。それでも正面を崩すことに兵を使い切らずに済むなら、選ぶ価値はある。出口まで誰を先に回す？"),
                TEXT("林道を選んでも、車だけで出口を抜けられるとは限らぬ。先鋒を崩す隊に全員を集めず、前路を開く隊と出口を守る隊を決めよう。どちらの役も、戦果になる。"),
                TEXT("なら、敵の旗が立ってても車を通せばいいんだな。出口で邪魔する奴を押しのける役なら任せろ。だが俺まで車道を塞いだら、遠慮なくどけって言えよ。")};
            P.Dialogue=Lines[FMath::Clamp(State->Interlocutor,0,2)];
        }
    }
    if (!P.bDebrief) return;
    const auto& R=J.battleReview;
    P.Outcome=JourneyText(J.ResolutionName())+TEXT("\n")+JourneyText(J.message);
    P.ReviewFacts=FString::Printf(TEXT("経過%d周期 / 帰還兵%d / 損失%d / 信頼%d\nこの戦いで成立した戦法%d手（最大%d連鎖）"),FMath::Max(0,R.endedAt-1),J.survivors,J.casualties,J.trust,R.tacticSteps,R.longestChain);
    if (J.HasConvoy())
        P.ReviewFacts+=FString::Printf(TEXT("\n%s %d/%d区間 / 前路の敵で停止%d回 / 通路が塞がり停止%d回"),*JourneyText(ConvoyPlan(J.IsRescue() ? J.convoyRoute : JourneyConvoyRoute::Road).name),J.convoyStep,static_cast<int>(J.ConvoyPath().size())-1,R.enemyStops,R.trafficStops);
    if (J.Definition().kind==JourneyObjective::Escort) P.ReviewFacts+=FString::Printf(TEXT("\n持ち帰った積荷%d"),J.recoveredCargo);
    P.ReviewLesson=JourneyText(J.BattleReviewLesson());
    if (J.won && J.IsRescue() && J.convoyRoute==JourneyConvoyRoute::SouthernWood)
    {
        const TCHAR* Lines[]={
            TEXT("林道の案内人が、季安と一緒に戻った。君が払った金は、道を知る者を仲間にするためのものだった。あの車が届いたという事実を、次の軍議でも覚えておこう。"),
            TEXT("正面を崩すことだけが退路を作る方法ではなかった。車が止まった場所と、そこへ誰を回したかを振り返ろう。次の戦で同じ道が使えるとは限らぬ。"),
            TEXT("林の向こうから、ちゃんと車が出てきたな。敵の旗ばかり見てたら、あの瞬間を見逃してたかもしれねえ。季安に言っとけ。次も出口で待ってるってな。")};
        P.Dialogue=Lines[FMath::Clamp(State->Interlocutor,0,2)];
    }
    P.Outcome+=J.won ? TEXT("\n生存兵と残りの兵糧は野営地へ精算済み。") : TEXT("\n軍議に戻り、経路と役割を選び直せる。");
}
