#include "StrategistJourney.hpp"
#include "TacticBattlefield.hpp"
#include <memory>

namespace frontline
{
    namespace
    {
        JourneyEnemyContext Context(const StrategistJourney& journey)
        {
            return {journey.Definition().kind, journey.goal, journey.Definition().secondGoal,
                    journey.enemyPolicy, journey.HasConvoy(),journey.campaignFlow};
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
        std::vector<JourneyEnemyPlan> AdaptivePlans(const Campaign& observed,const TacticChain& chain,const JourneyEnemyContext& context,bool campaign)
        {
            auto plans=PlanJourneyEnemies(observed,chain,context);
            if(!campaign || context.policy!=1) return plans;
            for(auto& plan:plans)
            {
                if(plan.intent!=JourneyEnemyIntent::Guard ||
                   (context.objective==JourneyObjective::CaptureCamp && plan.from==context.goal)) continue;
                int attacker=-1;
                for(int a=0;a<int(observed.armies.size());++a)
                    if(observed.armies[a].troops>0 && observed.Hostile(observed.armies[a].faction,observed.armies[plan.army].faction)) { attacker=a;break; }
                if(attacker<0) continue;
                // The supplied world already contains only this side's contacts.
                // Do not re-hide a concealed officer who has actually been spotted.
                const int pressure=ReadTacticBattlefield(observed,{},attacker,plan.army).directions;
                if(pressure<2) continue;
                auto Exposure=[&](const Campaign& position)
                {
                    int risk=0;
                    for(int a=0;a<int(position.armies.size());++a)
                    {
                        const auto& opponent=position.armies[a];
                        if(opponent.troops<=0 || opponent.arm==Arm::Transport || !position.Hostile(opponent.faction,position.armies[plan.army].faction) ||
                           !position.CanStrike(a,position.armies[plan.army].tile)) continue;
                        risk+=ReadTacticBattlefield(position,{},a,plan.army).Damage(100+opponent.troops/12);
                    }
                    return risk;
                };
                const int currentRisk=Exposure(observed);
                auto trial=std::make_unique<Campaign>(observed);int best=10000000,chosen=plan.from;
                // Include the cost of losing the brace, not only the number of fronts.
                trial->armies[plan.army].stance=battle::Stance::Balanced;
                for(int tile:observed.MapNeighbors(plan.from))
                {
                    if(!StrategistJourney::OnMap(tile) || tile%Width<1 || tile/Width<1 ||
                       observed.CityAt(tile)>=0 || !TacticChain::Free(observed,tile)) continue;
                    bool reserved=false;
                    for(const auto& other:plans) if(other.army!=plan.army && (other.from==tile || other.next==tile)) reserved=true;
                    if(reserved) continue;
                    trial->armies[plan.army].tile=tile;
                    const int nextPressure=ReadTacticBattlefield(*trial,{},attacker,plan.army).directions;
                    if(nextPressure>=pressure) continue;
                    const int risk=Exposure(*trial);
                    if(risk>=currentRisk) continue;
                    const int score=risk*100+observed.MapDistance(tile,context.goal);
                    if(score<best) { best=score;chosen=tile; }
                }
                if(chosen==plan.from) continue;
                plan.next=chosen;plan.danger.clear();trial->armies[plan.army].tile=chosen;
                for(int y=1;y<13;++y) for(int x=1;x<15;++x)
                {
                    const int tile=Campaign::At(x,y);
                    if(tile!=chosen && trial->tiles[tile].terrain!=Terrain::Sea && trial->CanStrike(plan.army,tile)) plan.danger.push_back(tile);
                }
            }
            return plans;
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
        return AdaptivePlans(*view,chain,Context(*this),campaignFlow);
    }
    std::vector<JourneyEnemyPlan> StrategistJourney::PlanEnemyTurn() const
    {
        auto observed=EnemyView(world,intelligence[1]);
        return AdaptivePlans(*observed,chain,Context(*this),campaignFlow);
    }
    void StrategistJourney::ApplyEnemyPlans(const std::vector<JourneyEnemyPlan>& plans)
    {
        for (const auto& plan : plans)
            if (plan.next!=plan.from && TacticChain::Free(world,plan.next))
            {
                world.armies[plan.army].tile=world.armies[plan.army].target=plan.next;
                // Re-forming costs the guard's brace. A blocked move above never pays this cost.
                if(campaignFlow && world.armies[plan.army].stance==battle::Stance::Guard) world.armies[plan.army].stance=battle::Stance::Balanced;
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
        if(campaignFlow && mission==YellowTurbanMission && world.generals[3].tactic==Tactic::Rally &&
           !chain.disordered[3] && world.armies[3].morale<=85 &&
           intelligence[1].EnemyNear(world,world.armies[3].tile,1))
            world.ActivateTactic(3,true);
        // Shared combat remains authoritative. Ambushers halve ordinary offense.
        for (int i = 0; i < 3; ++i) world.armies[i].scouting = chain.concealed[i];
        world.AdvanceBattleDay();
        for(const auto& event:world.tacticEvents)
        {
            const auto* enemy=intelligence[0].Find(3);
            if(event.general==3 && event.tactic==Tactic::Rally && enemy && enemy->current)
                Report(U"張梁が「鼓舞」を発動。近くの黄巾兵の士気が上がった。");
        }
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
