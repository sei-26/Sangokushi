#include "TacticalTrial.h"
#include "TacticalState.h"
#include "JourneyAutoBattle.hpp"
using namespace frontline;

bool ATacticalField::PrepareDecisionBriefPreview(int Kind)
{
    if(Kind==0)
    {
        if(!PrepareStrategicDepthPreview(0)) return false;
        State->bNoPlayback=false;State->bAutoBattle=true;State->Realtime.paused=false;bPauseOnBattleAlert=true;
        RunRealtimeCycle(false);AdvancePresentation(60);return State->HUD.Decision.bVisible;
    }
    if(Kind<1 || Kind>2 || !PrepareStrategicDepthPreview(2)) return false;
    if(Kind==1)
    {
        Issue(ETacticalCommand::AutoBattle);AdvanceRealtime(12.1f);AdvancePresentation(60);
        return IsAutoBattle() && !State->HUD.Decision.bVisible;
    }
    auto& J=State->Journey;
    for(int Cycle=0;Cycle<60 && J.stage==JourneyStage::Battle;++Cycle)
    {
        auto Plan=PlanJourneyAutoBattle(J);
        if(!Plan.tactics.empty()) J.Execute(Plan.tactics);
        if(J.stage==JourneyStage::Battle) Plan.movement.Resolve(J);
    }
    Refresh();return J.won && J.stage==JourneyStage::Debrief;
}

bool ATacticalField::DecisionBriefSmokeTest()
{
    auto Check=[](bool V,const TCHAR* Why) { if(!V) UE_LOG(LogTemp,Error,TEXT("GUNYU_DECISION_BRIEF: FAIL %s"),Why);return V; };
    if(!Check(PrepareDecisionBriefPreview(0),TEXT("the real captured flank opens a decision brief after playback"))) return false;
    std::vector<std::uint8_t> Before,After;const int Selected=State->Selected;
    if(!ExportSession(Before)) return false;Refresh();
    if(!Check(ExportSession(After) && Before==After && State->Selected==Selected,TEXT("reading the brief never selects an officer or consumes an action"))) return false;
    auto Brief=Presentation().Decision;
    if(!Check(!Brief.KeepPlan.IsEmpty() && !Brief.Takeover.IsEmpty() && !Brief.Ready.IsEmpty(),TEXT("continuation, intervention and actual remaining budgets are visible together"))) return false;
    // A completed action may change while an old button press is pending.
    // The new recommendation must be confirmed, never silently substituted.
    State->Journey.chain.spent[Brief.Officer]=true;Refresh();
    if(!Check(!UseDecisionBrief(Brief,true) && IsAutoBattle(),TEXT("a stale officer recommendation cannot silently take over"))) return false;
    Brief=Presentation().Decision;
    const auto Spent=State->Journey.chain.spent;const int Turn=State->Journey.turn,Revision=State->Journey.world.revision;
    if(!Check(Brief.Officer>=0 && !Spent[Brief.Officer] && UseDecisionBrief(Brief,true) &&
        !IsAutoBattle() && RealtimePaused() && State->Selected==Brief.Officer && State->Orders.empty(),TEXT("takeover selects a living unspent officer and waits for the player's own order"))) return false;
    if(!Check(State->Journey.turn==Turn && State->Journey.world.revision==Revision && State->Journey.chain.spent==Spent,
        TEXT("takeover cannot refund actions, grant a turn or move an army"))) return false;
    if(!Check(!UseDecisionBrief(Brief,false),TEXT("a dismissed brief cannot resume a different control state"))) return false;
    if(!Check(PrepareDecisionBriefPreview(0),TEXT("reopen the decision through real movement"))) return false;
    Brief=Presentation().Decision;const int ResumeTurn=State->Journey.turn;
    for(int I=0;I<7 && State->Realtime.speed<4.99;++I) Issue(ETacticalCommand::RealtimeSpeed);
    if(!Check(State->Realtime.speed==5. && Presentation().Decision.bVisible && State->Journey.turn==ResumeTurn,
        TEXT("auto decisions retain five-times speed control without advancing the battle"))) return false;
    if(!Check(UseDecisionBrief(Brief,false) && IsAutoBattle() && !RealtimePaused() && !Presentation().Decision.bVisible &&
        State->Journey.turn==ResumeTurn,TEXT("continuation resumes the existing auto clock without a free update"))) return false;
    if(!Check(PrepareDecisionBriefPreview(2),TEXT("earned Guangzong support and actual coalition victory reach the aftermath"))) return false;
    const auto& C=State->Journey.battleReview.grand.coalition;const auto& Voices=Presentation().Journey.BattleResponse;
    if(!Check(C.rescuedBy>=0 && C.reliefReturned>0 && Voices.Num()==3 && Voices[1].Speaker==C.rescuedBy && Voices[2].bProtagonist,
        TEXT("the quick aftermath shows Cao Cao, the actual rescuer and the protagonist"))) return false;
    UE_LOG(LogTemp,Display,TEXT("GUNYU_DECISION_BRIEF: PASS (observed change, immutable read, stale press, unspent takeover, paid budgets, resume and actual rescuer aftermath)"));return true;
}
