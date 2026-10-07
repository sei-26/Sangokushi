#pragma once
#include <algorithm>
#include <string>

// Shared rules for people, preparation and formation. Values are game settings.
namespace officer
{
	enum class Role
	{
		Leader,
		Vanguard,
		Guardian,
		Archer,
		Strategist,
		Logistics,
		Civil,
		Diplomat
	};
	struct Bond
	{
		int a = 0, b = 0, value = 20;
	};
	inline Role RoleOf(const std::u32string& name)
	{
		if (name == U"劉備" || name == U"曹操" || name == U"孫権")
			return Role::Leader;
		if (name == U"趙雲" || name == U"曹仁" || name == U"満寵")
			return Role::Guardian;
		if (name == U"黄忠")
			return Role::Archer;
		if (name == U"諸葛亮" || name == U"周瑜" || name == U"呂蒙" || name == U"陸遜")
			return Role::Strategist;
		if (name == U"徐晃" || name == U"歩騭")
			return Role::Logistics;
		if (name == U"簡雍")
			return Role::Diplomat;
		if (name == U"糜竺" || name == U"任峻" || name == U"顧雍" || name == U"荀彧")
			return Role::Civil;
		return Role::Vanguard;
	}
	inline std::u32string RoleName(Role role)
	{
		const char32_t* names[]{U"旗頭", U"突破役", U"守護役", U"射撃役",
		                        U"軍師", U"補給役", U"内政役", U"交渉役"};
		return names[static_cast<int>(role)];
	}
	inline std::u32string RoleHelp(Role role)
	{
		const char32_t* descriptions[]{U"近くの仲間の士気・闘志を支える。", U"近くの仲間の攻撃を支える。",
		                               U"近くの仲間が受ける損害を減らす。", U"近くの仲間の攻撃を支える。",
		                               U"近くの仲間の攻撃を支える。",       U"近くの仲間の兵糧消費を抑える。",
		                               U"都市を育て、出陣を支える。",       U"交渉と共同任務で道を開く。"};
		return descriptions[static_cast<int>(role)];
	}
	inline int BondTier(int affinity)
	{
		return affinity >= 80 ? 3 : affinity >= 60 ? 2 : affinity >= 40 ? 1 : 0;
	}
	struct Link
	{
		int attack = 0, defense = 0, morale = 0, supply = 0;
	};
	inline Link Contribution(Role own, Role ally, int affinity)
	{
		Link effect;
		int tier = BondTier(affinity);
		if (!tier)
			return effect;
		int power = tier * 5;
		if (own == ally)
			power = (power + 1) / 2;
		if (ally == Role::Vanguard || ally == Role::Archer || ally == Role::Strategist)
			effect.attack = power;
		if (ally == Role::Guardian)
			effect.defense = power;
		if (ally == Role::Leader)
			effect.morale = own == ally ? tier : tier * 2;
		if (ally == Role::Logistics)
			effect.supply = power;
		return effect;
	}
	inline void Merge(Link& a, const Link& b)
	{
		a.attack = std::max(a.attack, b.attack);
		a.defense = std::max(a.defense, b.defense);
		a.morale = std::max(a.morale, b.morale);
		a.supply = std::max(a.supply, b.supply);
	}
	inline int StoryAttack(const Link& link)
	{
		return (link.attack + 9) / 10;
	}
	inline int StoryDefense(const Link& link)
	{
		return link.defense >= 10 ? 1 : 0;
	}
	inline int StartingMorale(int order)
	{
		return std::clamp(70 + std::clamp(order, 0, 100) / 3, 70, 100);
	}
	inline int SupplyPack(int soldiers, int logistics)
	{
		return soldiers / 2 * (80 + std::clamp(logistics, 0, 100) / 2) / 100;
	}
	inline int PlanStrength(Role role, int plan)
	{
		if (plan == 0)
			return role == Role::Logistics || role == Role::Civil                               ? 3
			       : role == Role::Guardian || role == Role::Leader || role == Role::Strategist ? 2
			                                                                                    : 1;
		return role == Role::Vanguard || role == Role::Archer                               ? 3
		       : role == Role::Guardian || role == Role::Leader || role == Role::Strategist ? 2
		                                                                                    : 1;
	}
} // namespace officer
