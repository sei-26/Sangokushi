#include "../NewGame/JourneySave.hpp"
#include "../NewGame/JourneyDefense.hpp"
#include <cassert>
#include <iostream>
#include <memory>
using namespace frontline;
static void Place(StrategistJourney& j,int a,int tile)
{ j.world.armies[a].tile=j.world.armies[a].target=tile; }
static std::unique_ptr<JourneySaveData> Scene(int guard=1)
{
    auto s=std::make_unique<JourneySaveData>();auto& j=s->journey;j.ResetCampaign();j.Meet();j.mission=2;s->beforeBattle=j;s->realtimeMode=true;
    assert(j.Prepare(1,0)); // Authored contact boundary after ordinary paid deployment; no settled outcome is fabricated.
    for(int y=1;y<StrategistJourney::Rows-1;++y) for(int x=1;x<StrategistJourney::Columns-1;++x) j.world.tiles[Campaign::At(x,y)].terrain=Terrain::Plain;
    Place(j,guard,j.goal);Place(j,1-guard,Campaign::At(6,6));Place(j,2,j.Definition().secondGoal);Place(j,3,Campaign::At(6,5));
    j.world.armies[3].troops=3000;j.world.armies[3].morale=90;j.world.armies[4].troops=0;j.chain.Reset(j.world);j.RefreshIntelligence();
    for(int a=0;a<3;++a) assert(s->realtime.Queue(j,a,JourneyDirective::Hold));return s;
}
static std::vector<std::uint8_t> Bytes(const JourneySaveData& s)
{ std::vector<std::uint8_t> b;assert(EncodeJourneySave(s,b));return b; }
int main()
{
    for(int guard:{0,1})
    {
        auto s=Scene(guard);auto& j=s->journey;const int source=1-guard,guardBit=1<<guard;
        j.assignments[guard]={JourneyDuty::Hold,j.goal,true};
        for(int n=0;n<4;++n) { ReadJourneyDefense(j,j.goal);ReadJourneyDefenseCheck(j,j.goal);j.Preview({{source,3,ChainTactic::Feint,-1}}); }
        assert(j.battleReview.confirmedDefenders==0 && j.battleReview.spentGuardFailures==0);
        assert(j.Execute({{source,3,ChainTactic::Feint,-1},{guard,3,ChainTactic::Disrupt,-1}}).completed==2);
        assert(j.battleReview.coordinatedOfficers==3 && j.battleReview.confirmedDefenders==0 && j.battleReview.spentGuardFailures==0);
        assert(s->realtime.Resolve(j) && j.stage==JourneyStage::Battle);
        const auto failed=ReadJourneyDefenseCheck(j,j.goal),other=ReadJourneyDefenseCheck(j,j.Definition().secondGoal);
        assert(failed.reason==JourneyDefenseReason::GuardSpent && !failed.secured && other.secured);
        assert(j.battleReview.spentGuardFailures==guardBit && j.battleReview.confirmedDefenders==4 && j.heldTurns==0);
        const auto before=Bytes(*s);for(int n=0;n<4;++n) { ReadJourneyDefense(j,j.goal);ReadJourneyDefenseCheck(j,j.goal); }assert(Bytes(*s)==before);
        auto loaded=std::make_unique<JourneySaveData>();assert(DecodeJourneySave(before,*loaded));
        assert(loaded->journey.battleReview.spentGuardFailures==guardBit && loaded->journey.battleReview.confirmedDefenders==4 && loaded->journey.defenseChecks[0].goal<0);
        j.defenseChecks={};assert(j.battleReview.confirmedDefenders==4 && j.battleReview.spentGuardFailures==guardBit);
        for(int n=0;n<8 && j.stage==JourneyStage::Battle;++n) assert(s->realtime.Resolve(j));
        assert(j.stage==JourneyStage::Debrief && j.won && j.history[2].review.resolution==JourneyResolution::Held);
        const auto review=j.history[2].review;
        assert(review.spentGuardFailures==guardBit && review.confirmedDefenders==(guardBit|4) && review.coordinatedOfficers==3);
        assert(!(review.confirmedDefenders&(1<<source))); // The damaging participant away from either beacon receives no defense credit.
        assert(DecodeJourneySave(Bytes(*s),*loaded) && loaded->journey.history[2].review.confirmedDefenders==review.confirmedDefenders);
        j.stage=JourneyStage::Preparation;j.world.cities[0].troops=6000;j.world.cities[0].food=18000;j.world.cities[0].gold=900;
        assert(j.Prepare(1,0) && j.battleReview.confirmedDefenders==0 && j.battleReview.spentGuardFailures==0);
        assert(j.history[2].review.confirmedDefenders==review.confirmedDefenders);
    }
    auto weak=Scene();auto& w=weak->journey;w.world.armies[1].troops=499;w.chain.disordered[3]=true;assert(w.Hold(1) && weak->realtime.Resolve(w));
    const auto weakCheck=ReadJourneyDefenseCheck(w,w.goal);
    assert((weakCheck.reason==JourneyDefenseReason::GuardWeak || weakCheck.reason==JourneyDefenseReason::Depleted) && w.battleReview.confirmedDefenders==4 && w.battleReview.spentGuardFailures==0);
    auto absent=Scene();auto& a=absent->journey;Place(a,1,Campaign::At(5,6));a.assignments[1]={JourneyDuty::Hold,a.goal,true};a.RefreshIntelligence();
    assert(absent->realtime.Resolve(a) && a.battleReview.confirmedDefenders==4 && a.battleReview.spentGuardFailures==0);
    assert(ReadJourneyDefenseCheck(a,a.goal).reason==JourneyDefenseReason::Vacant);
    std::cout<<"Defense history PASS: actual per-officer local checks only, failed spent guard and independent wing, later recovery, real settled victory, persisted masks, no preview/assignment credit and weaker/vacant causes excluded\n";
}
