#include "../NewGame/StrategistJourney.hpp"
#include "../NewGame/JourneyEnemyDoctrine.hpp"
#include "../NewGame/JourneySave.hpp"
#include <cassert>
#include <memory>
using namespace frontline;
static const JourneyEnemyPlan& Find(const std::vector<JourneyEnemyPlan>& plans,int id)
{ for(const auto& p:plans) if(p.army==id) return p;std::abort(); }
static std::unique_ptr<StrategistJourney> Battle(int mission)
{
    auto j=std::make_unique<StrategistJourney>();j->ResetCampaign();j->mission=mission;j->Meet();assert(j->Prepare(1,0));return j;
}
static std::vector<JourneyEnemyPlan> Plans(const StrategistJourney& j)
{
    return PlanJourneyEnemies(j.world,j.chain,{j.Definition().kind,j.goal,j.Definition().secondGoal,j.enemyPolicy,j.HasConvoy(),true});
}
static bool Same(const std::vector<JourneyEnemyPlan>& a,const std::vector<JourneyEnemyPlan>& b)
{
    if(a.size()!=b.size()) return false;
    for(int i=0;i<int(a.size());++i)
        if(a[i].army!=b[i].army || a[i].intent!=b[i].intent || a[i].role!=b[i].role ||
           a[i].from!=b[i].from || a[i].next!=b[i].next || a[i].target!=b[i].target ||
           a[i].targetArmy!=b[i].targetArmy || a[i].danger!=b[i].danger) return false;
    return true;
}
int main()
{
    auto opening=Battle(0);auto& a=*opening;
    // Reserve waits, then genuinely advances after its own vanguard is pressed.
    auto plans=Plans(a);assert(Find(plans,4).intent==JourneyEnemyIntent::Reserve);
    const int reserve=a.world.armies[4].tile;assert(a.EndTurn() && a.world.armies[4].tile==reserve);
    a.world.armies[3].morale=50;a.RefreshIntelligence();plans=Plans(a);
    assert(Find(plans,4).intent==JourneyEnemyIntent::Counterattack && Find(plans,4).next!=reserve);
    const int next=Find(plans,4).next;assert(a.EndTurn() && a.world.armies[4].tile==next);
    // Recovery is authoritative even when the mission calls for a counterattack.
    a.chain.disordered[4]=true;assert(Find(Plans(a),4).intent==JourneyEnemyIntent::Recover);

    auto port=Battle(1);auto& p=*port;plans=Plans(p);
    assert(Find(plans,3).role==JourneyEnemyRole::Anchor && Find(plans,3).next==p.goal);
    assert(Find(plans,4).intent==JourneyEnemyIntent::Reserve);
    // A genuine crossing releases the counterattack, without a timer or spawn.
    p.world.armies[0].tile=p.world.armies[0].target=Campaign::At(12,9);p.RefreshIntelligence();plans=Plans(p);
    assert(Find(plans,4).intent==JourneyEnemyIntent::Counterattack && Find(plans,4).targetArmy==0);
    const int counter=Find(plans,4).next;assert(counter!=p.world.armies[4].tile);
    assert(p.EndTurn() && p.world.armies[4].tile==counter && p.world.armies[3].tile==p.goal);
    // An isolated, displaced defender can really return onto the empty post.
    auto returner=Battle(1);auto& r=*returner;
    r.enemyPolicy=1;r.world.armies[3].stance=battle::Stance::Guard;
    r.world.armies[3].tile=r.world.armies[3].target=Campaign::At(12,10);
    for(int i=0;i<3;++i) r.world.armies[i].tile=r.world.armies[i].target=Campaign::At(3+i,10);
    r.RefreshIntelligence();assert(Find(Plans(r),3).next==r.goal);
    assert(r.EndTurn() && r.world.armies[3].tile==r.goal && r.world.armies[3].stance==battle::Stance::Balanced);

    auto final=Battle(2);auto& f=*final;
    // Give the main attack a clear view of the two wings, with no adjacent screen.
    f.world.armies[4].tile=f.world.armies[4].target=Campaign::At(10,6);
    f.world.armies[0].tile=f.world.armies[0].target=f.goal;
    f.world.armies[1].tile=f.world.armies[1].target=Campaign::At(7,7);
    f.world.armies[2].tile=f.world.armies[2].target=f.Definition().secondGoal;
    f.world.armies[3].tile=f.world.armies[3].target=Campaign::At(12,10);
    // A fully observed world exercises the role logic independently of fog.
    JourneyEnemyContext context{JourneyObjective::HoldPositions,f.goal,f.Definition().secondGoal,0,false,true};
    plans=PlanJourneyEnemies(f.world,f.chain,context);
    assert(Find(plans,3).role==JourneyEnemyRole::Fixing && Find(plans,3).target==context.secondGoal);
    assert(Find(plans,4).role==JourneyEnemyRole::MainAttack && Find(plans,4).target==context.secondGoal);
    auto actual=std::make_unique<StrategistJourney>(f);actual->RefreshIntelligence();
    const auto observedMain=Find(actual->ReadEnemyPlans(),4);
    assert(observedMain.target==context.secondGoal && observedMain.next!=observedMain.from);
    assert(actual->EndTurn() && actual->world.armies[4].tile==observedMain.next);
    // Conceal a real officer with the public ambush command: appearing weaker
    // changes the enemy's actual approach, not just the wording of its hint.
    auto bait=std::make_unique<StrategistJourney>(f);
    bait->world.armies[0].tile=bait->world.armies[0].target=Campaign::At(8,6);
    bait->world.armies[1].tile=bait->world.armies[1].target=bait->goal;
    bait->RefreshIntelligence();assert(Find(bait->ReadEnemyPlans(),4).target==context.secondGoal);
    assert(bait->Ambush(1) && bait->chain.concealed[1]);
    const auto baited=Find(bait->ReadEnemyPlans(),4);assert(baited.target==context.goal && baited.next!=baited.from);
    assert(bait->EndTurn() && bait->world.armies[4].tile==baited.next);
    f.world.armies[0].tile=f.world.armies[0].target=Campaign::At(6,9);
    f.world.armies[1].tile=f.world.armies[1].target=Campaign::At(7,9);
    plans=PlanJourneyEnemies(f.world,f.chain,context);assert(Find(plans,4).target==context.goal);
    assert(!EnemyDoctrineHint(Find(plans,4),context).empty());
    f.world.armies[0].tile=f.world.armies[0].target=Campaign::At(10,5);
    plans=PlanJourneyEnemies(f.world,f.chain,context);assert(Find(plans,4).intent==JourneyEnemyIntent::Engage);

    // Presentation cannot read hidden private displacement or award resources.
    // Compare pure reads directly; a full validated archive is covered below.
    const auto read=p.ReadEnemyPlans();const int revision=p.world.revision;
    p.world.armies[4].tile=Campaign::At(14,2);p.world.armies[4].target=Campaign::At(3,2);
    assert(Same(read,p.ReadEnemyPlans()) && p.world.revision==revision);
    auto stable=Battle(0);auto session=std::make_unique<JourneySaveData>();session->journey=*stable;
    auto before=std::make_unique<StrategistJourney>();before->ResetCampaign();before->Meet();session->beforeBattle=*before;
    std::vector<std::uint8_t> bytes,after;assert(EncodeJourneySave(*session,bytes));
    stable->ReadEnemyPlans();session->journey=*stable;assert(EncodeJourneySave(*session,after) && bytes==after && bytes[4]==12);
    context.campaign=false;plans=PlanJourneyEnemies(f.world,f.chain,context);
    for(const auto& plan:plans) assert(plan.role==JourneyEnemyRole::None && plan.intent==JourneyEnemyIntent::Assault);
}
