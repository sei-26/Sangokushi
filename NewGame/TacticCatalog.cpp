#include "TacticChain.hpp"

namespace frontline
{
	const ChainTacticDefinition& TacticChain::Definition(ChainTactic tactic)
	{
		static const ChainTacticDefinition entries[] = {
		    {ChainTactic::Lure, U"誘導", U"未移動で隣接敵を引き出す。敵から離れる後退先が必要。固守には無効。囮の兵60を消耗。", 10, false, true},
		    {ChainTactic::Ambush, U"伏兵", U"森林で伏兵準備した隊が隣接敵を急襲。士気25低下・混乱を与え、突破へつなぐ。", 10, false, true},
		    {ChainTactic::Breakthrough, U"突破", U"混乱中か士気60以下の隣接敵を押し出す。敵の背後に空きが必要。士気15低下。", 10, false, true},
		    {ChainTactic::Feint, U"牽制", U"隣接敵へ小損害・士気12低下。この指揮の間「隙」を作り、崩しや青龍偃月へつなぐ。", 10, false, true},
		    {ChainTactic::Disrupt, U"崩し", U"牽制で隙ができた隣接敵へ攻撃。固守を解除し、士気18低下・混乱。森林なしで突破へつなぐ。", 10, false, true},
		    {ChainTactic::Oath, U"義勇の誓い", U"周囲2マスの味方戦闘隊を鼓舞。士気18回復・混乱と隙を解除。今の指揮の次の攻撃を20％強化。", 25, true, false},
		    {ChainTactic::AzureDragon, U"青龍偃月", U"隙・混乱中か士気60以下の隣接敵を一閃。大損害・士気22低下・混乱・固守解除。押し出し不要。", 25, true, true},
		    {ChainTactic::SerpentRoar, U"燕人の大喝", U"隣接敵を威圧し、小損害・士気25低下・混乱・固守解除。事前の隙なしで連鎖の起点を作る。", 30, true, true}};
		for (const auto& entry : entries) if (entry.id == tactic) return entry;
		static const ChainTacticDefinition unknown{ChainTactic::Lure, U"不明な戦法", U"担当できない戦法。", 0, false, true};
		return unknown;
	}
	const char32_t* TacticChain::Name(ChainTactic tactic) { return Definition(tactic).name; }
	std::vector<ChainTactic> TacticChain::Loadout(const Campaign& world, int army)
	{
		if (!Living(world, army)) return {};
		const auto& unit = world.armies[army];
		if (unit.faction != world.player || unit.arm == Arm::Transport || unit.general < 0 || unit.general >= static_cast<int>(world.generals.size())) return {};
		const auto& general = world.generals[unit.general];
		std::vector<ChainTactic> result;
		if (general.tactic == Tactic::Rally) result.push_back(ChainTactic::Lure);
		else if (general.tactic == Tactic::Ambush) result.push_back(ChainTactic::Ambush);
		else if (general.tactic == Tactic::Charge) result.push_back(ChainTactic::Breakthrough);
		result.push_back(ChainTactic::Feint); result.push_back(ChainTactic::Disrupt);
		// Canonical roster names identify these prototype heroes, not army slot numbers.
		if (general.name == U"劉備") result.push_back(ChainTactic::Oath);
		if (general.name == U"関羽") result.push_back(ChainTactic::AzureDragon);
		if (general.name == U"張飛") result.push_back(ChainTactic::SerpentRoar);
		return result;
	}
	bool TacticChain::CanUse(const Campaign& world, int army, ChainTactic tactic)
	{
		const auto options = Loadout(world, army);
		return std::find(options.begin(), options.end(), tactic) != options.end();
	}
}
