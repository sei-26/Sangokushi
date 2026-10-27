#include "TacticChain.hpp"

namespace frontline
{
	ChainEvent TacticChain::Apply(Campaign& world, const ChainOrder& order, int combo)
	{
		ChainEvent event;
		event.army = order.army; event.enemy = order.enemy; event.tactic = order.tactic;
		const auto fail = [&](const char32_t* reason) { event.reason = reason; return event; };
		if (!Living(world, order.army) || world.armies[order.army].faction != world.player)
			return fail(U"担当部隊が戦場にいない。後続の戦法は保留した。");
		if (!CanUse(world, order.army, order.tactic)) return fail(U"この武将は、その戦法を担当できない。");
		const auto& definition = Definition(order.tactic);
		if (spent[order.army]) return fail(U"担当部隊はこの指揮で戦法・攻撃を使用済み。");
		if (definition.unique && signatureUsed[order.army]) return fail(U"固有戦法はこの戦闘ですでに使用済み。次の指揮でも回復しない。");
		auto& actor = world.armies[order.army];
		if (actor.troops < 150 || actor.morale < std::max(15, definition.moraleCost))
			return fail(U"担当部隊の兵力か士気が不足。戦法の士気消費を確認しよう。");
		const auto before = world.armies;
		const auto oldConcealed = concealed, oldDisordered = disordered, oldPressured = pressured, oldInspired = inspired;
		if (!definition.targetsEnemy)
		{
			// Pay before self recovery so a lone officer benefits even at the morale cap.
			actor.morale -= definition.moraleCost;
			for (int i = 0; i < static_cast<int>(world.armies.size()); ++i)
			{
				auto& ally = world.armies[i];
				if (!Living(world, i) || ally.faction != actor.faction || ally.arm == Arm::Transport || world.MapDistance(actor.tile, ally.tile) > 2) continue;
				ally.morale = std::min(100, ally.morale + 18);
				disordered[i] = pressured[i] = false; inspired[i] = true;
			}
			event.enemy = -1;
			event.reason = U"義勇の誓い。自隊と周囲2マスの味方の士気を上限まで18回復、混乱と隙を解除。次の攻撃を20％強化（使用まで持続）。";
		}
		else
		{
			if (!Living(world, order.enemy) || !world.Hostile(world.player, world.armies[order.enemy].faction)) return fail(U"対象の敵がいない。後続の戦法は保留した。");
			auto& victim = world.armies[order.enemy];
			if (world.MapDistance(actor.tile, victim.tile) != 1) return fail(U"敵が隣接していない。誘導先と担当の配置を見直そう。");
			event.from = victim.tile;
			const int leadership = world.generals[actor.general].leadership;
			int damage = 0, moraleLoss = 0;
			switch (order.tactic)
			{
			case ChainTactic::Lure:
				if (victim.stance == battle::Stance::Guard) return fail(U"敵は固守して誘いを追わない。牽制から崩すか、別の相手から組み直そう。");
				if (moved[order.army]) return fail(U"誘導役は移動済み。後退する余力がない。");
				if (!Free(world, order.destination) || world.MapDistance(actor.tile, order.destination) != 1 || world.MapDistance(victim.tile, order.destination) <= 1) return fail(U"敵から離れる隣接マスへ後退先を指定しよう。");
				{
					const int old = actor.tile;
					actor.tile = actor.target = order.destination; victim.tile = victim.target = old;
					actor.troops -= std::min(60, actor.troops - 1); moved[order.army] = true;
				}
				event.reason = U"敵が追撃し、誘導役のいたマスへ入った。囮の兵60を消耗。";
				break;
			case ChainTactic::Ambush:
				damage = actor.troops / 8 + leadership * 2;
				if (concealed[order.army] && world.tiles[actor.tile].terrain == Terrain::Forest)
				{
					event.setupPercent = 125; moraleLoss = 25; disordered[order.enemy] = true;
					event.reason = U"準備した森林の伏兵で急襲。仕込みで威力25％増・士気25低下・混乱。";
				}
				else
				{
					moraleLoss = 12; pressured[order.enemy] = true;
					event.reason = U"準備なしの急襲。士気12低下・隙。森林で伏兵を準備すれば威力と妨害が強まる。";
				}
				break;
			case ChainTactic::Breakthrough:
				if (disordered[order.enemy] || victim.morale <= 60)
				{
					event.setupPercent = 125;
					const int push = PushTile(world, order.army, order.enemy);
					if (!Free(world, push)) return fail(U"押し出す先が塞がれている。後続の戦法は保留した。");
					actor.tile = actor.target = victim.tile; victim.tile = victim.target = push; moved[order.army] = true;
				}
				damage = actor.troops / 6 + leadership * 2; moraleLoss = 15;
				event.reason = event.setupPercent > 100
				    ? U"崩れた隊列を突破。仕込みで威力25％増。敵を１マス押し出し、空いた場所へ進んだ。"
				    : U"単独で正面へ突撃。士気15低下。混乱中か士気60以下なら威力25％増と押し出し。";
				break;
			case ChainTactic::Feint:
				damage = actor.troops / 16 + leadership / 2; moraleLoss = 12; pressured[order.enemy] = true;
				event.reason = U"牽制で敵の隙を作った。士気12低下。この指揮の間、崩し・青龍偃月をつなげられる。";
				break;
			case ChainTactic::Disrupt:
				damage = actor.troops / 12 + leadership;
				victim.stance = battle::Stance::Balanced;
				if (pressured[order.enemy])
				{
					event.setupPercent = 125; moraleLoss = 18;
					disordered[order.enemy] = true; pressured[order.enemy] = false;
					event.reason = U"隙を突いて隊列を崩した。仕込みで威力25％増・固守解除・士気18低下・混乱。";
				}
				else
				{
					moraleLoss = 10;
					event.reason = U"正面から構えを崩す。固守解除・士気10低下。先に牽制で隙を作れば威力25％増と混乱。";
				}
				break;
			case ChainTactic::AzureDragon:
				if (pressured[order.enemy] || disordered[order.enemy] || victim.morale <= 60) event.setupPercent = 125;
				damage = actor.troops / 4 + leadership * 3; moraleLoss = 22;
				victim.stance = battle::Stance::Balanced; disordered[order.enemy] = true; pressured[order.enemy] = false;
				event.reason = event.setupPercent > 100
				    ? U"青龍偃月。敵の隙・混乱・低士気を見切り、仕込みで威力25％増。固守解除・士気22低下・混乱。"
				    : U"青龍偃月。単独で敵陣を一閃。固守解除・士気22低下・混乱。隙・混乱・低士気なら威力25％増。";
				break;
			case ChainTactic::SerpentRoar:
				damage = actor.troops / 10 + leadership; moraleLoss = 25;
				victim.stance = battle::Stance::Balanced; disordered[order.enemy] = true;
				event.reason = U"燕人の大喝。敵の固守を破り、士気25低下・混乱。事前の隙なしで次の一手を開いた。";
				break;
			default: return fail(U"担当できない戦法。");
			}
			if (damage > 0)
			{
				const int prepared = damage * event.setupPercent / 100;
				const int supported = prepared * (inspired[order.army] ? 120 : 100) / 100;
				event.damage = std::min(victim.troops, supported * ComboPercent(combo) / 100);
				if (inspired[order.army]) event.reason += U" 鼓舞の20％強化を消費。";
				inspired[order.army] = false; victim.troops -= event.damage;
			}
			victim.morale = std::max(0, victim.morale - moraleLoss); victim.path.clear(); event.to = victim.tile;
		}
		actor.path.clear(); concealed[order.army] = false; spent[order.army] = true;
		if (definition.targetsEnemy) actor.morale -= definition.moraleCost;
		if (definition.unique) signatureUsed[order.army] = true;
		event.combo = combo; event.comboPercent = ComboPercent(combo);
		const auto n = std::to_string(combo), percent = std::to_string(event.comboPercent);
		event.reason += U" " + std::u32string(n.begin(), n.end()) + U"コンボ / 火力" + std::u32string(percent.begin(), percent.end()) + U"％。";
		event.success = true; ++world.revision;
		for (int i = 0; i < static_cast<int>(world.armies.size()); ++i)
		{
			const auto& unit = world.armies[i]; const auto& old = before[i];
			if (unit.tile != old.tile || unit.troops != old.troops || unit.morale != old.morale || concealed[i] != oldConcealed[i] || disordered[i] != oldDisordered[i] || pressured[i] != oldPressured[i] || inspired[i] != oldInspired[i])
				event.changes.push_back({i, unit.tile, unit.troops, unit.morale, concealed[i], disordered[i], pressured[i], inspired[i]});
		}
		return event;
	}
}
