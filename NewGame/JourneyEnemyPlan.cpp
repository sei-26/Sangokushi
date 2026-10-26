#include "JourneyEnemyPlan.hpp"
#include <memory>
#include <queue>

namespace frontline
{
	namespace
	{
		bool OnMap(int tile)
		{
			return Campaign::Valid(tile) && tile % Width < 16 && tile / Width < 14;
		}
		// Breadth-first search reaches a cell next to the target; rivers, friends
		// and observed opponents block paths. No hidden occupancy is queried.
		int NextStep(const Campaign& view, int from, int target)
		{
			if (target < 0 || view.MapDistance(from, target) <= 1) return from;
			std::array<int, TileCount> first;
			first.fill(-1);
			first[from] = from;
			std::queue<int> open;
			open.push(from);
			while (!open.empty())
			{
				const int current = open.front(); open.pop();
				for (int tile : view.MapNeighbors(current))
				{
					if (!OnMap(tile) || first[tile] >= 0 || !TacticChain::Free(view, tile)) continue;
					first[tile] = current == from ? tile : first[current];
					if (view.MapDistance(tile, target) <= 1) return first[tile];
					open.push(tile);
				}
			}
			return from;
		}
	}
	std::vector<JourneyEnemyPlan> PlanJourneyEnemies(const Campaign& observed,
	    const TacticChain& chain, const JourneyEnemyContext& context)
	{
		auto view = std::make_unique<Campaign>(observed);
		std::vector<JourneyEnemyPlan> result;
		for (int id = 0; id < static_cast<int>(view->armies.size()); ++id)
		{
			const auto& unit = view->armies[id];
			if (unit.faction != 1 || unit.troops <= 0 || unit.arm == Arm::Transport) continue;
			JourneyEnemyPlan plan;
			plan.army = id; plan.from = plan.next = unit.tile;
			int nearest = 10000, cart = -1;
			for (int other = 0; other < static_cast<int>(view->armies.size()); ++other)
			{
				const auto& target = view->armies[other];
				if (target.troops <= 0 || !view->Hostile(unit.faction, target.faction)) continue;
				const int distance = view->MapDistance(unit.tile, target.tile);
				if (distance < nearest) { nearest = distance; plan.targetArmy = other; plan.target = target.tile; }
				if (target.arm == Arm::Transport && distance <= 4) cart = other;
			}
			plan.intent = plan.target < 0 ? JourneyEnemyIntent::Wait : JourneyEnemyIntent::Engage;
			if (context.objective == JourneyObjective::HoldPositions)
			{
				plan.intent = JourneyEnemyIntent::Assault;
				plan.targetArmy = -1;
				plan.target = id == 3 ? context.secondGoal : context.goal;
			}
			else if (context.convoy && cart >= 0 && nearest > 1)
			{
				// An engaged unit cannot slip through its opponent to snipe a cart.
				plan.intent = JourneyEnemyIntent::Intercept;
				plan.targetArmy = cart; plan.target = view->armies[cart].tile;
			}
			if (unit.scouting && plan.intent == JourneyEnemyIntent::Engage)
				plan.intent = JourneyEnemyIntent::Search;
			if (unit.stance == battle::Stance::Guard && context.policy == 1 && id == 3 &&
			    context.objective != JourneyObjective::HoldPositions)
				plan.intent = JourneyEnemyIntent::Guard;
			if (id < static_cast<int>(chain.disordered.size()) && chain.disordered[id])
				plan.intent = JourneyEnemyIntent::Recover;
			if (plan.intent != JourneyEnemyIntent::Recover && plan.intent != JourneyEnemyIntent::Guard)
				plan.next = NextStep(*view, unit.tile, plan.target);
			view->armies[id].tile = view->armies[id].target = plan.next;
			// This is a possible attack area after the estimated one-cell move,
			// not guaranteed damage or the enemy's private command.
			for (int y = 1; y < 13; ++y) for (int x = 1; x < 15; ++x)
			{
				const int tile = Campaign::At(x, y);
				if (tile != plan.next && view->tiles[tile].terrain != Terrain::Sea && view->CanStrike(id, tile))
					plan.danger.push_back(tile);
			}
			result.push_back(std::move(plan));
		}
		return result;
	}
	const char32_t* EnemyIntentName(JourneyEnemyIntent intent)
	{
		switch (intent)
		{
		case JourneyEnemyIntent::Engage: return U"接近・交戦";
		case JourneyEnemyIntent::Guard: return U"陣を維持";
		case JourneyEnemyIntent::Search: return U"索敵しつつ接近";
		case JourneyEnemyIntent::Intercept: return U"護送車へ接近";
		case JourneyEnemyIntent::Assault: return U"烽火へ進軍";
		case JourneyEnemyIntent::Recover: return U"混乱・移動停止";
		default: return U"様子をうかがう";
		}
	}
	const char32_t* EnemyIntentCounter(JourneyEnemyIntent intent)
	{
		switch (intent)
		{
		case JourneyEnemyIntent::Guard: return U"誘導は不可。牽制→崩しで固守を解除。";
		case JourneyEnemyIntent::Search: return U"斥候は射線の通る3マス内を看破。森だけでは隠せない。";
		case JourneyEnemyIntent::Intercept: return U"護衛を車との間へ。混乱で敵の足を止めよう。";
		case JourneyEnemyIntent::Assault: return U"拠点で固守。敵2隊の集中を避け、片方を引き離そう。";
		case JourneyEnemyIntent::Recover: return U"この指揮は移動不可。通常交戦は続くため、隣接に注意。";
		case JourneyEnemyIntent::Engage: return U"誘導で孤立させるか、固守で受け止めよう。";
		default: return U"見える敵からの読み。視界外の動きにも備えよう。";
		}
	}
}
