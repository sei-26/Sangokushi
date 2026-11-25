#include "JourneyAutoBattle.hpp"
#include "JourneySave.hpp"
#include "JourneyArchive.hpp"
#include "JourneyEpisode.hpp"
#include "JourneyBattlefield.hpp"
#include "JourneyPlanExample.hpp"
#include "JourneyCampaign.hpp"
#include <cassert>
#include <iostream>
#include <memory>
using namespace frontline;

static std::unique_ptr<JourneySaveData> Deploy(bool strategic=true,JourneyDifficulty difficulty=JourneyDifficulty::Story)
{
    auto save=std::make_unique<JourneySaveData>();auto& j=save->journey;
    j.ResetCampaign();j.Meet();j.mission=YellowTurbanMission;j.grandCampaign=true;j.grandStrategyCampaign=strategic;
    for(int m=0;m<YellowTurbanMission;++m) j.history[m].completed=j.history[m].won=j.history[m].campaignFlow=true;
    assert(j.SelectDifficulty(difficulty));save->beforeBattle=j;
    assert(j.Prepare(1,0));return save;
}

static void RoundTrip(const JourneySaveData& original)
{
    std::vector<std::uint8_t> bytes;auto restored=std::make_unique<JourneySaveData>();
    assert(EncodeJourneySave(original,bytes) && DecodeJourneySave(bytes,*restored));
    const auto& before=original.journey.battleReview.grand;const auto& after=restored->journey.battleReview.grand;
    assert(before.approach==after.approach && before.relievedOn==after.relievedOn && before.reliefOfficer==after.reliefOfficer && before.forcedSteps==after.forcedSteps);
    assert(original.journey.Battlefield()==restored->journey.Battlefield());
}

static std::vector<std::uint8_t> Version23(JourneySaveData& session)
{
    std::vector<std::uint8_t> bytes;unsigned magic=0x53594e47,version=23,checksum=0;
    JourneyArchive archive{false,bytes};archive.version=version;archive(magic,version,checksum);archive(session);
    checksum=2166136261u;
    for(std::size_t i=12;i<bytes.size();++i) { checksum^=bytes[i];checksum*=16777619u; }
    for(int i=0;i<4;++i) bytes[8+i]=std::uint8_t(checksum>>(i*8));
    return bytes;
}

static void PlayCampaign()
{
    auto session=std::make_unique<JourneySaveData>();auto& j=session->journey;
    j.ResetCampaign();j.Meet();j.grandCampaign=j.grandStrategyCampaign=true;j.SelectDifficulty(JourneyDifficulty::Story);
    int completed=0;
    for(int transition=0;transition<30;++transition)
    {
        if(j.CanChooseCoalitionPlan()) assert(j.ChooseCoalitionPlan(CoalitionPlan::Advance));
        else if(j.CanChooseLuoyangPlan(LuoyangPlan::SouthGate)) assert(j.ChooseLuoyangPlan(LuoyangPlan::SouthGate));
        else if(j.stage==JourneyStage::Preparation)
        {
            if(j.world.cities[0].troops<4500) j.Reinforce();
            session->beforeBattle=j;assert(j.Prepare(1,j.enemyPolicy));
            if(j.CanChooseGrandApproach()) assert(j.ChooseGrandApproach(GrandApproach::RelieveLeft));
            for(int cycle=0;cycle<36 && j.stage==JourneyStage::Battle;++cycle)
            {
                auto plan=PlanJourneyAutoBattle(j);
                if(!plan.tactics.empty()) j.Execute(plan.tactics);
                if(j.stage==JourneyStage::Battle) assert(plan.movement.Resolve(j));
            }
            std::cerr<<"strategic campaign mission="<<j.mission<<" won="<<j.won<<"\n";
            assert(j.won);++completed;RoundTrip(*session);
        }
        else if(j.NeedsStoryReply()) assert(j.ChooseStoryReply(JourneyPledge::Coordinate));
        else if(j.CanContinue()) assert(j.Continue());
        else break;
    }
    assert(completed==7 && j.history[3].review.grand.relievedOn>0);
}

