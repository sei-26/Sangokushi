#include "Campaign.hpp"
namespace frontline
{
	// These percentages are also shown in the army panel, using the current terrain and arm.
	int Campaign::OfficerAttackPercent(int index, bool siege) const
	{
		if (index < 0 || index >= static_cast<int>(armies.size()))
			return 100;
		const auto& a = armies[index];
		if (a.troops <= 0 || a.arm == Arm::Transport)
			return 100;
		const auto& g = generals[a.general];
		const auto terrain = tiles[a.tile].terrain;
		const bool cover = terrain == Terrain::Forest || terrain == Terrain::Mountain;
		int value = 100;
		if (siege)
		{
			if (g.trait == Trait::SiegeExpert)
				value += 25;
		}
		else
		{
			if (g.trait == Trait::Valiant)
				value += 12;
			if (g.trait == Trait::Raider && terrain == Terrain::Forest)
				value += 18;
			if (g.trait == Trait::CavalryExpert && a.arm == Arm::Cavalry && terrain == Terrain::Plain)
				value += 20;
			if (g.trait == Trait::ArcherExpert && a.arm == Arm::Bow)
				value += 18;
			if (g.trait == Trait::TerrainExpert && cover)
				value += 15;
		}
		if (a.tacticLeft > 0)
		{
			if (g.tactic == Tactic::Charge)
				value += 35;
			if (g.tactic == Tactic::Volley && a.arm == Arm::Bow)
				value += 20;
			if (!siege && g.tactic == Tactic::MountedCharge && a.arm == Arm::Cavalry &&
			    terrain == Terrain::Plain)
				value += 60;
			if (!siege && g.tactic == Tactic::Ambush && cover)
				value += 45;
			if (siege && g.tactic == Tactic::SiegeStrike && a.arm == Arm::Siege)
				value += 60;
		}
		return value;
	}
	int Campaign::OfficerDamagePercent(int index) const
	{
		if (index < 0 || index >= static_cast<int>(armies.size()))
			return 100;
		const auto& a = armies[index];
		if (a.troops <= 0 || a.arm == Arm::Transport)
			return 100;
		const auto& g = generals[a.general];
		const auto terrain = tiles[a.tile].terrain;
		const bool cover = terrain == Terrain::Forest || terrain == Terrain::Mountain;
		int value = 100;
		if (g.trait == Trait::Guardian)
			value -= 12;
		if (g.trait == Trait::TerrainExpert && cover)
			value -= 15;
		if (a.tacticLeft > 0 && g.tactic == Tactic::Fortify)
			value -= 30;
		if (a.tacticLeft > 0 && g.tactic == Tactic::Ambush && cover)
			value -= 20;
		return value;
	}
	bool Campaign::SetStance(int index, battle::Stance stance, bool ai)
	{
		if (result != 0 || index < 0 || index >= static_cast<int>(armies.size()) ||
		    stance < battle::Stance::Balanced || stance > battle::Stance::Guard)
			return false;
		auto& a = armies[index];
		if (a.troops <= 0 || a.arm == Arm::Transport || a.stance == stance ||
		    (!ai && (a.faction != player || commands <= 0)))
			return false;
		a.stance = stance;
		if (!ai)
			--commands;
		Note(generals[a.general].name + U"隊が「" + battle::Name(stance) + U"」の構えに変更。");
		return true;
	}
	int Campaign::AttackRange(int index) const
	{
		if (index < 0 || index >= static_cast<int>(armies.size()))
			return 0;
		const auto& a = armies[index];
		if (a.troops <= 0 || a.arm == Arm::Transport)
			return 0;
		if (a.arm != Arm::Bow)
			return 1;
		return (a.tacticLeft > 0 && generals[a.general].tactic == Tactic::Volley ? 3 : 2) +
		       (tiles[a.tile].terrain == Terrain::Mountain ? 1 : 0);
	}
	bool Campaign::CanStrike(int index, int tile) const
	{
		if (!Valid(tile) || index < 0 || index >= static_cast<int>(armies.size()))
			return false;
		const auto& a = armies[index];
		if (MapDistance(a.tile, tile) > AttackRange(index) || AttackRange(index) == 0)
			return false;
		if (MapDistance(a.tile, tile) <= 1)
			return true;
		return ClearShot(a.tile, tile);
	}
} // namespace frontline
