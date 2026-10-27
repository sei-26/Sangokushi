#include "../NewGame/JourneySave.hpp"
#include <cassert>
#include <iostream>
#include <memory>
using namespace frontline;
static std::vector<std::uint8_t> Bytes(const JourneySaveData& s)
{ std::vector<std::uint8_t> out;assert(EncodeJourneySave(s,out));return out; }
static std::unique_ptr<JourneySaveData> Round(const JourneySaveData& s)
{
    auto out=std::make_unique<JourneySaveData>();assert(DecodeJourneySave(Bytes(s),*out));
    auto expected=std::make_unique<JourneySaveData>(s);expected->realtime.paused=true;
    assert(Bytes(*out)==Bytes(*expected));return out;
}
int main()
{
    auto s=std::make_unique<JourneySaveData>();s->journey.Reset();Round(*s);
    s->journey.Meet();s->journey.SelectPromise(JourneyPromise::EscortRefugees);
    s->journey.SelectConvoyRoute(JourneyConvoyRoute::SouthernWood);s->journey.Reinforce();
    s->dialogue=3;s->preparation=1;s->interlocutor=2;Round(*s);
    s->beforeBattle=s->journey;assert(s->journey.Prepare(1,1));s->realtimeMode=true;
    assert(s->realtime.Queue(s->journey,0,JourneyDirective::March,Campaign::At(8,12)));
    assert(s->realtime.AppendWaypoint(s->journey,0,Campaign::At(9,12)));
    assert(s->realtime.SetArrival(s->journey,0,JourneyArrival::Hold));
    s->orders={{0,3,ChainTactic::Feint,-1},{2,3,ChainTactic::Breakthrough,-1}};
    assert(s->signal.Arm(s->journey,s->orders));
    auto before=JourneyBattleWatch::Capture(s->journey,s->realtime),after=before;
    after.units[0].troops=400;after.turn++;
    assert(!s->watch.Record(before,after).empty());
    s->realtime.paused=false;s->realtime.Advance(5.25);auto loaded=Round(*s);
    assert(loaded->signal.Armed() && loaded->watch.History().size()==1 && loaded->realtime.elapsed==5.25);
    assert(loaded->journey.intelligence[0].visible==s->journey.intelligence[0].visible);
    // Deterministic continuation: commands, AI, observations and finance all match.
    for(int i=0;i<3;++i)
    {
        assert(s->realtime.Resolve(s->journey)==loaded->realtime.Resolve(loaded->journey));
        s->realtime.paused=true;assert(Bytes(*s)==Bytes(*loaded));loaded=Round(*loaded);
    }
    assert(s->journey.Withdraw());auto debrief=Round(*s);
    const auto& camp=s->beforeBattle->world.cities[0];
    assert(debrief->beforeBattle->world.cities[0].gold==camp.gold);
    debrief->journey=*debrief->beforeBattle;assert(debrief->journey.Prepare(1,1));
    assert(debrief->journey.world.cities[0].gold==camp.gold-200);
    assert(debrief->journey.world.cities[0].troops==camp.troops-debrief->journey.deployed-600);
    // Corruption, unknown schema, truncation and malformed domain data are transactional.
    auto good=Bytes(*loaded),bad=good;const auto original=Bytes(*debrief);
    for(std::size_t size:{std::size_t(0),std::size_t(10),good.size()/2,good.size()-1})
    { bad.assign(good.begin(),good.begin()+size);assert(!DecodeJourneySave(bad,*debrief));assert(Bytes(*debrief)==original); }
    bad=good;bad.back()^=1;assert(!DecodeJourneySave(bad,*debrief));
    bad=good;bad[4]=99;assert(!DecodeJourneySave(bad,*debrief));
    bad=good;bad.push_back(0);assert(!DecodeJourneySave(bad,*debrief));
    auto invalid=std::make_unique<JourneySaveData>(*loaded);invalid->journey.world.armies[0].general=1000;
    assert(!EncodeJourneySave(*invalid,bad));invalid->journey.world.armies[0].general=0;
    invalid->journey.chain.spent.clear();assert(!EncodeJourneySave(*invalid,bad));
    // All three mission conclusions and the final chapter state survive a round trip.
    for(int mission=0;mission<3;++mission)
    {
        auto result=std::make_unique<JourneySaveData>();result->journey.Reset();result->journey.Meet();result->journey.mission=mission;
        result->beforeBattle=result->journey;assert(result->journey.Prepare(0,1));
        // Isolated persistence fixture; objective victories are tested separately.
        for(int enemy:{3,4}) result->journey.world.armies[enemy].troops=0;
        if(mission==2) result->journey.world.armies[0].tile=result->journey.goal;
        result->journey.RefreshIntelligence();
        while(result->journey.stage==JourneyStage::Battle) result->journey.EndTurn();
        assert(result->journey.won);Round(*result);assert(result->journey.Continue());Round(*result);
    }
    std::cout<<"Journey save PASS: encounter/council/battle/debrief/complete, exact continuation, retry finance, hidden observations, march/signal/watch, corrupt rejection\n";
}
