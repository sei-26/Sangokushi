#include "TacticalTrial.h"
#include "TacticalState.h"
#include "JourneyPreludeOpening.hpp"
using namespace frontline;
namespace
{
    ChainOrder FirstOrder(const FJourneyState& S)
    {
        // A legal lure at the start of an existing combo is also a legal solo
        // decision. It brings the enemy towards the waiting ambush, without
        // requiring the first-time player to enter a withdrawal destination.
        for(const auto& Plan:S.BattleChoices)
            if(Plan.target==3 && !Plan.orders.empty() && Plan.orders.front().tactic==ChainTactic::Lure)
                return Plan.orders.front();
        if(!S.BattleChoices.empty() && S.BattleChoices[0].orders.size()==1 && S.BattleChoices[0].target==3 && S.BattleChoices[0].damage>0)
            return S.BattleChoices[0].orders.front();
        return {};
    }
}

bool ATacticalField::CanChooseFirstRoad() const
{
    const auto& J=State->Journey;
    return !IsPlaying() && J.campaignFlow && J.shortPrelude && J.mission==0 && J.stage==JourneyStage::Encounter;
}

bool ATacticalField::ChooseFirstRoad(int Choice)
{
    if(!CanChooseFirstRoad() || Choice<0 || Choice>1) return false;
    // Meet and adopt on a candidate: a rejected choice cannot advance the story.
    auto Candidate=std::make_unique<StrategistJourney>(State->Journey);
    Candidate->Meet();
    if(!AdoptPreludeOpening(*Candidate,Choice)) return false;
    State->Journey=std::move(*Candidate);State->PreparationChoice=Choice;State->DialogueIndex=2;
    State->bConfirmEndTurn=State->bConfirmWithdrawal=false;
    State->Report=TEXT("道を読む判断を、初陣の配置へつないだ。出陣するまでは兵や糧を使わず、軍議で選び直せる。");
    Refresh();CheckpointJourney(true);return true;
}

void ATacticalField::RevealFirstReply()
{
    if(IsPlaying() || !State->HUD.Journey.bFirstReplyHidden) return;
    State->bFirstReplyRevealed=true;Refresh();
}

void ATacticalField::BuildFirstStrike()
{
    auto& H=State->HUD;const auto& J=State->Journey;
    if(!J.campaignFlow || !J.shortPrelude || J.mission!=0 || J.stage!=JourneyStage::Battle ||
       J.turn!=1 || J.preparation!=0 || !State->BeforeBattle || !MatchesPreludeOpening(*State->BeforeBattle,0) ||
       IsPlaying() || State->bAutoBattle || State->OpeningOfficer<0 || State->Input!=ETacticalInput::Select ||
       !State->Orders.empty() || State->ChainSignal.Armed() || State->bRealtimeStepping || State->bRealtimeChainRequested) return;
    bool Used=false;for(int A=0;A<3;++A) Used=Used || J.chain.spent[A];
    if(Used)
    {
        if(J.lastChain.completed==1 && J.chain.pressured[3])
            H.BattleChoicePrompt=TEXT("一手が届いた。次は仲間の戦法を比べよう");
        return;
    }
    if(J.world.revision!=State->OpeningRevision) return; // Free movement or another decision takes precedence over the lesson.
    if(H.BattleChoices.IsEmpty() || State->BattleChoices.empty() || State->Target!=3) return;
    const auto Order=FirstOrder(*State);
    if(Order.army<0) return;
    const auto& Def=TacticChain::Definition(Order.tactic);
    const auto Preview=J.Preview({Order});
    if(Def.unique || !Def.targetsEnemy || Preview.completed!=1 || Preview.events.size()!=1) return;
    auto& P=H.OpeningOrder;P.Officer=Order.army;P.Tile=J.world.armies[Order.army].tile;
    P.Name=JourneyText(J.world.generals[J.world.armies[Order.army].general].name);
    P.Place=TEXT("最初は、一隊の戦法から");
    int Loss=0;for(const auto& Change:Preview.events[0].changes) if(Change.army==Order.army) Loss=FMath::Max(0,J.world.armies[Order.army].troops-Change.troops);
    P.Instruction=P.Name+TEXT("「")+JourneyText(Def.name)+TEXT("」。")+
        (Order.tactic==ChainTactic::Lure ? TEXT("敵を森の伏兵へ寄せる。後退先も今の捕捉から選ぶ。") : TEXT("先鋒に隙を作る。"))+
        FString::Printf(TEXT("士気%d・味方損耗%d。仲間の行動は残る。"),Def.moraleCost,Loss);
    P.ActionLabel=P.Name+TEXT("に、この一手を託す");P.Action=EOpeningOrderAction::Solo;
    P.SoloVersion=H.BattleChoiceVersion;P.SoloTarget=State->Target;
}

bool ATacticalField::UseFirstStrike(int ExpectedVersion,int ExpectedTarget)
{
    if(IsPlaying() || ExpectedTarget!=3) return false;
    Refresh();const auto P=State->HUD.OpeningOrder;
    if(P.Action!=EOpeningOrderAction::Solo || P.SoloVersion!=ExpectedVersion || P.SoloTarget!=ExpectedTarget) return false;
    if(BattleChoiceContext(State->Journey,State->Target)!=State->BattleChoiceKey) return false;
    const auto Order=FirstOrder(*State);
    if(Order.army<0 || State->Journey.Preview({Order}).completed!=1) return false;
    const int Selected=State->Selected;const auto Tactic=State->SelectedTactics[Order.army];
    State->Selected=Order.army;State->SelectedTactics[Order.army]=Order.tactic;State->Orders={Order};
    if(FireManualTactics()) return true; // The normal reservation/fire pipeline pays every cost.
    State->Orders.clear();State->Selected=Selected;State->SelectedTactics[Order.army]=Tactic;Refresh();return false;
}

void ATacticalController::ChooseEncounterRoad(int Choice)
{
    if(!Field->ChooseFirstRoad(Choice)) return;
    OpenJourneyEntry();
    CouncilExchange=Field->ReadPreludeCouncil(Choice);CouncilTopic=-1;CouncilPage=0;
    CouncilElapsed=CouncilLineTime=0;CouncilCachePlan=Choice;
    bCouncilCacheRescue=Field->Presentation().Journey.bRescue;bCouncilCacheSouth=Field->Presentation().Journey.Map.bSouthern;
}
