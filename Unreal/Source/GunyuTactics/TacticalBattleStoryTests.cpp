#include "TacticalTrial.h"
#include "TacticalState.h"
using namespace frontline;
void ATacticalField::PrepareBattleStoryPreview(bool bCostly)
{
    const bool SavedPlayback=State->bNoPlayback;State->bNoPlayback=true;
    FirstMission=0;EnemyPolicy=1;bStartWithCouncil=true;bRealtimeCommand=false;Issue(ETacticalCommand::Restart);
    for(int I=0;I<3;++I) Issue(ETacticalCommand::Dialogue);
    Issue(ETacticalCommand::PromiseRoad);Issue(ETacticalCommand::PlanSupplies);Issue(ETacticalCommand::Deploy);
    const int Dest[]={Campaign::At(9,11),Campaign::At(9,9),Campaign::At(9,10)};
    for(int I=0;I<3;++I) { SelectOfficer(I);Issue(ETacticalCommand::Move);CommandAt(Dest[I]); }
    // QA isolates two real resolutions from controlled casualty/target fixtures.
    State->Journey.world.armies[3].troops=bCostly ? 460:600;
    if(bCostly) State->Journey.world.armies[1].troops=200;
    State->Journey.RefreshIntelligence();Refresh();
    if(bCostly) { SelectOfficer(0);Issue(ETacticalCommand::Strike); }
    else
    {
        for(int I=0;I<3;++I) { SelectOfficer(I);SelectTactic(int(ChainTactic::Feint));Issue(ETacticalCommand::QueueTactic); }
        Issue(ETacticalCommand::Execute);
    }
    Issue(ETacticalCommand::TalkGuan);State->bNoPlayback=SavedPlayback;
}
bool ATacticalField::BattleStorySmokeTest()
{
    auto Check=[](bool V,const TCHAR* Why) { if(!V) UE_LOG(LogTemp,Error,TEXT("GUNYU_BATTLE_STORY: FAIL %s"),Why);return V; };
    const int SavedMission=FirstMission,SavedPolicy=EnemyPolicy;
    const bool SavedCouncil=bStartWithCouncil,SavedRealtime=bRealtimeCommand,SavedNoPlayback=State->bNoPlayback;
    State->bNoPlayback=true;PrepareBattleStoryPreview(false);
    if(!Check(State->Journey.won && Presentation().Journey.bNeedsReply && !CanIssue(ETacticalCommand::Continue),TEXT("victory opens a real protagonist decision before the next council"))) return false;
    for(auto Command:{ETacticalCommand::TalkLiu,ETacticalCommand::TalkGuan,ETacticalCommand::TalkZhang})
    {
        Issue(Command);bool Hero=false;for(const auto& L:Presentation().Journey.Conversation) Hero|=L.bProtagonist;
        if(!Check(Hero && Presentation().Journey.Conversation.Num()>=4 && Presentation().Journey.Dialogue.Contains(TEXT("主人公")),TEXT("each officer converses with the protagonist instead of a one-way tutorial"))) return false;
    }
    const auto Trust=State->Journey.officerTrust;Action(EKeys::Two);
    if(!Check(State->Journey.officerTrust==Trust && CanIssue(ETacticalCommand::Continue) && !Presentation().Journey.bNeedsReply,TEXT("words choose a future duty without giving free relationship points"))) return false;
    Issue(ETacticalCommand::StoryReplyPreserve);
    if(!Check(State->Journey.history[0].story.reply==JourneyPledge::Coordinate,TEXT("the same reply cannot be farmed or rewritten"))) return false;
    std::vector<std::uint8_t> Bytes;
    if(!Check(ExportSession(Bytes) && ImportSession(Bytes) && State->Journey.officerTrust==Trust && CanIssue(ETacticalCommand::Continue),TEXT("save restores the resolved relationship and committed reply once"))) return false;
    Issue(ETacticalCommand::Continue);
    if(!Check(Presentation().Journey.PledgeText.Contains(TEXT("二隊")) && Presentation().Journey.Dialogue.Contains(TEXT("主人公")),TEXT("the next council remembers what this protagonist said"))) return false;
    Issue(ETacticalCommand::PlanAmbush);Issue(ETacticalCommand::Deploy);
    for(int I=0;I<3;++I)
        if(!Check(State->Journey.world.armies[I].morale==90+State->Journey.StoryMoraleBonus(I),TEXT("individual trust changes actual deployment morale"))) return false;
    if(!Check(Presentation().Objective.Contains(TEXT("主人公の約束")),TEXT("the real battlefield keeps the commitment visible"))) return false;
    const auto BeforeTrust=State->Journey.officerTrust;State->Journey.world.armies[1].troops=200;
    Issue(ETacticalCommand::Withdraw);Issue(ETacticalCommand::Withdraw);
    if(!Check(State->Journey.officerTrust!=BeforeTrust,TEXT("breaking a battle commitment produces an actual consequence"))) return false;
    Issue(ETacticalCommand::Retry);
    if(!Check(State->Journey.officerTrust==BeforeTrust && State->Journey.CurrentPledge()==JourneyPledge::Coordinate,TEXT("retry restores pre-battle trust and keeps the original commitment"))) return false;
    Issue(ETacticalCommand::Deploy);State->Journey.world.armies[3].troops=State->Journey.world.armies[4].troops=0;State->Journey.RefreshIntelligence();Refresh();
    for(int I=0;I<6 && State->Journey.stage==JourneyStage::Battle;++I) Issue(ETacticalCommand::EndTurn);
    if(!Check(State->Journey.won && !State->Journey.history[1].story.pledgeKept && State->Journey.history[1].story.trustDelta[2]<0,TEXT("mission victory does not erase a broken promise to the officers"))) return false;
    Issue(ETacticalCommand::TalkZhang);
    if(!Check(Presentation().Journey.Dialogue.Contains(TEXT("合図")) && Presentation().Journey.StoryFacts.Contains(TEXT("果たせなかった")),TEXT("dramatic dialogue and factual consequence agree"))) return false;
    PrepareBattleStoryPreview(true);
    if(!Check(State->Journey.history[0].story.costly && State->Journey.officerTrust[1]<0 && Presentation().Journey.Dialogue.Contains(TEXT("戻った列")),TEXT("a costly victory changes Guan Yu's response and personal trust"))) return false;
    FirstMission=SavedMission;EnemyPolicy=SavedPolicy;bStartWithCouncil=SavedCouncil;bRealtimeCommand=SavedRealtime;State->bNoPlayback=SavedNoPlayback;Issue(ETacticalCommand::Restart);
    UE_LOG(LogTemp,Display,TEXT("GUNYU_BATTLE_STORY: PASS (protagonist dialogue/choice, distinct victories, officer trust, real vow, actual morale, save, no click farming, retry, promise-driven next council)"));return true;
}
