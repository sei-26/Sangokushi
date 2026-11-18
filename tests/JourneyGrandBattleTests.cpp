#include "JourneyAutoBattle.hpp"
#include "JourneySave.hpp"
#include "JourneyArchive.hpp"
#include "JourneyEpisode.hpp"
#include <cassert>
#include <iostream>
#include <memory>
using namespace frontline;

static std::unique_ptr<JourneySaveData> DeployGrand(int mission,JourneyDifficulty difficulty=JourneyDifficulty::Story)
{
    auto save=std::make_unique<JourneySaveData>();
    auto& journey=save->journey;
    journey.ResetCampaign();journey.Meet();journey.grandCampaign=true;
    journey.mission=mission;
    if(mission==YangrenMission) journey.coalitionPlan=CoalitionPlan::Advance;
    for(int earlier=0;earlier<mission;++earlier)
    {
        journey.history[earlier].completed=journey.history[earlier].won=true;
        journey.history[earlier].campaignFlow=true;
    }
    assert(journey.SelectDifficulty(difficulty));
    save->beforeBattle=journey;
    assert(journey.Prepare(1,0));
    return save;
}

static void RoundTrip(const JourneySaveData& save)
{
    std::vector<std::uint8_t> bytes;
    auto loaded=std::make_unique<JourneySaveData>();
    assert(EncodeJourneySave(save,bytes) && DecodeJourneySave(bytes,*loaded));
    assert(loaded->journey.grandCampaign && loaded->journey.world.armies.size()==save.journey.world.armies.size());
    assert(loaded->journey.battleReview.grand.openedOn==save.journey.battleReview.grand.openedOn);
}

static void PlayHistoricalCampaign()
{
    auto session=std::make_unique<JourneySaveData>();auto& journey=session->journey;
    journey.ResetCampaign();journey.Meet();journey.grandCampaign=true;
    assert(journey.SelectDifficulty(JourneyDifficulty::Story));
    int completed=0;
    for(int transition=0;transition<30;++transition)
    {
        if(journey.CanChooseCoalitionPlan()) assert(journey.ChooseCoalitionPlan(CoalitionPlan::Advance));
        else if(journey.CanChooseLuoyangPlan(LuoyangPlan::SouthGate)) assert(journey.ChooseLuoyangPlan(LuoyangPlan::SouthGate));
        else if(journey.stage==JourneyStage::Preparation)
        {
            if(journey.world.cities[0].troops<4500) journey.Reinforce();
            session->beforeBattle=journey;
            assert(journey.Prepare(1,journey.enemyPolicy));
            for(int cycle=0;cycle<32 && journey.stage==JourneyStage::Battle;++cycle)
            {
                auto plan=PlanJourneyAutoBattle(journey);
                if(!plan.tactics.empty()) journey.Execute(plan.tactics);
                if(journey.stage==JourneyStage::Battle) assert(plan.movement.Resolve(journey));
            }
            std::cerr<<"historical grand campaign mission="<<journey.mission<<" won="<<journey.won<<" camp="<<journey.world.cities[0].troops<<"\n";
            assert(journey.won);++completed;RoundTrip(*session);
        }
        else if(journey.NeedsStoryReply()) assert(journey.ChooseStoryReply(JourneyPledge::Coordinate));
        else if(journey.CanContinue()) assert(journey.Continue());
        else break;
    }
    assert(completed==7 && journey.history[YellowTurbanMission].review.grand.active && journey.history[YangrenMission].review.grand.active);
}

