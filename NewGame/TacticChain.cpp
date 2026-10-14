#include "TacticChain.hpp"
#include <memory>

namespace frontline
{
	const char32_t* TacticChain::Name(ChainTactic tactic)
	{
		switch (tactic)
		{
		case ChainTactic::Lure:
			return U"誘導";
		case ChainTactic::Ambush:
			return U"伏兵";
		default:
			return U"突破";
		}
	}
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
		BeginTurn(world);
	}
	void TacticChain::BeginTurn(const Campaign& world)
	{
		moved.assign(world.armies.size(), false);
		spent.assign(world.armies.size(), false);
		disordered.assign(world.armies.size(), false);
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
		const int damage = std::min(victim.troops, 100 + actor.troops / 12);
		victim.troops -= damage;
		victim.morale = std::max(0, victim.morale - 5);
		spent[army] = true;
		concealed[army] = false;
		++world.revision;
		const auto number = std::to_string(damage);
		reason = world.generals[actor.general].name + U"隊が攻撃。敵兵を" +
		         std::u32string(number.begin(), number.end()) + U"減らした。";
		return true;
	}
	ChainEvent TacticChain::Apply(Campaign& world, const ChainOrder& order)
	{
		ChainEvent event;
		event.army = order.army;
		event.enemy = order.enemy;
		event.tactic = order.tactic;
		const auto fail = [&](const char32_t* reason) {
			event.reason = reason;
			return event;
		};
		if (!Living(world, order.army) || world.armies[order.army].faction != world.player)
			return fail(U"担当部隊が戦場にいない。後続の戦法は保留した。");
		if (!Living(world, order.enemy) || !world.Hostile(world.player, world.armies[order.enemy].faction))
			return fail(U"対象の敵がいない。後続の戦法は保留した。");
		if (spent[order.army])
			return fail(U"担当部隊はこの指揮で戦法・攻撃を使用済み。");
		auto& actor = world.armies[order.army];
		auto& victim = world.armies[order.enemy];
		if (actor.troops < 150 || actor.morale < 15)
			return fail(U"担当部隊の兵力か士気が不足。");
		if (world.MapDistance(actor.tile, victim.tile) != 1)
			return fail(U"敵が隣接していない。誘導先と担当の配置を見直そう。");
		const auto expected = order.tactic == ChainTactic::Lure     ? Tactic::Rally
		                      : order.tactic == ChainTactic::Ambush ? Tactic::Ambush
		                                                            : Tactic::Charge;
		if (world.generals[actor.general].tactic != expected)
			return fail(U"この武将は、その戦法を担当できない。");
		event.from = victim.tile;
		if (order.tactic == ChainTactic::Lure)
		{
			if (victim.stance == battle::Stance::Guard)
				return fail(U"敵は固守して誘いを追わない。攻撃や別の相手から組み直そう。");
			if (moved[order.army])
				return fail(U"誘導役は移動済み。後退する余力がない。");
			if (!Free(world, order.destination) || world.MapDistance(actor.tile, order.destination) != 1 ||
			    world.MapDistance(victim.tile, order.destination) <= 1)
				return fail(U"敵から離れる隣接マスへ後退先を指定しよう。");
			const int old = actor.tile;
			actor.tile = actor.target = order.destination;
			victim.tile = victim.target = old;
			actor.troops -= std::min(60, actor.troops - 1);
			moved[order.army] = true;
			event.reason = U"敵が追撃し、誘導役のいたマスへ入った。囮の兵60を消耗。";
		}
		else if (order.tactic == ChainTactic::Ambush)
		{
			if (!concealed[order.army] || world.tiles[actor.tile].terrain != Terrain::Forest)
				return fail(U"伏兵が準備されていないか、すでに露見している。");
			event.damage =
			    std::min(victim.troops, actor.troops / 6 + world.generals[actor.general].leadership * 2);
			victim.troops -= event.damage;
			victim.morale = std::max(0, victim.morale - 25);
			disordered[order.enemy] = true;
			event.reason = U"森林から急襲。敵の隊列を崩し、突破をつなげられる。";
		}
		else
		{
			if (!disordered[order.enemy] && victim.morale > 60)
				return fail(U"敵の隊列が整っている。伏兵で崩すか、士気を60以下へ落とそう。");
			const int push = PushTile(world, order.army, order.enemy);
			if (!Free(world, push))
				return fail(U"押し出す先が塞がれている。後続の戦法は保留した。");
			event.damage =
			    std::min(victim.troops, actor.troops / 5 + world.generals[actor.general].leadership * 3);
			victim.troops -= event.damage;
			victim.morale = std::max(0, victim.morale - 15);
			actor.tile = actor.target = victim.tile;
			victim.tile = victim.target = push;
			moved[order.army] = true;
			event.reason = U"崩れた隊列を突破。敵を１マス押し出し、空いた場所へ進んだ。";
		}
		actor.path.clear();
		victim.path.clear();
		concealed[order.army] = false;
		spent[order.army] = true;
		actor.morale = std::max(0, actor.morale - 10);
		event.to = victim.tile;
		event.success = true;
		++world.revision;
		return event;
	}
	ChainResult TacticChain::Resolve(Campaign& world, const std::vector<ChainOrder>& orders)
	{
		ChainResult result;
		if (orders.empty() || orders.size() > 3)
			return result;
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
