#include "TacticChain.hpp"
#include <memory>

namespace frontline
{
	bool TacticChain::Living(const Campaign& world, int army)
	{
		return army >= 0 && army < static_cast<int>(world.armies.size()) && world.armies[army].troops > 0;
	}
	bool TacticChain::Free(const Campaign& world, int tile)
	{
		if (!Campaign::Valid(tile) || world.tiles[tile].terrain == Terrain::Sea ||
		    world.tiles[tile].terrain == Terrain::River || world.CityAt(tile) >= 0)
			return false;
		for (const auto& army : world.armies)
			if (army.troops > 0 && army.tile == tile)
				return false;
		return true;
	}
	int TacticChain::PushTile(const Campaign& world, int attacker, int enemy)
	{
		if (!Living(world, attacker) || !Living(world, enemy))
			return -1;
		const auto a =
		    hexgrid::CubeAt(world.armies[attacker].tile % Width, world.armies[attacker].tile / Width);
		const auto b = hexgrid::CubeAt(world.armies[enemy].tile % Width, world.armies[enemy].tile / Width);
		const auto p = hexgrid::Offset(2 * b.q - a.q, 2 * b.r - a.r, 2 * b.s - a.s);
		if (p.first < 0 || p.first >= Width || p.second < 0 || p.second >= Height)
			return -1;
		return Campaign::At(p.first, p.second);
	}
	void TacticChain::Reset(const Campaign& world)
	{
		concealed.assign(world.armies.size(), false);
		signatureUsed.assign(world.armies.size(), false);
		BeginTurn(world);
	}
	void TacticChain::BeginTurn(const Campaign& world)
	{
		moved.assign(world.armies.size(), false);
		spent.assign(world.armies.size(), false);
		disordered.assign(world.armies.size(), false);
		pressured.assign(world.armies.size(), false);
		inspired.assign(world.armies.size(), false);
		signatureUsed.resize(world.armies.size(), false);
		concealed.resize(world.armies.size(), false);
		for (int i = 0; i < static_cast<int>(world.armies.size()); ++i)
			if (!Living(world, i))
				concealed[i] = false;
	}
	bool TacticChain::ArmAmbush(Campaign& world, int army, std::u32string& reason)
	{
		if (!Living(world, army) || world.armies[army].faction != world.player || moved[army] || spent[army])
		{
			reason = U"この隊は今の指揮で伏兵を準備できない。";
			return false;
		}
		const auto& unit = world.armies[army];
		if (world.generals[unit.general].tactic != Tactic::Ambush ||
		    world.tiles[unit.tile].terrain != Terrain::Forest)
		{
			reason = U"伏兵役を森林に置いてから準備する。";
			return false;
		}
		for (const auto& enemy : world.armies)
			if (enemy.troops > 0 && world.Hostile(unit.faction, enemy.faction) &&
			    world.MapDistance(unit.tile, enemy.tile) <= 1)
			{
				reason = U"敵が隣接しているため、身を隠せない。";
				return false;
			}
		concealed[army] = moved[army] = true;
		reason = U"伏兵を準備。移動すると解除。接近や敵の索敵で見破られる。";
		++world.revision;
		return true;
	}
	bool TacticChain::Move(Campaign& world, const BattleIntelligence& known, int army, int tile,
	                       std::u32string& reason)
	{
		if (!Living(world, army) || world.armies[army].faction != world.player || moved[army] ||
		    world.MapDistance(world.armies[army].tile, tile) != 1 || !Campaign::Valid(tile) ||
		    !known.visible[tile])
		{
			reason = U"未移動の隊を選び、確認できる隣接マスを指定する。";
			return false;
		}
		if (!Free(world, tile))
		{
			reason = U"そのマスへは進めない。地形と部隊を確認しよう。";
			return false;
		}
		auto& unit = world.armies[army];
		unit.tile = unit.target = tile;
		unit.path.clear();
		concealed[army] = false;
		moved[army] = true;
		++world.revision;
		reason = U"配置を変更。移動は各隊、指揮ごとに１マス。戦法は続けて使える。";
		return true;
	}
	bool TacticChain::Strike(Campaign& world, const BattleIntelligence& known, int army, int enemy,
	                         std::u32string& reason)
	{
		const auto* contact = known.Find(enemy);
		if (!Living(world, army) || world.armies[army].faction != world.player || spent[army] || !contact ||
		    !contact->current || !Living(world, enemy) || !world.CanStrike(army, world.armies[enemy].tile))
		{
			reason = U"未攻撃の隊で、射程内の確認中の敵を指定する。";
			return false;
		}
		auto& victim = world.armies[enemy];
		const auto& actor = world.armies[army];
		const int damage = std::min(victim.troops, (100 + actor.troops / 12) * (inspired[army] ? 120 : 100) / 100);
		victim.troops -= damage;
		victim.morale = std::max(0, victim.morale - 5);
		spent[army] = true;
		inspired[army] = false;
		concealed[army] = false;
		++world.revision;
		const auto number = std::to_string(damage);
		reason = world.generals[actor.general].name + U"隊が攻撃。敵兵を" +
		         std::u32string(number.begin(), number.end()) + U"減らした。";
		return true;
	}
	ChainResult TacticChain::Resolve(Campaign& world, const std::vector<ChainOrder>& orders)
	{
		ChainResult result;
		if (orders.empty() || orders.size() > 3)
			return result;
		const auto count = world.armies.size();
		for (auto* flags : {&concealed, &moved, &spent, &disordered, &pressured, &inspired, &signatureUsed}) flags->resize(count, false);
		for (const auto& order : orders)
		{
			auto event = Apply(world, order);
			result.events.push_back(event);
			if (!event.success)
				break; // Unspent followers remain available for a new plan.
			++result.completed;
		}
		return result;
	}
	ChainResult TacticChain::Preview(const Campaign& world, const BattleIntelligence& known,
	                                 const std::vector<ChainOrder>& orders) const
	{
		auto view = std::make_unique<Campaign>(world);
		for (int i = 0; i < static_cast<int>(view->armies.size()); ++i)
		{
			if (!world.Hostile(world.player, view->armies[i].faction))
				continue;
			const auto* contact = known.Find(i);
			view->armies[i] = contact && contact->current ? contact->observed : Army{};
		}
		auto local = *this;
		auto result = local.Resolve(*view, orders);
		result.prediction = true;
		return result;
	}
	ChainResult TacticChain::Execute(Campaign& world, const BattleIntelligence& known,
	                                 const std::vector<ChainOrder>& orders)
	{
		// Validate all initial targets against the same observation before any
		// action mutates the world. A bad hidden target must not probe reality.
		for (const auto& order : orders)
		{
			if (!Definition(order.tactic).targetsEnemy) continue;
			const auto* contact = known.Find(order.enemy);
			if (!contact || !contact->current)
			{
				ChainResult result;
				ChainEvent event;
				event.army = order.army;
				event.enemy = order.enemy;
				event.tactic = order.tactic;
				event.reason = U"確認中の敵を指定する。未確認の敵へは戦法を予約できない。";
				result.events.push_back(event);
				return result;
			}
		}
		return Resolve(world, orders);
	}
} // namespace frontline