int main()
{
    for(int mission:{YellowTurbanMission,YangrenMission})
    {
        auto session=DeployGrand(mission);auto& journey=session->journey;
        assert(journey.HasGrandBattle() && journey.world.armies.size()==GrandBattleArmyCount &&
            journey.world.generals.size()==GrandBattleArmyCount && journey.battleReview.grand.alliedStarted==15000);
        assert(BuildJourneyEpisode(*session->beforeBattle).lines.size()>=6);
        RoundTrip(*session);
        const int left=journey.world.armies[6].tile,right=journey.world.armies[7].tile;
        assert(journey.EndTurn());
        assert(journey.world.armies[6].tile!=left && journey.world.armies[7].tile!=right);
        auto opened=DeployGrand(mission);auto& battle=opened->journey;
        battle.world.armies[3].troops=battle.world.armies[4].troops=0;
        battle.world.armies[0].tile=battle.world.armies[0].target=Campaign::At(mission==YangrenMission ? 10:11,8);
        battle.RefreshIntelligence();
        assert(battle.Move(0,battle.goal));
        assert(battle.stage==JourneyStage::Battle && battle.battleReview.grand.openedBy==0 && battle.battleReview.grand.openedOn==1);
        RoundTrip(*opened);
        auto lost=std::make_unique<JourneySaveData>(*opened);
        lost->journey.world.armies[5].troops=0;
        assert(lost->journey.EndTurn() && !lost->journey.won && lost->journey.stage==JourneyStage::Debrief);
        RoundTrip(*lost);
        assert(lost->journey.RetryAtCouncil(*lost->beforeBattle) && lost->journey.grandCampaign &&
            lost->journey.Prepare(1,0) && lost->journey.world.armies.size()==GrandBattleArmyCount);
        battle.world.armies[GrandCommander].troops=0;
        battle.world.armies[5].tile=battle.world.armies[5].target=battle.GrandRallyTile();
        battle.world.armies[6].troops-=1000;
        assert(battle.EndTurn() && battle.won && battle.history[mission].review.grand.alliedLosses>=1000);
        RoundTrip(*opened);
        assert(BuildJourneyEpisode(battle).lines.size()>=4);
        if(mission==YellowTurbanMission)
        {
            assert(battle.Continue() && battle.ChooseCoalitionPlan(CoalitionPlan::Advance));
            RoundTrip(*opened);
            opened->beforeBattle=battle;
            assert(battle.Prepare(1,0) && !battle.HasGrandBattle() && battle.world.armies.size()==5 && battle.world.generals.size()==5);
            RoundTrip(*opened);
        }
    }
    auto legacy=DeployGrand(YellowTurbanMission);
    auto headquarters=DeployGrand(YellowTurbanMission);
    auto& assault=headquarters->journey;
    assault.world.armies[0].tile=assault.world.armies[0].target=Campaign::At(12,6);
    assault.world.armies[0].morale=100;
    assault.RefreshIntelligence();
    headquarters->target=GrandCommander;
    const int before=assault.world.armies[GrandCommander].troops;
    const auto strike=assault.Execute({{0,GrandCommander,ChainTactic::Disrupt,-1}});
    assert(strike.completed==1 && assault.world.armies[GrandCommander].troops<before);
    RoundTrip(*headquarters);
    for(const auto& plan:assault.ReadEnemyPlans())
        if(plan.army==GrandCommander) assert(plan.next==plan.from && plan.intent==JourneyEnemyIntent::HoldCamp);
    legacy->journey=*legacy->beforeBattle;legacy->journey.grandCampaign=false;legacy->beforeBattle=legacy->journey;
    assert(legacy->journey.Prepare(1,0) && !legacy->journey.HasGrandBattle());
    std::vector<std::uint8_t> oldBytes;
    unsigned magic=0x53594e47,version=22,checksum=0;
    JourneyArchive archive{false,oldBytes};archive.version=22;
    archive(magic,version,checksum);archive(*legacy);
    checksum=2166136261u;
    for(std::size_t i=12;i<oldBytes.size();++i) { checksum^=oldBytes[i];checksum*=16777619u; }
    for(int i=0;i<4;++i) oldBytes[8+i]=static_cast<std::uint8_t>(checksum>>(i*8));
    auto restored=std::make_unique<JourneySaveData>();
    assert(DecodeJourneySave(oldBytes,*restored) && !restored->journey.HasGrandBattle() &&
        !restored->journey.grandCampaign && restored->journey.world.armies.size()==5);
    const int oldBudget=restored->journey.TurnBudget();
    const auto oldTitle=std::u32string(restored->journey.MissionName());
    const auto oldObjective=restored->journey.ObjectiveBrief();
    restored->journey.grandCampaign=true;
    assert(!restored->journey.PlansGrandBattle() && restored->journey.TurnBudget()==oldBudget &&
        restored->journey.MissionName()==oldTitle && restored->journey.ObjectiveBrief()==oldObjective);
    for(int mission:{YellowTurbanMission,YangrenMission})
        for(auto difficulty:{JourneyDifficulty::Story,JourneyDifficulty::Standard})
        {
            auto session=DeployGrand(mission,difficulty);auto& battle=session->journey;
            for(int cycle=0;cycle<32 && battle.stage==JourneyStage::Battle;++cycle)
            {
                auto plan=PlanJourneyAutoBattle(battle);
                if(!plan.tactics.empty()) battle.Execute(plan.tactics);
                if(battle.stage==JourneyStage::Battle) assert(plan.movement.Resolve(battle));
            }
            std::cerr<<"grand mission="<<mission<<" difficulty="<<int(difficulty)<<" won="<<battle.won<<" turn="<<battle.turn<<" opened="<<battle.battleReview.grand.openedOn<<" main="<<battle.world.armies[5].troops<<" boss="<<battle.world.armies[GrandCommander].troops<<"\n";
            assert(battle.won);
            RoundTrip(*session);
        }
    PlayHistoricalCampaign();
    std::cout<<"Grand battles: PASS\n";
}
