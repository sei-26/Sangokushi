#include "JourneyTestFlow.hpp"
#include "../NewGame/JourneyArchive.hpp"
#include <cassert>
#include <iostream>
#include <memory>
using namespace frontline;
static std::unique_ptr<StrategistJourney> Council(int mission)
{ auto j=std::make_unique<StrategistJourney>();j->Reset();j->Meet();j->mission=mission;return j; }
static void WinDefense(StrategistJourney& j)
{
    for(int i:{3,4}) j.world.armies[i].troops=0;
    j.world.armies[0].tile=j.goal;j.world.armies[2].tile=j.Definition().secondGoal;
    j.RefreshIntelligence();while(j.stage==JourneyStage::Battle) j.EndTurn();assert(j.won);
}
int main()
{
    for(int mission=0;mission<3;++mission) for(int policy=0;policy<3;++policy)
    {
        auto j=Council(mission);j->enemyPolicy=policy;
        const auto camp=j->world.cities[0];const auto trust=j->officerTrust;
        assert(!j->AssignDuty(-1,0,JourneyDuty::Hold));
        assert(!j->AssignDuty(0,Campaign::At(0,0),JourneyDuty::Hold));
        assert(!j->AssignDuty(0,Campaign::At(14,3),JourneyDuty::Hold));
        assert(!j->AssignDuty(0,j->Definition().positions[1],JourneyDuty::Ambush));
        assert(j->AssignDuty(1,j->Definition().positions[1],JourneyDuty::Ambush));
        assert(!j->AssignDuty(0,j->Definition().positions[1],JourneyDuty::Hold));
        assert(j->world.cities[0].gold==camp.gold && j->world.cities[0].food==camp.food && j->officerTrust==trust);
        assert(!j->assignments[1].completed);
        assert(j->Prepare(1,policy));assert(!j->assignments[1].completed);
        assert(!j->AssignDuty(0,j->goal,JourneyDuty::Hold));
    }
    auto j=Council(2);const int north=j->Definition().goal,south=j->Definition().secondGoal;
    assert(!j->AssignDuty(0,Campaign::At(9,7),JourneyDuty::Hold)); // River, not a free waypoint.
    assert(j->AssignDuty(0,north,JourneyDuty::Hold));
    assert(j->AssignDuty(1,j->Definition().positions[1],JourneyDuty::Ambush));
    assert(j->AssignDuty(2,south,JourneyDuty::Hold));
    auto save=std::make_unique<JourneySaveData>();save->journey=*j;save->beforeBattle=*j;
    assert(j->Prepare(1,1));
    for(int i:{3,4}) j->world.armies[i].troops=0; // Isolate order execution from combat outcomes.
    j->RefreshIntelligence();JourneyRealtime rt;rt.ApplyAssignments(*j);
    assert(rt.paused && rt.orders[0].kind==JourneyDirective::March && rt.orders[1].arrival==JourneyArrival::Ambush);
    assert(rt.Resolve(*j));
    assert(j->world.armies[0].tile==north && !j->assignments[0].completed); // Moving grants no free guard.
    assert(j->assignments[1].completed && j->assignments[2].completed);
    assert(rt.Resolve(*j) && j->assignments[0].completed);
    save->journey=*j;save->realtime=rt;std::vector<std::uint8_t> bytes;
    assert(EncodeJourneySave(*save,bytes) && bytes[4]==7);
    auto loaded=std::make_unique<JourneySaveData>();assert(DecodeJourneySave(bytes,*loaded));
    assert(loaded->journey.assignments[0].completed && loaded->beforeBattle->assignments[0].tile==north);
    assert(!loaded->beforeBattle->assignments[0].completed);
    auto without=std::make_unique<StrategistJourney>(*j);without->assignments={};
    WinDefense(*j);WinDefense(*without);
    for(int i=0;i<3;++i) assert(j->officerTrust[i]==without->officerTrust[i]+2);
    assert(j->history[2].assignments[0].completed && JourneyConversation(*j,0).size()>JourneyConversation(*without,0).size());
    const auto trust=j->officerTrust;assert(!j->EndTurn() && j->officerTrust==trust);
    auto failed=Council(2);assert(failed->AssignDuty(2,south,JourneyDuty::Hold) && failed->Prepare(1,1));
    assert(failed->Hold(2));assert(failed->assignments[2].completed && failed->Withdraw());
    assert(failed->history[2].story.trustDelta[2]<=0); // Placement is not mission success.
    auto unseen=Council(0);assert(unseen->SelectPromise(JourneyPromise::EscortRefugees));
    assert(unseen->AssignDuty(1,Campaign::At(9,12),JourneyDuty::Ambush));
    assert(unseen->SelectPromise(JourneyPromise::OpenRoad) && unseen->assignments[1].duty==JourneyDuty::None);
    auto opening=Council(0);
    assert(opening->AssignDuty(1,opening->Definition().positions[1],JourneyDuty::Ambush));
    assert(opening->Prepare(0,1) && opening->assignments[1].completed); // Actual paid opening preparation counts.
    for(int e:{3,4}) opening->world.armies[e].troops=0;
    opening->RefreshIntelligence();assert(opening->EndTurn() && opening->won);
    assert(opening->ChooseStoryReply(JourneyPledge::Preserve) && opening->Continue());
    assert(opening->assignments[1].duty==JourneyDuty::None && opening->history[0].assignments[1].completed);
    // Encode the former v3 wire layout; its absent assignments must default to none.
    auto legacy=Council(0);save->journey=*legacy;save->beforeBattle.reset();save->realtime.Reset();bytes.clear();
    JourneyArchive writer{false,bytes};writer.version=3;unsigned magic=0x53594e47,version=3,checksum=0;writer(magic,version,checksum);writer(*save);
    checksum=2166136261u;for(std::size_t i=12;i<bytes.size();++i) { checksum^=bytes[i];checksum*=16777619u; }
    for(int i=0;i<4;++i) bytes[8+i]=static_cast<std::uint8_t>(checksum>>(i*8));
    assert(DecodeJourneySave(bytes,*loaded) && loaded->journey.assignments[0].duty==JourneyDuty::None);
    loaded->journey.assignments[0]={JourneyDuty::Hold,-1,false};assert(!EncodeJourneySave(*loaded,bytes));
    std::cout<<"Assignments PASS: public council choices, real march/arrival costs, no free guard, win-only trust, actual dialogue, retries, v3 migration and v4 persistence\n";
}
