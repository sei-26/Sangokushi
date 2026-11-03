#include "JourneyContributionPlayer.hpp"
#include "../NewGame/JourneyPlanExample.hpp"
#include "../NewGame/JourneyCouncilStage.hpp"
#include <cassert>
#include <iostream>
using namespace frontline;
static std::vector<std::uint8_t> Bytes(const JourneySaveData& s)
{ std::vector<std::uint8_t> b;assert(EncodeJourneySave(s,b));return b; }
int main()
{
    for(int variant:{0,1,3}) for(bool north:{false,true}) for(int preparation:{0,1})
    {
        auto s=contributionqa::Play(variant);assert(s);auto& j=s->journey;
        if(north) assert(j.ChooseStoryReply(JourneyPledge::Preserve));
        assert(j.Continue());s->beforeBattle=j;s->realtimeMode=true;
        j.AssignDuty(0,j.Definition().positions[0],JourneyDuty::Hold);const auto before=Bytes(*s);
        const auto p=PlanExample(j);assert(p.remembersCooperation && !p.offer.empty() && p.adviser==(variant==3 ? 2:1));
        const auto exchange=BuildCouncilExchange(j,-1,0);assert(exchange.beats.size()==4 && exchange.beats[0].dialogue.speaker==p.adviser && exchange.beats[0].dialogue.text==p.offer);
        for(int n=0;n<12;++n) { PlanExample(j);BuildCouncilExchange(j,-1,0); }assert(Bytes(*s)==before);
        const int duty=variant==3 ? 2:1;const int destination=variant==3 ? Campaign::At(10,north ? 8:10):j.Definition().goal;
        assert(p.assignments[duty].duty==JourneyDuty::Hold && p.assignments[duty].tile==destination && p.assignments[0].duty==JourneyDuty::None);
        auto hidden=std::make_unique<StrategistJourney>(j);hidden->enemyPolicy=2;
        assert(PlanExample(*hidden).offer==p.offer && PlanExample(*hidden).assignments[duty].tile==destination);
        const auto camp=j.world.cities[0];const auto trust=j.officerTrust;const auto previous=j.history[j.mission-1].review.coordinatedOfficers;
        assert(j.ApplyPlanExample() && j.world.cities[0].gold==camp.gold && j.world.cities[0].food==camp.food && j.officerTrust==trust);
        assert(j.history[j.mission-1].review.coordinatedOfficers==previous && j.assignments[duty].tile==destination);
        auto loaded=std::make_unique<JourneySaveData>();assert(DecodeJourneySave(Bytes(*s),*loaded));
        assert(PlanExample(loaded->journey).offer==p.offer && loaded->journey.assignments[duty].tile==destination);
        s->beforeBattle=j;const auto cost=j.PreparationCosts(preparation);assert(j.Prepare(preparation,0));
        assert(j.world.cities[0].gold==camp.gold-cost.gold && j.world.cities[0].food==camp.food-cost.TotalFood());
        assert(j.world.armies[duty].tile!=destination && !j.assignments[duty].completed);
        s->realtime.ApplyAssignments(j);assert(s->realtime.orders[0].kind==JourneyDirective::None && s->realtime.orders[duty].kind==JourneyDirective::March && s->realtime.orders[duty].destination==destination);
        const bool concealed=j.chain.concealed[duty];const int start=j.world.armies[duty].tile,turn=j.turn;assert(s->realtime.Resolve(j));
        assert(j.turn==turn+1 && j.world.MapDistance(start,j.world.armies[duty].tile)==1); // The next cycle has already renewed its action flags.
        if(concealed) assert(!j.chain.concealed[duty]);
        assert(j.Withdraw() && j.RetryAtCouncil(*s->beforeBattle));
        assert(j.assignments[duty].tile==destination && j.world.cities[0].food==camp.food && j.world.cities[0].gold==camp.gold);
    }
    auto ordinary=contributionqa::Play(2);assert(ordinary && ordinary->journey.Continue());
    assert(!PlanExample(ordinary->journey).remembersCooperation);
    auto fixture=contributionqa::Play(0);assert(fixture && fixture->journey.Continue());auto& v=fixture->journey;auto& h=v.history[1];
    h.story.costly=true;assert(!PlanExample(v).remembersCooperation && PlanExample(v).adviser==0);h.story.costly=false;
    h.story.pledge=JourneyPledge::Preserve;h.story.pledgeKept=false;assert(!PlanExample(v).remembersCooperation);h.story.pledge=JourneyPledge::None;
    h.review.coordinatedOfficers=0;assert(!PlanExample(v).remembersCooperation);h.review.coordinatedOfficers=3;
    h.review.endedAt=0;assert(!PlanExample(v).remembersCooperation);h.review.endedAt=5;
    JourneyReliefRecord r;r.outgoing=2;r.incoming=0;r.source=Campaign::At(8,9);r.destination=Campaign::At(9,9);r.issuedTurn=r.departedTurn=1;r.arrivedTurn=r.checkedTurn=2;r.gapCycles=1;r.outcome=JourneyReliefOutcome::Recovered;
    v.retryRelief=r;assert(!PlanExample(v).remembersCooperation && !PlanExample(v).warning.empty());
    std::cout<<"Cooperation plan PASS: actual prior chains, chosen council offer, paid normal deployment, real march budgets, read/save invariance, retry, unknown and corrective-advice priority\n";
}
