#include "../NewGame/StrategistJourney.hpp"
#include <cassert>
#include <iostream>
#include <memory>

using namespace frontline;
static const JourneyEnemyPlan& Find(const std::vector<JourneyEnemyPlan>& plans, int id)
{
    for (const auto& plan : plans) if (plan.army == id) return plan;
    assert(false); std::abort();
}
static bool Same(const std::vector<JourneyEnemyPlan>& a, const std::vector<JourneyEnemyPlan>& b)
{
    if (a.size() != b.size()) return false;
    for (size_t i = 0; i < a.size(); ++i)
        if (a[i].army != b[i].army || a[i].from != b[i].from || a[i].next != b[i].next ||
            a[i].target != b[i].target || a[i].targetArmy != b[i].targetArmy ||
            a[i].intent != b[i].intent || a[i].danger != b[i].danger) return false;
    return true;
}
int main()
{
    auto j = std::make_unique<StrategistJourney>();
    j->Reset(); j->Meet(); assert(j->Prepare(1, 1));
    const auto read = j->ReadEnemyPlans();
    assert(Find(read, 3).intent == JourneyEnemyIntent::Guard);
    assert(Find(read, 3).next == j->world.armies[3].tile);
    assert(!Find(read, 3).danger.empty());
    const auto revision = j->world.revision;
    const auto morale = j->world.armies[0].morale;
    assert(Same(read, j->ReadEnemyPlans()) && j->world.revision == revision && j->world.armies[0].morale == morale);
    // A private order, hidden reserve position, status or scouting cannot change
    // the estimate. The observed snapshot is deliberately not refreshed here.
    j->world.armies[4].tile = Campaign::At(1, 1);
    j->world.armies[4].target = Campaign::At(9, 9);
    j->world.armies[4].scouting = true; j->chain.disordered[4] = true;
    j->world.armies[3].path = {Campaign::At(1, 2)};
    j->world.armies[3].target = Campaign::At(1, 2);
    j->world.cities[1].troops = 0;
    assert(Same(read, j->ReadEnemyPlans()));
    // Breaking a guard actually releases its anchored behavior. Disorder keeps
    // it stationary until BeginTurn, rather than secretly allowing a move.
    j->world.armies[3].stance = battle::Stance::Balanced;
    j->chain.disordered[3] = true; j->RefreshIntelligence();
    assert(Find(j->ReadEnemyPlans(), 3).intent == JourneyEnemyIntent::Recover);
    const auto stopped = j->world.armies[3].tile;
    assert(j->EndTurn() && j->world.armies[3].tile == stopped);
    assert(!j->chain.disordered[3]);
    assert(Find(j->ReadEnemyPlans(), 3).intent != JourneyEnemyIntent::Guard);

    j->Reset(); j->Meet(); j->mission = 1; assert(j->Prepare(1, 0));
    // A detached reserve sees the convoy and a more distant screen.
    j->world.armies[3].troops = 0;
    j->world.armies[4].tile = Campaign::At(9, 11);
    j->world.armies[0].tile = Campaign::At(9, 9);
    j->world.armies[1].tile = Campaign::At(8, 8);
    j->world.armies[2].tile = Campaign::At(8, 7);
    j->RefreshIntelligence();
    assert(j->intelligence[1].Find(5) && j->intelligence[1].Find(5)->current);
    const auto convoyRead = j->ReadEnemyPlans();
    const auto intercept = Find(convoyRead, 4);
    assert(intercept.intent == JourneyEnemyIntent::Intercept && intercept.targetArmy == 5);
    assert(intercept.from != intercept.next && j->world.MapDistance(intercept.from, intercept.next) == 1);
    assert(j->EndTurn() && j->world.armies[4].tile == intercept.next);

    // Adjacent guards prevent ignoring the screen just to chase the cart.
    j->world.armies[0].tile = j->world.MapNeighbors(j->world.armies[4].tile)[0];
    j->RefreshIntelligence();
    assert(Find(j->ReadEnemyPlans(), 4).intent != JourneyEnemyIntent::Intercept);

    j->Reset(); j->Meet(); assert(j->Prepare(1, 2));
    assert(j->intelligence[0].Find(4)->observed.scouting);
    assert(Find(j->ReadEnemyPlans(), 4).intent == JourneyEnemyIntent::Search);

    j->Reset(); j->Meet(); assert(j->Prepare(0, 0));
    j->world.armies[0].tile = Campaign::At(2, 10);
    j->world.armies[2].tile = Campaign::At(2, 9);
    j->world.armies[4].troops = 0;
    j->RefreshIntelligence();
    const auto* hidden = j->intelligence[1].Find(1);
    assert(j->chain.concealed[1] && (!hidden || !hidden->current));
    const int before = j->world.armies[3].tile;
    assert(j->EndTurn() && j->world.armies[3].tile == before);

    // Pure planner reserves one-cell destinations and routes around barriers.
    JourneyEnemyContext context;
    context.objective = JourneyObjective::HoldPositions;
    context.goal = context.secondGoal = Campaign::At(6, 5);
    for (auto& unit : j->world.armies) unit.troops = 0;
    j->world.armies[3].troops = j->world.armies[4].troops = 1300;
    j->world.armies[3].tile = Campaign::At(10, 5);
    j->world.armies[4].tile = Campaign::At(10, 6);
    j->world.tiles[Campaign::At(9,5)].terrain = Terrain::River;
    j->chain.BeginTurn(j->world);
    const auto planned = PlanJourneyEnemies(j->world, j->chain, context);
    assert(planned.size() == 2 && planned[0].next != planned[1].next);
    for (const auto& plan : planned)
    {
        assert(plan.intent == JourneyEnemyIntent::Assault);
        assert(j->world.MapDistance(plan.from, plan.next) == 1);
        assert(j->world.tiles[plan.next].terrain != Terrain::River);
    }
    assert(Same(planned, PlanJourneyEnemies(j->world, j->chain, context)));
    std::cout << "Enemy reads: privacy, immutable estimate, guard break, recovery, convoy, screen, scout, paths and reservations passed\n";
}
