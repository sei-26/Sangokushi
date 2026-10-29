#include "../NewGame/JourneyPlanExample.hpp"
#include "../NewGame/JourneySave.hpp"
#include "../NewGame/JourneyBattlefield.hpp"
#include <cassert>
#include <iostream>
#include <memory>
using namespace frontline;

static std::unique_ptr<JourneySaveData> Council(int mission)
{
    auto s=std::make_unique<JourneySaveData>();s->journey.ResetCampaign();s->journey.Meet();s->journey.mission=mission;
    s->preparation=1;s->journey.SelectDifficulty(JourneyDifficulty::Story);return s;
}
static JourneyResult& History(StrategistJourney& j)
{
    // Isolated settled-history fixtures exercise advice selection, not a combat-playthrough claim.
    auto& h=j.history[j.mission-1];h={};h.completed=h.won=h.campaignFlow=true;h.survivors=4200;
    h.review.preparationTracked=true;h.review.endedAt=3;h.story.recorded=true;
    h.review.resolution=j.mission==1 ? JourneyResolution::EnemyBroken:JourneyResolution::Captured;
    return h;
}
static void Evidence(JourneyResult& h,int adviser)
{
    if(adviser==0) { h.casualties=1050;h.survivors=3150;h.story.loss=1050; }
    if(adviser==1) h.review.openingAmbushes=1;
    if(adviser==2) { h.review.resupplyTurn=2;h.review.postSupplySteps=h.review.tacticSteps=1; }
}
static std::vector<std::uint8_t> Bytes(const JourneySaveData& s)
{ std::vector<std::uint8_t> bytes;assert(EncodeJourneySave(s,bytes));return bytes; }
static bool SameAssignments(const std::array<JourneyAssignment,3>& a,const std::array<JourneyAssignment,3>& b)
{
    for(int i=0;i<3;++i) if(a[i].duty!=b[i].duty || a[i].tile!=b[i].tile || a[i].completed!=b[i].completed) return false;
    return true;
}
static void SamePlan(const JourneyPlanExample& a,const JourneyPlanExample& b)
{
    assert(a.name==b.name && a.purpose==b.purpose && a.reason==b.reason && a.adviser==b.adviser);
    assert(a.manualTask==b.manualTask && a.caution==b.caution && SameAssignments(a.assignments,b.assignments));
}
static void Hold(const JourneyAssignment& a,int tile) { assert(a.duty==JourneyDuty::Hold && a.tile==tile && !a.completed); }
static void CheckRoles(const StrategistJourney& j,const JourneyPlanExample& p,int adviser)
{
    assert(p.adviser==adviser && !p.reason.empty() && !p.manualTask.empty() && !p.caution.empty());
    if(j.mission==1 && adviser==1)
    {
        Hold(p.assignments[0],Campaign::At(10,8));
        assert(p.assignments[1].duty==JourneyDuty::Ambush && p.assignments[1].tile==Campaign::At(9,7));
    }
    else if(j.mission==1)
    {
        const bool north=j.CurrentPledge()==JourneyPledge::Preserve;
        Hold(p.assignments[0],Campaign::At(10,north ? 8:10));Hold(p.assignments[1],Campaign::At(10,north ? 10:8));
    }
    else
    {
        Hold(p.assignments[0],j.Definition().goal);
        if(adviser==2) assert(p.assignments[1].duty==JourneyDuty::None);
        else Hold(p.assignments[1],Campaign::At(7,7));
    }
    if(j.mission==1) assert(p.assignments[2].duty==JourneyDuty::None);
    else Hold(p.assignments[2],j.Definition().secondGoal);
    for(int i=0;i<3;++i)
    {
        assert(j.AssignmentProblem(i,p.assignments[i].tile,p.assignments[i].duty).empty());
        if(p.assignments[i].duty!=JourneyDuty::None)
        {
            const auto t=BattlefieldTerrain(j.mission,p.assignments[i].tile%Width,p.assignments[i].tile/Width);
            assert(t!=Terrain::River && t!=Terrain::Sea);
        }
    }
}
int main()
{
    for(int mission:{1,2}) for(int adviser:{0,1,2}) for(auto pledge:{JourneyPledge::Preserve,JourneyPledge::Coordinate})
    {
        auto s=Council(mission);auto& j=s->journey;auto& h=History(j);Evidence(h,adviser);h.story.reply=pledge;
        j.assignments[2]={JourneyDuty::Hold,j.Definition().positions[2],false};
        const auto before=Bytes(*s);const auto plan=PlanExample(j);CheckRoles(j,plan,adviser);
        assert(Bytes(*s)==before); // Previewing advice never issues orders or awards trust.
        for(int policy:{0,1,2})
        { auto other=std::make_unique<StrategistJourney>(j);other->enemyPolicy=policy;SamePlan(plan,PlanExample(*other)); }
        auto unpaid=std::make_unique<JourneySaveData>(*s);
        assert(j.ApplyPlanExample() && SameAssignments(j.assignments,plan.assignments));
        auto normalized=std::make_unique<JourneySaveData>(*s);
        normalized->journey.assignments=unpaid->journey.assignments;normalized->journey.message=unpaid->journey.message;
        assert(Bytes(*normalized)==before); // Only the reviewed duties and their notice change.
        auto restored=std::make_unique<JourneySaveData>();assert(DecodeJourneySave(Bytes(*s),*restored));
        assert(SameAssignments(restored->journey.assignments,plan.assignments));SamePlan(plan,PlanExample(restored->journey));
        assert(restored->journey.AssignDuty(1,restored->journey.Definition().positions[1],JourneyDuty::Hold)); // Adopted advice stays editable.
        s->beforeBattle=j;const auto camp=j.world.cities[0];const auto costs=j.PreparationCosts(1);
        assert(j.Prepare(1,j.enemyPolicy));assert(j.world.cities[0].food==camp.food-costs.TotalFood() && j.world.cities[0].gold==camp.gold-costs.gold);
        assert(!j.ApplyPlanExample());
        for(int i=0;i<3;++i) assert(!j.assignments[i].completed); // Adoption grants no completed guard for free.
        s->realtime.ApplyAssignments(j);
        for(int i=0;i<3;++i)
        {
            const auto& duty=plan.assignments[i];const auto& order=s->realtime.orders[i];
            if(duty.duty==JourneyDuty::None) assert(order.kind==JourneyDirective::None);
            else assert(order.kind==JourneyDirective::March && order.destination==duty.tile &&
                order.arrival==(duty.duty==JourneyDuty::Hold ? JourneyArrival::Hold:JourneyArrival::Ambush));
        }
        assert(DecodeJourneySave(Bytes(*s),*restored));
        assert(SameAssignments(restored->journey.assignments,plan.assignments));
    }
    // No new story or invented reserve choice is inferred from missing measurements.
    auto base=Council(1);auto& j=base->journey;auto& h=History(j);
    const auto plain=PlanExample(j);assert(plain.reason.empty() && plain.adviser==-1);
    h.review.openingAmbushes=1;h.review.preparationTracked=false;assert(PlanExample(j).reason.empty());
    h.review.preparationTracked=true;h.completed=false;assert(PlanExample(j).reason.empty());
    h.completed=true;h.won=false;assert(PlanExample(j).reason.empty());
    h.won=true;j.campaignFlow=false;assert(PlanExample(j).reason.empty());j.campaignFlow=true;
    j.stage=JourneyStage::Battle;assert(PlanExample(j).reason.empty());j.stage=JourneyStage::Preparation;
    auto opening=Council(0);assert(PlanExample(opening->journey).reason.empty());
    // Cost takes priority over an impressive tactic; otherwise actual ambush precedes supply evidence.
    h=History(j);Evidence(h,2);h.review.openingAmbushes=1;assert(PlanExample(j).adviser==1);
    h.casualties=1050;h.survivors=3150;assert(PlanExample(j).adviser==0);
    h=History(j);h.review.resupplyTurn=2;assert(PlanExample(j).adviser==0 && !PlanExample(j).reason.empty());
    // The split-defense proposal grants no free completed guard. Enemies are removed only to isolate movement/arrival.
    auto defense=Council(2);auto& d=defense->journey;Evidence(History(d),2);assert(d.ApplyPlanExample());defense->beforeBattle=d;
    assert(d.Prepare(1,0));for(int enemy:{3,4}) d.world.armies[enemy].troops=0;d.RefreshIntelligence();
    defense->realtime.ApplyAssignments(d);assert(defense->realtime.orders[1].kind==JourneyDirective::None);
    assert(d.Move(1,Campaign::At(6,5))); // The player explicitly moves the unassigned support officer.
    for(int cycles=0;cycles<10 && d.stage==JourneyStage::Battle && !(d.assignments[0].completed && d.assignments[2].completed);++cycles)
        assert(defense->realtime.Resolve(d));
    assert(d.assignments[0].completed && d.assignments[2].completed && !d.assignments[1].completed);
    assert(d.world.armies[1].tile==Campaign::At(6,5) && defense->realtime.orders[1].kind==JourneyDirective::None);
    auto restored=std::make_unique<JourneySaveData>();assert(DecodeJourneySave(Bytes(*defense),*restored));
    assert(restored->journey.assignments[2].completed && !restored->beforeBattle->assignments[2].completed);
    std::cout<<"Plan advice PASS: settled-history gates and priorities, public preview, unpaid adoption, editable duties, actual deploy/march/guard, save and legacy fallback\n";
}