int main()
{
    for(auto Difficulty:{JourneyDifficulty::Story,JourneyDifficulty::Standard})
    {
        auto Planning=std::make_unique<JourneySaveData>();auto& J=Planning->journey;
        J.ResetCampaign();J.Meet();J.mission=YellowTurbanMission;J.grandCampaign=J.grandStrategyCampaign=true;assert(J.SelectDifficulty(Difficulty));
        for(int M=0;M<YellowTurbanMission;++M) J.history[M].completed=J.history[M].won=J.history[M].campaignFlow=true;
        const auto Camp=J.world.cities[0];const auto Example=PlanExample(J);
        assert(J.HasGrandStrategy() && J.PlanningTerrain(Example.assignments[1].tile)==Terrain::Forest);
        assert(J.ApplyPlanExample() && J.world.cities[0].food==Camp.food && J.world.cities[0].gold==Camp.gold);
        assert(J.assignments[1].duty==JourneyDuty::Ambush && J.assignments[1].tile==Example.assignments[1].tile);
        Planning->beforeBattle=J;
        assert(J.Prepare(1,0) && J.world.armies[1].tile==Example.assignments[1].tile);
        assert(J.world.tiles[J.world.armies[1].tile].terrain==Terrain::Forest);
        RoundTrip(*Planning);
    }
    auto left=Deploy();auto center=std::make_unique<JourneySaveData>(*left);
    auto& j=left->journey;
    assert(j.HasGrandStrategy() && j.Battlefield()==JourneyMap::GuangzongFronts);
    for(const auto row:BattlefieldDefinition(j.Battlefield()).rows) assert(row.size()==14);
    assert(j.world.tiles[Campaign::At(8,6)].terrain==Terrain::River && j.world.tiles[Campaign::At(8,8)].terrain==Terrain::Bridge);
    assert(BuildJourneyEpisode(*left->beforeBattle).lines[1].text.find(U"北")!=std::u32string::npos);
    const auto troops=j.world.armies[0].troops;const auto morale=j.world.armies[0].morale;
    assert(j.ChooseGrandApproach(GrandApproach::RelieveLeft) && center->journey.ChooseGrandApproach(GrandApproach::CentralAssault));
    assert(!j.ChooseGrandApproach(GrandApproach::CentralAssault));
    assert(j.world.armies[0].troops==troops && j.world.armies[0].morale==morale && !j.chain.spent[0] && !j.chain.moved[0]);
    assert(j.GrandMainTarget()==GrandNorthPass && center->journey.GrandMainTarget()==Campaign::At(7,8));
    RoundTrip(*left);RoundTrip(*center);
    auto firstLeft=std::make_unique<JourneySaveData>(*left),firstCenter=std::make_unique<JourneySaveData>(*center);
    assert(firstLeft->journey.EndTurn() && firstCenter->journey.EndTurn());
    const int forced=firstCenter->journey.battleReview.grand.forcedSteps;
    assert(forced>0 && firstLeft->journey.world.armies[5].food-firstCenter->journey.world.armies[5].food==forced*100);
    auto malformed=std::make_unique<JourneySaveData>(*left);malformed->journey.battleReview.grand.forcedSteps=1;
    assert(!ValidJourneySave(*malformed));
    bool diverged=false;
    for(int t=0;t<6;++t)
    {
        assert(j.EndTurn());assert(center->journey.EndTurn());
        diverged=diverged || j.world.armies[5].tile!=center->journey.world.armies[5].tile;
    }
    assert(diverged);
    auto destroyed=Deploy();
    destroyed->journey.world.armies[6].troops=destroyed->journey.world.armies[8].troops=0;
    destroyed->journey.world.armies[1].tile=destroyed->journey.world.armies[1].target=GrandNorthPass;
    assert(destroyed->journey.EndTurn() && !destroyed->journey.battleReview.grand.relievedOn);
    RoundTrip(*destroyed);
    auto fled=Deploy();
    fled->journey.world.armies[6].tile=fled->journey.world.armies[6].target=Campaign::At(2,3);
    fled->journey.world.armies[8].troops=0;
    fled->journey.world.armies[1].tile=fled->journey.world.armies[1].target=GrandNorthPass;
    fled->journey.CheckGrandRelief();assert(!fled->journey.battleReview.grand.relievedOn);
    auto routed=Deploy();auto& rout=routed->journey;
    rout.world.armies[3].troops=rout.world.armies[4].troops=rout.world.armies[6].troops=0;
    rout.world.armies[0].tile=rout.world.armies[0].target=Campaign::At(11,8);
    rout.world.armies[5].tile=rout.world.armies[5].target=GrandRally;
    rout.world.armies[10].tile=rout.world.armies[10].target=Campaign::At(14,4);rout.world.armies[10].morale=0;
    rout.RefreshIntelligence();assert(rout.Move(0,rout.goal) && rout.won && rout.world.armies[10].troops>0);
    assert(BuildJourneyEpisode(rout).lines[2].text.find(U"戻らぬ")!=std::u32string::npos);RoundTrip(*routed);
    int failures=0;
    for(auto choice:{GrandApproach::RelieveLeft,GrandApproach::CentralAssault})
        for(auto difficulty:{JourneyDifficulty::Story,JourneyDifficulty::Standard})
        {
            auto session=Deploy(true,difficulty);auto& battle=session->journey;
            assert(battle.ChooseGrandApproach(choice));
            for(int cycle=0;cycle<32 && battle.stage==JourneyStage::Battle;++cycle)
            {
                auto plan=PlanJourneyAutoBattle(battle);
                if(!plan.tactics.empty()) battle.Execute(plan.tactics);
                if(battle.stage==JourneyStage::Battle) assert(plan.movement.Resolve(battle));
            }
            const auto& g=battle.battleReview.grand;
            std::cerr<<"grand strategy="<<int(choice)<<" difficulty="<<int(difficulty)<<" won="<<battle.won<<" turn="<<battle.turn<<" left="<<battle.world.armies[6].troops<<" relieved="<<g.relievedOn<<" by="<<g.reliefOfficer<<" boss="<<battle.world.armies[10].troops<<"\n";
            if(!battle.won) { ++failures;continue; }
            if(choice==GrandApproach::RelieveLeft) assert(g.relievedOn && g.leftReturned>0);
            RoundTrip(*session);
            const auto episode=BuildJourneyEpisode(battle);
            assert(episode.lines.size()>=4);
            const auto beat=CampaignBeat(battle);
            assert(beat.change.find(U"本陣")!=std::u32string::npos);
            assert((beat.next.find(U"士気＋10")!=std::u32string::npos)==
                (g.relievedOn>0 && g.leftReturned>0 && g.reliefOfficer>=0));
            if(g.relievedOn && g.leftReturned>0)
            {
                const int officer=g.reliefOfficer;
                assert(battle.Continue() && battle.ChooseCoalitionPlan(CoalitionPlan::Advance));
                RoundTrip(*session);
                const int supported=battle.PreparationMorale(1,officer);
                assert(battle.PreparationPositions(1)[officer]==Campaign::At(10,5));
                auto baseline=std::make_unique<StrategistJourney>(battle);
                auto& past=baseline->history[YellowTurbanMission].review.grand;
                past.relievedOn=0;past.reliefOfficer=-1;
                assert(supported==std::min(100,baseline->PreparationMorale(1,officer)+10));
            }
        }
    assert(failures==0);
    auto previous=Deploy(false);
    auto bytes=Version23(*previous);
    auto loaded=std::make_unique<JourneySaveData>();
    assert(DecodeJourneySave(bytes,*loaded) && !loaded->journey.HasGrandStrategy() &&
        loaded->journey.Battlefield()==JourneyMap::Guangzong && loaded->journey.world.armies.size()==11);
    loaded->journey.grandStrategyCampaign=true;
    assert(!loaded->journey.HasGrandStrategy() && loaded->journey.Battlefield()==JourneyMap::Guangzong);
    auto oldShort=std::make_unique<JourneySaveData>();auto& old=oldShort->journey;
    old.ResetCampaign();old.Meet();oldShort->beforeBattle=old;assert(old.Prepare(1,0));
    old.world.bonds.push_back({0,5,40}); // Version 23 could retain a removed NPC's bond.
    bytes=Version23(*oldShort);
    assert(DecodeJourneySave(bytes,*loaded));
    for(const auto& bond:loaded->journey.world.bonds) assert(bond.a<5 && bond.b<5);
    PlayCampaign();
    std::cout<<"Grand strategy: PASS\n";
}
