#include "StrategistJourney.hpp"
#include <memory>

namespace frontline
{
    namespace
    {
        JourneyEnemyContext Context(const StrategistJourney& journey)
        {
            return {journey.Definition().kind, journey.goal, journey.Definition().secondGoal,
                    journey.enemyPolicy, journey.HasConvoy()};
        }
        std::unique_ptr<Campaign> EnemyView(const Campaign& world, const BattleIntelligence& known)
        {
            auto view = std::make_unique<Campaign>(world);
            for (auto& unit : view->armies) if (unit.faction != 1) unit.troops = 0;
            for (const auto& contact : known.contacts)
                if (contact.current && contact.id >= 0 && contact.id < static_cast<int>(view->armies.size()))
                    view->armies[contact.id] = contact.observed;
            return view;
        }
    }
    std::vector<JourneyEnemyPlan> StrategistJourney::ReadEnemyPlans() const
    {
        if (stage != JourneyStage::Battle) return {};
        // Remove unseen enemy allies before estimating their observation and paths.
        auto observed = std::make_unique<Campaign>(world);
        for (auto& unit : observed->armies) if (unit.faction != 0) unit.troops = 0;
        for (const auto& contact : intelligence[0].contacts)
            if (contact.current && contact.id >= 0 && contact.id < static_cast<int>(observed->armies.size()))
                observed->armies[contact.id] = contact.observed;
        // The estimate derives sight from seen troops. Do not infer additional
        // enemy sight from the true, potentially unknown city garrisons.
        for (auto& city : observed->cities) city.troops = 0;
        for (int id = 0; id < 3 && id < static_cast<int>(observed->armies.size()); ++id)
        {
            if (!chain.concealed[id]) continue;
            bool exposed = false;
            for (const auto& enemy : observed->armies)
                if (enemy.faction == 1 && enemy.troops > 0 &&
                    observed->MapDistance(enemy.tile, observed->armies[id].tile) <= (enemy.scouting ? 3 : 1) &&
                    observed->ClearShot(enemy.tile, observed->armies[id].tile)) exposed = true;
            if (!exposed) observed->armies[id].troops = 0;
        }
        BattleIntelligence assumed;
        assumed.Reset(*observed); assumed.Observe(*observed, 1);
        auto view = EnemyView(*observed, assumed);
        return PlanJourneyEnemies(*view, chain, Context(*this));
    }
    std::vector<JourneyEnemyPlan> StrategistJourney::PlanEnemyTurn() const
    {
        auto observed=EnemyView(world,intelligence[1]);
        return PlanJourneyEnemies(*observed,chain,Context(*this));
    }
    void StrategistJourney::ApplyEnemyPlans(const std::vector<JourneyEnemyPlan>& plans)
    {
        for (const auto& plan : plans)
            if (plan.next!=plan.from && TacticChain::Free(world,plan.next))
            {
                world.armies[plan.army].tile=world.armies[plan.army].target=plan.next;
                ++world.revision;
            }
        RefreshIntelligence();
    }
    void StrategistJourney::EnemyTurn()
    {
        RefreshIntelligence();
        ApplyEnemyPlans(PlanEnemyTurn());
    }
    bool StrategistJourney::EndTurn()
    {
        if (stage!=JourneyStage::Battle) return false;
        const auto previousReports=reports.size();
        EnemyTurn();
        return CompleteCommandCycle(previousReports);
    }
    bool StrategistJourney::CompleteCommandCycle(std::size_t previousReports)
    {
        // Shared combat remains authoritative. Ambushers halve ordinary offense.
        for (int i = 0; i < 3; ++i) world.armies[i].scouting = chain.concealed[i];
        world.AdvanceBattleDay();
        for (int i = 0; i < 3; ++i) world.armies[i].scouting = false;
        ++turn;
        RefreshIntelligence();
        AdvanceObjective();
        CheckReliefOutcome();
        RefreshIntelligence();
        Evaluate();
        if (stage == JourneyStage::Battle)
        {
            chain.BeginTurn(world);
            for (int i = 0; i < 3; ++i)
                if (guarding[i]) world.armies[i].stance = battle::Stance::Balanced;
            guarding = {};
            if (reports.size() == previousReports)
                Report(U"敵が動き、接近部隊が交戦した。新しい配置から次の一手を考えよう。");
        }
        return true;
    }
}
