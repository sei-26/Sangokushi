#include "Campaign.hpp"
namespace frontline
{
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
