#include "HeroStory.hpp"

namespace hero
{
	int Story::Affinity(int a, int b) const
	{
		if (a < 0 || a >= 6 || b < 0 || b >= 6 || a == b)
			return 0;
		if (a == 0 || b == 0)
			return bonds[a == 0 ? b : a];
		if (a > b)
			std::swap(a, b);
		for (const auto& r : relationships)
			if (r.a == a && r.b == b)
				return r.value;
		return 20;
	}

	void Story::ChangeBond(int a, int b, int amount)
	{
		if (a < 0 || a >= 6 || b < 0 || b >= 6 || a == b)
			return;
		if (a == 0 || b == 0)
		{
			auto& value = bonds[a == 0 ? b : a];
			value = std::clamp(value + amount, 0, 100);
			return;
		}
		if (a > b)
			std::swap(a, b);
		for (auto& r : relationships)
			if (r.a == a && r.b == b)
			{
				r.value = std::clamp(r.value + amount, 0, 100);
				return;
			}
		relationships.push_back({a, b, std::clamp(20 + amount, 0, 100)});
	}

	bool Story::AssignPlanner(int who)
	{
		if (phase != 1 || who < 0 || who >= Unlocked())
			return false;
		planner = who;
		return true;
	}

	int Story::PlanPower(int plan) const
	{
		return planner < 0 ? 0 : officer::PlanStrength(officer::RoleOf(Name(planner)), plan);
	}

	officer::Link Story::Formation(int index) const
	{
		officer::Link effect;
		if (index < 0 || index >= static_cast<int>(units.size()) || units[index].hero < 0 ||
		    units[index].hp <= 0)
			return effect;
		const auto& a = units[index];
		for (const auto& b : units)
			if (b.hero >= 0 && b.hero != a.hero && b.hp > 0 && Distance(a, b) <= 2)
				officer::Merge(effect, officer::Contribution(officer::RoleOf(Name(a.hero)),
				                                             officer::RoleOf(Name(b.hero)),
				                                             Affinity(a.hero, b.hero)));
		return effect;
	}
} // namespace hero
