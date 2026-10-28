#include "TacticalTrial.h"
#include "TacticalState.h"
#include "JourneyBattlefield.hpp"
#include "JourneyPlanExample.hpp"
#include "TacticalPlanningGuide.h"
using namespace frontline;
void ATacticalField::BuildPreparationPresentation()
{
    BuildPreparationMap();
    const auto& J=State->Journey;auto& P=State->HUD.Journey;const auto& Camp=J.world.cities[0];
    const auto Example=PlanExample(J);
    P.ExampleName=JourneyText(Example.name);P.ExamplePurpose=JourneyText(Example.purpose);
    P.ExampleManual=JourneyText(Example.manualTask);P.ExampleCaution=JourneyText(Example.caution);
    for(int I=0;I<3;++I)
    {
        const auto& A=Example.assignments[I];
        P.ExampleOrders.Add(A.duty==JourneyDuty::None ? FString(TEXT("固定せず、戦場で自分で指揮")):
            planningguide::Place(P.Map,A.tile)+(A.duty==JourneyDuty::Hold ? TEXT(" → 固守"):TEXT(" → 伏兵準備")));
    }
    P.PlanObjective=(J.IsRescue() ? FString(TEXT("季安の避難車を出口まで護送")):JourneyText(J.ObjectiveBrief()))+
        FString::Printf(TEXT(" / 期限%d指揮"),J.TurnBudget());
    const auto& Field=BattlefieldDefinition(J.mission);
    P.PlanRoute=JourneyText(Field.name)+TEXT(" / ")+(J.Definition().kind==JourneyObjective::CaptureCamp ? FString(TEXT("二つの橋から指揮所へ急襲。護送と残敵掃討は不要")):JourneyText(Field.reading));
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
        if(J.campaignFlow)
        {
            const int Start=J.PreparationPositions(Choice)[1];
            Plan.Title=Choice==0 ? TEXT("先遣伏兵"):TEXT("陣中補給");
            Plan.Description=Choice==0 ? FString::Printf(TEXT("初手を仕込む。関羽を森林(%d,%d)で伏兵準備。初手の移動・戦法を残せる。"),Start%frontline::Width,Start/frontline::Width):
                TEXT("戦法の後に立て直す。戦闘中に１回、主力へ糧1,800を分配し、士気＋20。使う時は自分で決める。");
            Plan.Tradeoff=Choice==0 ? TEXT("代償：追加の予備糧なし。動けば隠蔽解除、敵の接近・斥候で露見。Mで２歩以内の伏兵地点を指定可。"):
                TEXT("代償：先遣・初期伏兵なし。行動権や固有戦法の回数は戻らない。未使用の糧は帰還時に回収。");
            if(Choice==0 && J.assignments[1].duty==JourneyDuty::Ambush && Start!=J.assignments[1].tile)
                Plan.Tradeoff=TEXT("指定の森林は先遣範囲外／他隊の出陣地点と重なる。関羽は通常位置の森で準備し、戦場で移動。予備糧はなし。");
        }
        Plan.Food=Cost.TotalFood();Plan.Gold=Cost.gold;Plan.MinimumTroops=Cost.MinimumTroops();
        Plan.MissingFood=FMath::Max(0,Plan.Food-Camp.food);Plan.MissingGold=FMath::Max(0,Plan.Gold-Camp.gold);
        Plan.MissingTroops=FMath::Max(0,Plan.MinimumTroops-Camp.troops);
        if(Plan.MissingTroops) Plan.Shortage+=FString::Printf(TEXT("兵あと%d  "),Plan.MissingTroops);
        if(Plan.MissingFood) Plan.Shortage+=FString::Printf(TEXT("兵糧あと%d  "),Plan.MissingFood);
        if(Plan.MissingGold) Plan.Shortage+=FString::Printf(TEXT("軍資金あと%d"),Plan.MissingGold);
        for(int I=0;I<3;++I) Plan.Morale.Add(J.PreparationMorale(Choice,I));
        Plan.MoraleText=FString::Printf(TEXT("出陣士気　劉備%d / 関羽%d / 張飛%d"),Plan.Morale[0],Plan.Morale[1],Plan.Morale[2]);
        Plan.Cost=FString::Printf(TEXT("出陣 %d%s\n兵糧 %d / 資金 %d"),Deployed,Cost.escort ? TEXT(" ＋護衛600"):TEXT(""),Plan.Food,Plan.Gold);
        if(J.campaignFlow) Plan.Cost=FString::Printf(TEXT("出陣%d%s ／ 携行糧%d%s\n支払：糧%d・資金%d"),Deployed,Cost.escort ? TEXT("＋護衛600"):TEXT(""),Cost.food-(Choice==1 ? 1800:0)-(Cost.escort ? 900:0),
            Choice==1 ? TEXT("＋予備1,800"):TEXT(""),Plan.Food,Plan.Gold);
        Plan.Remaining=FString::Printf(TEXT("残兵 %d / 糧 %d\n残資金 %d"),Camp.troops-Deployed-Cost.escort,Camp.food-Plan.Food,Camp.gold-Plan.Gold);
        if(Cost.cargo) Plan.Description+=TEXT(" 積荷2,000と護衛用の糧900を含む。");
        Plan.bSelected=Choice==State->PreparationChoice;Plan.bAffordable=J.CanPrepare(Choice);
        if(!Plan.bAffordable) Plan.Remaining=TEXT("不足：")+Plan.Shortage;
        P.Plans.Add(MoveTemp(Plan));
    }
    const auto& First=P.Plans[0];const auto& Second=P.Plans[1];
    P.PlanDifference=FString::Printf(TEXT("案2は案1より  兵糧 %+d / 軍資金 %+d。初動の伏兵準備と、高い士気・携行糧を比べる。"),Second.Food-First.Food,Second.Gold-First.Gold);
    if(J.campaignFlow) P.PlanDifference=TEXT("開戦前の仕込みか、戦闘中の立て直しか。選んだ準備が、実際に使える手になる。");
    const auto& Selected=P.Plans[State->PreparationChoice];
    P.PlanReadiness=Selected.bAffordable ? TEXT("出陣条件を満たしている。選択中：")+Selected.Title:TEXT("出陣できない：")+Selected.Shortage;
    if(!Selected.bAffordable && P.Plans[1-State->PreparationChoice].bAffordable)
        P.PlanReadiness+=FString::Printf(TEXT("　案%dなら現在の資源で出陣できる。"),2-State->PreparationChoice);
    const TCHAR* Names[]={TEXT("劉備"),TEXT("関羽"),TEXT("張飛")};
    const FString Liu=J.IsRescue() ? TEXT("敵の旗が倒れても、季安の車が着くまでは終わらない。道に残す隊を、先に決めよう。"):
        J.Definition().kind==JourneyObjective::CaptureCamp ? TEXT("北橋か南橋を渡り、指揮所を取る。勝利後は本隊との共同作戦へ進む。二隊連鎖も成せば、決戦の合図が早くなる。"):
        J.mission==1 ? TEXT("積荷は1,200以上残して届ける。追撃より先に、糧車が通る場所を考えてほしい。"):
        J.mission==2 ? TEXT("二つの烽火を、同時に三指揮守る。片方に全員を集めず、誰を信じて残すか決めよう。"):
        TEXT("先鋒を崩すか、安全な出口を確保して道を開く。敵を倒すことと、役目を果たすことを一緒に考えよう。");
    FString Guan=!Selected.bAffordable ? TEXT("まず兵と糧を確かめよ。不足のある案で出ることはできぬ。別の案と、現在の蓄えを比べるのだ。"):
        J.CurrentPledge()==JourneyPledge::Preserve ? TEXT("九割を連れ帰ると言ったな。士気も、退く場所も要る。高い士気を選ぶ代わりに何を使うか、そこまで見て決めよ。"):
        State->PreparationChoice==0 ? TEXT("出陣時の伏兵準備を試みる。露見した時の動きも決めておけ。選ぶだけで、伏兵の成功を約束できるわけではない。"):
        TEXT("初期士気と携行糧を厚くする。その分の糧を使う。私の伏兵は戦場で準備することになる。主攻を急がせるな。");
    if(J.campaignFlow && Selected.bAffordable && J.CurrentPledge()==JourneyPledge::Preserve)
        Guan=TEXT("九割を連れ帰ると約束したな。先遣で敵の脇を取るか、補給を温存して退き際を支えるか。兵を守れる手順まで、考えよ。");
    if(J.campaignFlow && Selected.bAffordable && J.CurrentPledge()!=JourneyPledge::Preserve)
        Guan=State->PreparationChoice==0 ? TEXT("私を近くの森へ先に入れろ。準備は開戦前に済ませる。だが、敵が隣まで来ていれば隠れられぬ。地図で次の一手まで考えよ。"):
            TEXT("最初から糧を配り切らず、軍師の合図で回す。戦法を使って士気が落ちた時も考えよ。動き終えた隊が、また動けるわけではないぞ。");
    const FString Zhang=J.CurrentPledge()==JourneyPledge::Coordinate ? TEXT("皆をつなぐって言ったな！ 一人でも戦法は使えるが、二隊以上をつなぐ場所は、お前が作ってくれよ。"):
        J.CurrentPledge()==JourneyPledge::Preserve ? TEXT("帰れと言う合図も先に決めとけ。突っ込めだけじゃ、お前の約束は守れねえからな。"):
        J.HasConvoy() ? TEXT("前へ出る奴と、荷の横に残る奴。どっちも役目だってのは分かった。誰に任せるか、お前が呼べよ！"):
        J.mission==2 ? TEXT("離れていても、隣が危ねえ時は気にしてる。こっちを走らせるなら、元いた所も空くって覚えとけよ。"):
        TEXT("一人で戦法を撃つか、仲間へつなぐか。走る前に、その次の一手も教えてくれよ、軍師！");
    for(int I=0;I<3;++I)
    { FDialoguePresentation Advice;Advice.Speaker=I;Advice.Name=Names[I];Advice.Text=I==0 ? Liu:I==1 ? Guan:Zhang;P.PlanAdvice.Add(MoveTemp(Advice)); }
}
