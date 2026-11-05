#include "../NewGame/JourneySave.hpp"
#include "../NewGame/JourneyPlanExample.hpp"
#include <cassert>
#include <iostream>
#include <memory>
using namespace frontline;
static std::vector<std::uint8_t> Bytes(const StrategistJourney& j)
{ auto s=std::make_unique<JourneySaveData>();s->journey=j;std::vector<std::uint8_t> b;assert(EncodeJourneySave(*s,b));return b; }
static void Place(StrategistJourney& j,int a,int tile)
{ j.world.armies[a].tile=j.world.armies[a].target=tile; }
static std::unique_ptr<JourneySaveData> Attempt(int guard)
{
    auto s=std::make_unique<JourneySaveData>();auto& j=s->journey;j.ResetCampaign();j.Meet();j.mission=2;
    const int source=guard==0 ? 1:0,other=3-source-guard;
    assert(j.AssignDuty(guard,j.Definition().goal,JourneyDuty::Hold) && j.AssignDuty(other,j.Definition().secondGoal,JourneyDuty::Hold));
    s->beforeBattle=j;assert(j.Prepare(1,0));
    // Explicit contact boundary; the paid troop totals and all subsequent commands stay real.
    for(int y=1;y<StrategistJourney::Rows-1;++y) for(int x=1;x<StrategistJourney::Columns-1;++x) j.world.tiles[Campaign::At(x,y)].terrain=Terrain::Plain;
    Place(j,guard,j.goal);Place(j,source,Campaign::At(6,6));Place(j,other,j.Definition().secondGoal);Place(j,3,Campaign::At(6,5));
    j.world.armies[3].troops=3000;j.world.armies[3].morale=90;j.world.armies[4].troops=0;
    j.chain.Reset(j.world);j.RefreshIntelligence();for(int a=0;a<3;++a) assert(s->realtime.Queue(j,a,JourneyDirective::Hold));
    assert(j.Execute({{source,3,ChainTactic::Feint,-1},{guard,3,ChainTactic::Disrupt,-1}}).completed==2);
    assert(s->realtime.Resolve(j) && j.battleReview.spentGuardFailures==(1<<guard) && j.Withdraw());return s;
}
int main()
{
    for(int affected:{0,1,2})
    {
        auto s=Attempt(affected);auto& j=s->journey;const auto expected=Bytes(*s->beforeBattle);
        const auto lesson=ReadDefenseRetryLesson(j,*s->beforeBattle);assert(lesson && lesson->failedGuards==(1<<affected));
        assert(j.RetryAtCouncil(*s->beforeBattle) && j.retryDefense && !j.retryRelief && j.stage==JourneyStage::Preparation);
        auto neutral=std::make_unique<StrategistJourney>(j);neutral->retryDefense.reset();assert(Bytes(*neutral)==expected);
        const auto before=Bytes(j);const auto original=j.assignments;const auto camp=j.world.cities[0];const auto trust=j.officerTrust;
        const auto p=PlanExample(j);assert(p.remembersDefenseRetry && !p.remembersCooperation && p.adviser==affected);
        assert(p.assignments[affected].duty==JourneyDuty::None && original[affected].duty==JourneyDuty::Hold);
        auto candidate=std::make_unique<StrategistJourney>(j);candidate->assignments={};
        int holds=0;for(int a=0;a<3;++a)
        {
            assert(candidate->AssignDuty(a,p.assignments[a].tile,p.assignments[a].duty));
            if(p.assignments[a].duty==JourneyDuty::Hold) { assert(p.assignments[a].tile==(holds==0 ? j.Definition().goal:j.Definition().secondGoal));++holds; }
        }
        assert(holds==2 && !p.offer.empty());for(int n=0;n<5;++n) PlanExample(j);assert(Bytes(j)==before);
        auto hidden=std::make_unique<StrategistJourney>(j);hidden->enemyPolicy=2;assert(PlanExample(*hidden).name==p.name && PlanExample(*hidden).offer==p.offer);
        assert(j.ApplyPlanExample() && j.assignments[affected].duty==JourneyDuty::None && j.world.cities[0].food==camp.food && j.officerTrust==trust);
        auto adopted=std::make_unique<StrategistJourney>(j);const auto bill=j.PreparationCosts(1);assert(j.Prepare(1,0) && !j.retryDefense);
        assert(j.world.cities[0].food==camp.food-bill.TotalFood() && j.world.cities[0].gold==camp.gold-bill.gold);
        s->realtime.Reset();s->realtime.ApplyAssignments(j);assert(s->realtime.orders[affected].kind==JourneyDirective::None);
        for(int a=0;a<3;++a) if(a!=affected) assert(s->realtime.orders[a].kind==JourneyDirective::March || s->realtime.orders[a].kind==JourneyDirective::Hold);
        assert(j.heldTurns==0 && !j.chain.spent[affected] && !j.assignments[affected].completed);
        const int start=j.world.armies[affected].tile;assert(s->realtime.Resolve(j) && j.world.armies[affected].tile==start);
        assert(j.Withdraw() && j.battleReview.spentGuardFailures==0 && j.RetryAtCouncil(*adopted) && !j.retryDefense);
        auto priority=Attempt(affected);assert(priority->journey.RetryAtCouncil(*priority->beforeBattle));
        JourneyReliefRecord r;r.outgoing=1;r.incoming=0;r.source=priority->journey.Definition().goal;r.destination=Campaign::At(6,6);
        r.issuedTurn=r.departedTurn=1;r.arrivedTurn=r.checkedTurn=2;r.outcome=JourneyReliefOutcome::Lost;priority->journey.retryRelief=r;
        const auto corrective=PlanExample(priority->journey);assert(!corrective.warning.empty() && !corrective.remembersDefenseRetry);
    }
    auto old=Attempt(1);old->journey.history[2].review.spentGuardFailures=old->journey.battleReview.spentGuardFailures=0;
    assert(old->journey.RetryAtCouncil(*old->beforeBattle) && !old->journey.retryDefense);
    auto stale=Attempt(2);--stale->journey.history[2].review.endedAt;assert(!ReadDefenseRetryLesson(stale->journey,*stale->beforeBattle));
    auto win=Attempt(1);win->journey.won=true;assert(!win->journey.RetryAtCouncil(*win->beforeBattle));
    auto reset=Attempt(0);assert(reset->journey.RetryAtCouncil(*reset->beforeBattle));reset->journey.Reset();assert(!reset->journey.retryDefense);
    std::cout<<"Retry defense PASS: actual spent-guard failure, exact unpaid rollback plus separate lesson, freed affected officer, public editable plan, normal deployment and orders, relief priority and no unknown/stale retry credit\n";
}
