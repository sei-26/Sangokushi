#include "JourneyAutoBattle.hpp"
#include "JourneySave.hpp"
#include "JourneyArchive.hpp"
#include "JourneyBattlefield.hpp"
#include "JourneyEpisode.hpp"
#include <cassert>
#include <iostream>
#include <memory>
using namespace frontline;

static void Save(const JourneySaveData& s)
{
    std::vector<std::uint8_t> bytes;auto loaded=std::make_unique<JourneySaveData>();
    if(!EncodeJourneySave(s,bytes)) { std::cerr<<"save failed mission="<<s.journey.mission<<" stage="<<int(s.journey.stage)<<" turn="<<s.journey.turn<<"\n";assert(false); }
    assert(DecodeJourneySave(bytes,*loaded));
    const auto& a=s.journey.battleReview.grand.coalition;const auto& b=loaded->journey.battleReview.grand.coalition;
    assert(a.rescuedOn==b.rescuedOn && a.returnedOn==b.returnedOn && a.caoReturned==b.caoReturned);
}
static void Play(JourneySaveData& s)
{
    auto& j=s.journey;
    for(int cycle=0;cycle<45 && j.stage==JourneyStage::Battle;++cycle)
    {
        auto p=PlanJourneyAutoBattle(j);
        if(!p.tactics.empty()) j.Execute(p.tactics);
        if(j.stage==JourneyStage::Battle) assert(p.movement.Resolve(j));
        Save(s);
    }
}
static std::unique_ptr<JourneySaveData> Start(CoalitionPlan plan,JourneyDifficulty difficulty,bool modern=true)
{
    auto s=std::make_unique<JourneySaveData>();auto& j=s->journey;
    j.ResetCampaign();j.Meet();j.mission=CoalitionMission;j.grandCampaign=true;j.coalitionBattleCampaign=modern;j.coalitionPlan=plan;
    for(int m=0;m<CoalitionMission;++m) j.history[m].completed=j.history[m].won=j.history[m].campaignFlow=true;
    j.SelectDifficulty(difficulty);s->beforeBattle=j;assert(j.Prepare(1,1));return s;
}
int main()
{
    {
        auto s=Start(CoalitionPlan::Protect,JourneyDifficulty::Story);auto& j=s->journey;
        assert(!CoalitionReturnReached(j.world,-1) && !CoalitionReturnReached(j.world,99));
        j.world.armies[5].tile=Campaign::At(6,9);assert(CoalitionReturnReached(j.world,5));
        j.world.armies[5].tile=CoalitionBridge;assert(!CoalitionReturnReached(j.world,5));
        j.world.armies[5].tile=Campaign::At(6,4);assert(!CoalitionReturnReached(j.world,5));
        j.world.armies[5].tile=Campaign::At(6,9);j.world.armies[6].tile=Campaign::At(5,10);
        j.battleReview.grand.coalition.rescuedOn=j.turn;
        assert(CoalitionReturnReached(j.world,5) && CoalitionReturnReached(j.world,6));
        j.CheckCoalitionBattle();assert(!j.won && j.stage==JourneyStage::Battle); // Heroes must still secure the assembly site.
        j.world.armies[0].tile=CoalitionRetreat;j.CheckCoalitionBattle();
        assert(j.won && j.world.armies[10].troops>0);
        j.world.armies[6].troops=0;assert(!CoalitionReturnReached(j.world,6));
    }
    for(auto plan:{CoalitionPlan::Advance,CoalitionPlan::Protect})
        for(auto difficulty:{JourneyDifficulty::Story,JourneyDifficulty::Standard})
        {
            auto s=Start(plan,difficulty);auto& j=s->journey;
            assert(j.HasCoalitionBattle() && j.HasGrandBattle() && j.world.armies.size()==11 && j.Battlefield()==JourneyMap::CoalitionFronts);
            assert(j.world.armies[5].tile%Width>7 && j.world.armies[6].tile%Width>7);
            assert(BuildJourneyEpisode(*s->beforeBattle).lines.size()>=6);
            auto planning=std::make_unique<StrategistJourney>(*s->beforeBattle);
            assert(planning->ApplyPlanExample());
            for(int a=0;a<3;++a) assert(planning->AssignmentProblem(a,planning->assignments[a].tile,planning->assignments[a].duty).empty());
            Save(*s);Play(*s);
            const auto& c=j.battleReview.grand.coalition;
            std::cerr<<"coalition plan="<<int(plan)<<" difficulty="<<int(difficulty)<<" won="<<j.won<<" turn="<<j.turn<<" rescue="<<c.rescuedOn<<" return="<<c.returnedOn<<" Cao="<<j.world.armies[5].troops<<" relief="<<j.world.armies[6].troops<<" HQ="<<j.world.armies[10].troops<<"\n";
            assert(j.won && BuildJourneyEpisode(j).lines.size()>=4);
            if(plan==CoalitionPlan::Protect) assert(c.rescuedOn && c.returnedOn && j.HasCoalitionGuide() && j.world.armies[10].troops>0);
            Save(*s);assert(j.Continue() && j.Continue() && j.mission==YangrenMission);Save(*s);
        }
    for(auto route:{CoalitionPlan::Advance,CoalitionPlan::Protect})
    {
        auto s=std::make_unique<JourneySaveData>();auto& j=s->journey;
        j.ResetCampaign();j.Meet();j.grandCampaign=j.grandStrategyCampaign=j.yangrenStrategyCampaign=j.coalitionBattleCampaign=j.shortPrelude=true;
        j.SelectDifficulty(JourneyDifficulty::Story);int battles=0;
        for(int transition=0;transition<28;++transition)
        {
            if(j.CanChooseCoalitionPlan()) assert(j.ChooseCoalitionPlan(route));
            else if(j.CanChooseLuoyangPlan(LuoyangPlan::WesternDepot)) assert(j.ChooseLuoyangPlan(LuoyangPlan::WesternDepot));
            else if(j.CanChooseLuoyangPlan(LuoyangPlan::SouthGate)) assert(j.ChooseLuoyangPlan(LuoyangPlan::SouthGate));
            else if(j.stage==JourneyStage::Preparation)
            {
                if(j.world.cities[0].troops<4500) j.Reinforce();
                s->beforeBattle=j;assert(j.Prepare(1,j.enemyPolicy));
                for(int a=0;a<int(j.world.armies.size());++a)
                    for(int b=a+1;b<int(j.world.armies.size());++b)
                        assert(j.world.armies[a].troops<=0 || j.world.armies[b].troops<=0 || j.world.armies[a].tile!=j.world.armies[b].tile);
                if(j.CanChooseGrandApproach()) assert(j.ChooseGrandApproach(GrandApproach::RelieveLeft));
                if(j.CanChooseYangrenApproach()) assert(j.ChooseYangrenApproach(YangrenApproach::Forest));
                Play(*s);std::cerr<<"campaign route="<<int(route)<<" mission="<<j.mission<<" won="<<j.won<<" turn="<<j.turn<<" resolution="<<int(j.battleReview.resolution)<<"\n";
                if(!j.won) {
                    std::cerr<<"deployed="<<j.deployed<<" opened="<<j.battleReview.grand.openedOn<<" rallied="<<j.battleReview.grand.ralliedOn<<" counter="<<j.battleReview.grand.yangren.counterOn<<" repelled="<<j.battleReview.grand.yangren.repelledOn<<"\n";
                    for(const auto& a:j.world.armies) std::cerr<<"("<<a.tile%Width<<","<<a.tile/Width<<") troops="<<a.troops<<" morale="<<a.morale<<"\n";
                }
                assert(j.won);++battles;
            }
            else if(j.CanContinue()) assert(j.Continue());else break;
        }
        assert(battles==5 && j.history[LuoyangMission].won && j.history[CoalitionMission].review.grand.coalition.active);
    }
    auto lost=Start(CoalitionPlan::Protect,JourneyDifficulty::Story);lost->journey.world.armies[6].troops=0;
    lost->journey.EndTurn();assert(!lost->journey.won && lost->journey.stage==JourneyStage::Debrief && !lost->journey.HasCoalitionGuide());Save(*lost);
    auto premature=Start(CoalitionPlan::Protect,JourneyDifficulty::Story);
    premature->journey.world.armies[0].tile=CoalitionRetreat;premature->journey.EndTurn();
    assert(!premature->journey.won && !premature->journey.battleReview.grand.coalition.rescuedOn);
    auto old=Start(CoalitionPlan::Protect,JourneyDifficulty::Story,false);
    std::vector<std::uint8_t> bytes;unsigned magic=0x53594e47,version=25,checksum=0;
    JourneyArchive ar{false,bytes};ar.version=version;ar(magic,version,checksum);ar(*old);
    checksum=2166136261u;for(std::size_t i=12;i<bytes.size();++i) { checksum^=bytes[i];checksum*=16777619u; }
    for(int i=0;i<4;++i) bytes[8+i]=std::uint8_t(checksum>>(8*i));
    auto loaded=std::make_unique<JourneySaveData>();assert(DecodeJourneySave(bytes,*loaded));
    assert(!loaded->journey.HasCoalitionBattle() && loaded->journey.world.armies.size()==5 && loaded->journey.Definition().kind==JourneyObjective::HoldPositions);
    loaded->journey.coalitionBattleCampaign=true;assert(!loaded->journey.HasCoalitionBattle());Save(*loaded);
    std::cout<<"Coalition grand battle: PASS\n";
}
