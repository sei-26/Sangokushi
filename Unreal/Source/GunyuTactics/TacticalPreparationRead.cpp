#include "TacticalTrial.h"
#include "TacticalState.h"
#include "JourneyBattlefield.hpp"
using namespace frontline;
void ATacticalField::BuildPreparationPresentation()
{
    const auto& J=State->Journey;auto& P=State->HUD.Journey;const auto& Camp=J.world.cities[0];
    P.PlanObjective=(J.IsRescue() ? FString(TEXT("季安の避難車を出口まで護送")):JourneyText(J.Definition().objective))+
        FString::Printf(TEXT(" / 期限%d指揮"),J.TurnBudget());
    const auto& Field=BattlefieldDefinition(J.mission);
    P.PlanRoute=JourneyText(Field.name)+TEXT(" / ")+JourneyText(Field.reading);
    if(J.IsRescue())
    {
        const auto Route=J.IsRescue() ? J.convoyRoute:JourneyConvoyRoute::Road;const auto& Path=ConvoyPlan(Route);
        P.PlanRoute=JourneyText(Path.name)+FString::Printf(TEXT(" / %d区間 / 案内料%d"),int(J.ConvoyPath().size())-1,Path.guideGold);
    }
    for(int Choice=0;Choice<2;++Choice)
    {
        const auto Cost=J.PreparationCosts(Choice);
        const int Deployed=FMath::Max(0,FMath::Min(4200,Camp.troops-1000-Cost.escort));
        FPreparationPresentation Plan;Plan.Title=Choice==0 ? TEXT("伏兵の段取り"):TEXT("兵糧を厚くする");
        Plan.Description=Choice==0 ? TEXT("森林で関羽の伏兵準備を試みる。配置や斥候によって露見することがある。"):
            TEXT("携行兵糧と初期士気を厚くする。伏兵は戦場で別途準備する。");
        Plan.Food=Cost.TotalFood();Plan.Gold=Cost.gold;Plan.MinimumTroops=Cost.MinimumTroops();
        Plan.MissingFood=FMath::Max(0,Plan.Food-Camp.food);Plan.MissingGold=FMath::Max(0,Plan.Gold-Camp.gold);
        Plan.MissingTroops=FMath::Max(0,Plan.MinimumTroops-Camp.troops);
        if(Plan.MissingTroops) Plan.Shortage+=FString::Printf(TEXT("兵あと%d  "),Plan.MissingTroops);
        if(Plan.MissingFood) Plan.Shortage+=FString::Printf(TEXT("兵糧あと%d  "),Plan.MissingFood);
        if(Plan.MissingGold) Plan.Shortage+=FString::Printf(TEXT("軍資金あと%d"),Plan.MissingGold);
        for(int I=0;I<3;++I) Plan.Morale.Add(FMath::Clamp((Choice==1 ? 100:90)+J.StoryMoraleBonus(I),0,100));
        Plan.MoraleText=FString::Printf(TEXT("出陣士気　劉備%d / 関羽%d / 張飛%d"),Plan.Morale[0],Plan.Morale[1],Plan.Morale[2]);
        Plan.Cost=FString::Printf(TEXT("出陣 %d%s\n兵糧 %d / 資金 %d"),Deployed,Cost.escort ? TEXT(" ＋護衛600"):TEXT(""),Plan.Food,Plan.Gold);
        Plan.Remaining=FString::Printf(TEXT("残兵 %d / 糧 %d\n残資金 %d"),Camp.troops-Deployed-Cost.escort,Camp.food-Plan.Food,Camp.gold-Plan.Gold);
        if(Cost.cargo) Plan.Description+=TEXT(" 積荷2,000と護衛用の糧900を含む。");
        Plan.bSelected=Choice==State->PreparationChoice;Plan.bAffordable=J.CanPrepare(Choice);
        if(!Plan.bAffordable) Plan.Remaining=TEXT("不足：")+Plan.Shortage;
        P.Plans.Add(MoveTemp(Plan));
    }
    const auto& First=P.Plans[0];const auto& Second=P.Plans[1];
    P.PlanDifference=FString::Printf(TEXT("案2は案1より  兵糧 %+d / 軍資金 %+d。初動の伏兵準備と、高い士気・携行糧を比べる。"),Second.Food-First.Food,Second.Gold-First.Gold);
    const auto& Selected=P.Plans[State->PreparationChoice];
    P.PlanReadiness=Selected.bAffordable ? TEXT("出陣条件を満たしている。選択中：")+Selected.Title:TEXT("出陣できない：")+Selected.Shortage;
    if(!Selected.bAffordable && P.Plans[1-State->PreparationChoice].bAffordable)
        P.PlanReadiness+=FString::Printf(TEXT("　案%dなら現在の資源で出陣できる。"),2-State->PreparationChoice);
    const TCHAR* Names[]={TEXT("劉備"),TEXT("関羽"),TEXT("張飛")};
    const FString Liu=J.IsRescue() ? TEXT("敵の旗が倒れても、季安の車が着くまでは終わらない。道に残す隊を、先に決めよう。"):
        J.mission==1 ? TEXT("積荷は1,200以上残して届ける。追撃より先に、糧車が通る場所を考えてほしい。"):
        J.mission==2 ? TEXT("二つの烽火を、同時に三指揮守る。片方に全員を集めず、誰を信じて残すか決めよう。"):
        TEXT("先鋒を崩すか、安全な出口を確保して道を開く。敵を倒すことと、役目を果たすことを一緒に考えよう。");
    const FString Guan=!Selected.bAffordable ? TEXT("まず兵と糧を確かめよ。不足のある案で出ることはできぬ。別の案と、現在の蓄えを比べるのだ。"):
        J.CurrentPledge()==JourneyPledge::Preserve ? TEXT("九割を連れ帰ると言ったな。士気も、退く場所も要る。高い士気を選ぶ代わりに何を使うか、そこまで見て決めよ。"):
        State->PreparationChoice==0 ? TEXT("出陣時の伏兵準備を試みる。露見した時の動きも決めておけ。選ぶだけで、伏兵の成功を約束できるわけではない。"):
        TEXT("初期士気と携行糧を厚くする。その分の糧を使う。私の伏兵は戦場で準備することになる。主攻を急がせるな。");
    const FString Zhang=J.CurrentPledge()==JourneyPledge::Coordinate ? TEXT("皆をつなぐって言ったな！ 一人でも戦法は使えるが、二隊以上をつなぐ場所は、お前が作ってくれよ。"):
        J.CurrentPledge()==JourneyPledge::Preserve ? TEXT("帰れと言う合図も先に決めとけ。突っ込めだけじゃ、お前の約束は守れねえからな。"):
        J.HasConvoy() ? TEXT("前へ出る奴と、荷の横に残る奴。どっちも役目だってのは分かった。誰に任せるか、お前が呼べよ！"):
        J.mission==2 ? TEXT("離れていても、隣が危ねえ時は気にしてる。こっちを走らせるなら、元いた所も空くって覚えとけよ。"):
        TEXT("一人で戦法を撃つか、仲間へつなぐか。走る前に、その次の一手も教えてくれよ、軍師！");
    for(int I=0;I<3;++I)
    { FDialoguePresentation Advice;Advice.Speaker=I;Advice.Name=Names[I];Advice.Text=I==0 ? Liu:I==1 ? Guan:Zhang;P.PlanAdvice.Add(MoveTemp(Advice)); }
}
