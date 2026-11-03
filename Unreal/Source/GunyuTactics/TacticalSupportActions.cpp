#include "TacticalTrial.h"
#include "TacticalState.h"
using namespace frontline;
bool ATacticalField::BuildSupportActionChoices()
{
    const auto& J=State->Journey;auto& H=State->HUD;
    if(State->bRealtimeStepping || State->bRealtimeChainRequested) return false;
    auto Key=JourneySupportActionContext(J,State->Realtime,State->Selected);Key.push_back(bRealtimeCommand);
    if(Key!=State->SupportActionKey)
    { State->SupportActionKey=Key;State->SupportActions=ReadJourneySupportActions(J,State->Realtime,State->Selected);++State->BattleChoiceVersion; }
    if(State->SupportActions.empty()) return false;
    State->bSupportBattleChoices=true;State->BattleChoiceKey.clear();State->BattleChoices.clear();
    H.BattleChoiceVersion=State->BattleChoiceVersion;
    for(const auto& C:State->SupportActions)
    {
        State->BattleChoices.push_back(C.plan);const auto& O=C.plan.orders.front();
        const auto* Seen=J.intelligence[0].Find(O.enemy);if(!Seen) continue;
        const FString Name=JourneyText(J.world.generals[J.world.armies[O.army].general].name);
        const FString Enemy=JourneyText(J.world.generals[Seen->observed.general].name);
        const FString Side=C.goal==J.goal ? TEXT("北の烽火"):TEXT("南の烽火");
        const FString Guard=JourneyText(J.world.generals[J.world.armies[C.guard].general].name);
        const bool Team=C.plan.orders.size()>1;
        H.BattleChoiceTarget=Enemy;H.BattleChoicePrompt=Side+TEXT(" / ")+Enemy;
        FTacticalPresentation::FBattleChoice P;P.Actor=O.army;
        const bool Lure=O.tactic==ChainTactic::Lure,Push=C.enemyTo!=Seen->observed.tile && !Lure;
        const bool Defeat=C.plan.damage>=Seen->observed.troops;
        P.Title=Lure ? Name+TEXT("を囮に回す"):Defeat ? Name+TEXT("で敵を討つ"):Push ? Name+TEXT("で押し返す"):
            O.tactic==ChainTactic::Feint ? Name+TEXT("で隙を作る"):O.tactic==ChainTactic::Disrupt ? Name+TEXT("で構えを崩す"):
            O.tactic==ChainTactic::Breakthrough ? Name+TEXT("で突撃する"):Name+TEXT("の急襲に託す");
        if(Team) P.Title=Name+TEXT("→")+Guard+TEXT("で連携");
        else if(C.followup) P.Title=Guard+TEXT("に隙を託す");
        P.Outcome=Lure && !Team ? TEXT("予測：敵を誘導"):FString::Printf(TEXT("予測：敵兵−%d・士気−%d"),C.plan.damage,C.enemyMoraleLoss);
        P.Outcome+=FString::Printf(TEXT("\n烽火の隣接敵%d→%d"),C.threatsBefore,C.threatsAfter);
        P.Cost=FString::Printf(TEXT("士気%d"),C.plan.moraleCost);
        if(C.plan.friendlyLoss>0) P.Cost+=FString::Printf(TEXT("・味方兵%d"),C.plan.friendlyLoss);
        P.Cost+=C.actorTo!=J.world.armies[O.army].tile ? TEXT(" / 攻撃と移動を使う"):TEXT(" / 攻撃を使う");
        if(J.chain.concealed[O.army]) P.Cost+=TEXT("・潜伏解除");
        if(C.guardActs)
        {
            P.Cost=(Team ? FString(TEXT("士気計")):FString(TEXT("士気")))+FString::FromInt(C.plan.moraleCost);
            if(C.plan.friendlyLoss>0) P.Cost+=FString::Printf(TEXT("・味方兵%d"),C.plan.friendlyLoss);
            if(C.actorTo!=J.world.armies[O.army].tile) P.Cost+=TEXT(" / 移動あり");
            P.Cost+=TEXT("\n")+Guard+TEXT("も攻撃・今の固守は不可");
        }
        P.Detail=Name+TEXT("「")+JourneyText(TacticChain::Name(O.tactic))+TEXT("」\n")+P.Outcome+TEXT("\n")+P.Cost;
        if(Team)
        {
            P.Detail.Reset();for(const auto& Step:C.plan.orders)
            {
                if(!P.Detail.IsEmpty()) P.Detail+=TEXT(" → ");
                P.Detail+=JourneyText(J.world.generals[J.world.armies[Step.army].general].name)+TEXT("「")+JourneyText(TacticChain::Name(Step.tactic))+FString::Printf(TEXT("」士気%d"),TacticChain::Definition(Step.tactic).moraleCost);
            }
            P.Detail+=TEXT("\n2連携・2手目は火力125％\n")+P.Outcome+TEXT("\n")+P.Cost;
        }
        if(C.followup) P.Detail+=TEXT("\n先の一手は発動済み。これは別の単発戦法で連携倍率100％。隙の仕込みによる威力増は予測値に反映。2連携には数えない。");
        P.Detail+=TEXT("\n")+Guard+TEXT("は烽火に残る。捕捉からの予測で、未知の敵や障害で後続が止まることもある。");
        if(C.guardActs) P.Detail+=TEXT("\n守備役の攻撃を使うと、今の指揮では固守できない。敵が残れば維持が途切れるおそれ。常設の固守命令は保持し、通常の行動回復後に続ける。");
        for(const auto& Step:C.plan.orders) if(J.chain.concealed[Step.army]) { P.Detail+=TEXT("\n参加する潜伏隊は戦法で潜伏が解ける。");break; }
        P.Detail+=FString::Printf(TEXT("\n自隊(%d,%d)・敵(%d,%d)へ。"),C.actorTo%Width,C.actorTo/Width,C.enemyTo%Width,C.enemyTo/Width);
        P.Detail+=C.clearsThreat ? TEXT("隣接する敵を減らす見込み。維持の成否は次の指揮で確認する。"):TEXT("敵は烽火の近くに残る。配置や構えを変え、次の手につなぐ。");
        H.BattleChoices.Add(MoveTemp(P));
    }
    return true;
}
