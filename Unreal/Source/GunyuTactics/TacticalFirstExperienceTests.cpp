#include "TacticalTrial.h"
#include "TacticalState.h"
#include "JourneySave.hpp"
#include "JourneyPreludeOpening.hpp"
using namespace frontline;

bool ATacticalField::FirstExperienceSmokeTest()
{
    auto Check=[](bool V,const TCHAR* Why) { if(!V) UE_LOG(LogTemp,Error,TEXT("GUNYU_FIRST_EXPERIENCE: FAIL %s"),Why);return V; };
    const bool Mode=bRealtimeCommand,Grand=bGrandHistoricalBattles,Council=bStartWithCouncil,Legacy=bLegacyJourney;
    const bool Playback=State->bNoPlayback,Alerts=bPauseOnBattleAlert,Standard=bStartOnStandardDifficulty;const int Mission=FirstMission,Policy=EnemyPolicy;
    bRealtimeCommand=true;bGrandHistoricalBattles=bStartWithCouncil=true;bLegacyJourney=false;
    FirstMission=0;EnemyPolicy=0;bStartOnStandardDifficulty=false;State->bNoPlayback=true;bPauseOnBattleAlert=false;
    std::vector<std::uint8_t> Before,After;
    for(int Choice:{0,1})
    {
        Issue(ETacticalCommand::Restart);
        if(!Check(CanChooseFirstRoad() && ExportSession(Before),TEXT("normal new campaign offers the road decision before deployment"))) return false;
        Refresh();
        if(!Check(!ChooseFirstRoad(2) && ExportSession(After) && Before==After,TEXT("reading and invalid choices do not advance or pay"))) return false;
        const auto Camp=State->Journey.world.cities[0];
        if(!Check(ChooseFirstRoad(Choice) && State->Journey.stage==JourneyStage::Preparation && MatchesPreludeOpening(State->Journey,Choice) &&
            State->PreparationChoice==Choice && State->Journey.world.armies.empty() && State->Journey.world.cities[0].food==Camp.food &&
            State->Journey.world.cities[0].troops==Camp.troops && State->Journey.trust==0,TEXT("the chosen observation becomes actual duties and supplies without free troops, spending or trust"))) return false;
        if(!ExportSession(Before)) return false;
        if(!Check(!ChooseFirstRoad(1-Choice) && ExportSession(After) && Before==After && ImportSession(Before) &&
            MatchesPreludeOpening(State->Journey,Choice),TEXT("double choice is refused and the real adopted plan survives reload"))) return false;
        Issue(ETacticalCommand::Deploy);
        if(!Check(State->Journey.world.cities[0].food==Camp.food-State->Journey.PreparationCosts(Choice).TotalFood(),TEXT("deployment pays its original normal cost once"))) return false;
        if(Choice==1)
        {
            if(!Check(Presentation().OpeningOrder.Action!=EOpeningOrderAction::Solo && State->Realtime.orders[0].kind==JourneyDirective::March,
                TEXT("the exit road keeps its actual Liu Bei march instead of forcing an attack lesson"))) return false;
            continue;
        }
        const auto First=Presentation().OpeningOrder;
        if(!Check(First.Action==EOpeningOrderAction::Solo && State->BattleChoices[0].orders.size()==1,TEXT("forest plan offers a genuinely usable solo tactic"))) return false;
        if(!ExportSession(Before)) return false;Refresh();
        if(!Check(ExportSession(After) && Before==After,TEXT("the first-strike explanation is read-only"))) return false;
        State->Orders={{1,3,ChainTactic::Ambush,-1}};Refresh();if(!ExportSession(Before)) return false;
        if(!Check(!UseFirstStrike(First.SoloVersion,First.SoloTarget) && ExportSession(After) && Before==After,
            TEXT("a stale first-strike click never clears the player's reserved tactic"))) return false;
        State->Orders.clear();Refresh();const auto Ready=Presentation().OpeningOrder;
        ChainOrder Opening;
        for(const auto& Plan:State->BattleChoices) if(!Plan.orders.empty() && Plan.orders.front().tactic==ChainTactic::Lure) Opening=Plan.orders.front();
        if(!Check(Opening.army>=0,TEXT("the normal first battlefield has a real lure that can lead into the ambush"))) return false;
        const auto Forecast=State->Journey.Preview({Opening});
        const int Troops=State->Journey.world.armies[3].troops,Turn=State->Journey.turn,Origin=State->Journey.world.armies[Opening.army].tile;
        State->bNoPlayback=false;
        if(!Check(UseFirstStrike(Ready.SoloVersion,Ready.SoloTarget) && IsPlaying() && State->Journey.lastChain.completed==1 &&
            Forecast.completed==1 && Troops-State->Journey.world.armies[3].troops==Forecast.events[0].damage &&
            State->Journey.world.armies[3].tile==Origin && State->Journey.chain.spent[Opening.army],
            TEXT("one decision fires the exact real single tactic and pays its action"))) return false;
        for(int A=0;A<3;++A) if(A!=Opening.army)
            if(!Check(!State->Journey.chain.spent[A],TEXT("other officers retain their attacks for the player's next decision"))) return false;
        if(!Check(!UseFirstStrike(Ready.SoloVersion,Ready.SoloTarget),TEXT("a double click cannot fire during playback"))) return false;
        AdvancePresentation(60);
        if(!Check(!IsPlaying() && RealtimePaused() && State->Journey.turn==Turn,TEXT("the first strike ends paused without an extra enemy cycle"))) return false;
        if(!Check(Presentation().CommandGuide.Contains(TEXT("未行動")),TEXT("after the real solo tactic the guide points to the remaining officers"))) return false;
        const auto Spent=State->Journey.chain.spent;
        for(int Officer=0;Officer<3;++Officer) State->Journey.chain.spent[Officer]=true;
        Refresh();if(!ExportSession(Before)) return false;Refresh();
        if(!Check(Presentation().CommandGuide.Contains(TEXT("1周期")) && ExportSession(After) && Before==After,
            TEXT("with all actions spent the guide explains the real cycle control without advancing or paying"))) return false;
        State->Journey.chain.spent=Spent;Refresh();
        const auto GuardOrders=State->Realtime;
        State->Realtime.Queue(State->Journey,State->Selected,JourneyDirective::Hold);Refresh();
        if(!Check(Presentation().bGuardOrder && Presentation().CommandGuide.Contains(TEXT("固守解除")),TEXT("a continuing guard is explained before cycle recovery"))) return false;
        State->Realtime=GuardOrders;Refresh();
        State->bNoPlayback=true;Issue(ETacticalCommand::AutoBattle);
        for(int Cycle=0;Cycle<24 && State->Journey.stage==JourneyStage::Battle;++Cycle)
        { AdvanceRealtime(12.1f);AdvancePresentation(60);AdvanceRealtime(.1f);AdvancePresentation(60); }
        if(IsAutoBattle()) StopAutoBattle();
        if(!Check(State->Journey.won && Presentation().Journey.bFirstReplyHidden && CanIssue(ETacticalCommand::Continue),
            TEXT("the actual first victory starts with character responses and an optional next conversation"))) return false;
        if(!ExportSession(Before)) return false;Action(EKeys::One);
        if(!Check(ExportSession(After) && Before==After,TEXT("an unseen numbered vow cannot be committed by a stray key"))) return false;
        RevealFirstReply();
        if(!Check(!Presentation().Journey.bFirstReplyHidden && Presentation().Journey.bNeedsReply && ExportSession(After) && Before==After,
            TEXT("opening the conversation reveals conditions without changing the save, resources or relationships"))) return false;
        Action(EKeys::One);
        if(!Check(!State->Journey.NeedsStoryReply() && State->Journey.history[0].story.reply==JourneyPledge::Preserve,
            TEXT("the visible reply commits the normal once-only promise"))) return false;
        if(!ExportSession(Before)) return false;Action(EKeys::One);
        if(!Check(ExportSession(After) && Before==After && ImportSession(Before) && !Presentation().Journey.bNeedsReply,
            TEXT("a committed vow persists and is never awarded twice"))) return false;
        Issue(ETacticalCommand::Continue);
        if(!Check(State->Journey.mission==YellowTurbanMission && State->Journey.CurrentPledge()==JourneyPledge::Preserve,
            TEXT("the same vow reaches the actual next historical battle"))) return false;
    }
    bRealtimeCommand=Mode;bGrandHistoricalBattles=Grand;bStartWithCouncil=Council;bLegacyJourney=Legacy;
    State->bNoPlayback=Playback;bPauseOnBattleAlert=Alerts;bStartOnStandardDifficulty=Standard;FirstMission=Mission;EnemyPolicy=Policy;Issue(ETacticalCommand::Restart);
    UE_LOG(LogTemp,Display,TEXT("GUNYU_FIRST_EXPERIENCE: PASS (real road choices, paid deployment, exact solo tactic, reservations, normal playback, actual victory, optional conversation, keyboard safety, saved once-only vow and next chapter)"));
    return true;
}
