#include "Campaign.hpp"

namespace frontline
{
	int Campaign::WorkGain(int general, Duty duty, int helper) const
	{
		const auto& g = generals[general];
		int bonus = 0;
		if (g.trait == Trait::Administrator)
			bonus = 3;
		if ((g.trait == Trait::Farmer && duty == Duty::Farming) ||
		    (g.trait == Trait::Merchant && duty == Duty::Commerce) ||
		    (g.trait == Trait::Quartermaster && duty == Duty::Logistics))
			bonus = 6;
		if (duty == Duty::Order)
		{
			if (g.trait == Trait::Benevolent)
				bonus = 5;
			if (g.trait == Trait::Negotiator)
				bonus = 3;
		}
		return 6 + g.politics / 10 + (g.specialty == duty ? 8 : 0) + bonus +
		       (helper >= 0 ? generals[helper].politics / 20 + Affinity(general, helper) / 10 : 0);
	}

	bool Campaign::Develop(int city, int general, Duty duty, bool ai, int helper)
	{
		if (city < 0 || city >= static_cast<int>(cities.size()) || general < 0 ||
		    general >= static_cast<int>(generals.size()) || static_cast<int>(duty) < 0 ||
		    static_cast<int>(duty) > 3)
			return false;
		auto& c = cities[city];
		const auto available = Available(city);
		if (c.worker >= 0 || c.gold < 500 || (!ai && (c.owner != player || commands <= 0)) ||
		    std::find(available.begin(), available.end(), general) == available.end())
			return false;
		if (helper != -1 &&
		    (helper == general || std::find(available.begin(), available.end(), helper) == available.end()))
			return false;
		const int level = duty == Duty::Farming    ? c.farming
		                  : duty == Duty::Commerce ? c.commerce
		                  : duty == Duty::Order    ? c.order
		                                           : c.logistics;
		if (level >= 100)
			return false;
		c.gold -= 500;
		c.worker = general;
		c.helper = helper;
		c.work = static_cast<int>(duty);
		c.workLeft = 30;
		if (!ai)
			--commands;
		Note(generals[general].name + U"が" + c.name + U"の" + DutyName(duty) + U"を担当（30日）。");
		return true;
	}

	bool Campaign::CancelWork(int city)
	{
		if (city < 0 || city >= static_cast<int>(cities.size()) || cities[city].owner != player ||
		    cities[city].worker < 0)
			return false;
		cities[city].worker = cities[city].helper = -1;
		cities[city].workLeft = 0;
		Note(U"内政を中止。担当武将を軍務に戻しました。");
		return true;
	}

	bool Campaign::Recruit(int city, bool ai)
	{
		if (city < 0 || city >= static_cast<int>(cities.size()))
			return false;
		auto& c = cities[city];
		if ((!ai && (c.owner != player || commands <= 0)) || c.gold < 300 || c.food < 1000 ||
		    c.troops > 16000 || c.order < 35)
			return false;
		c.troops += 2000;
		c.gold -= 300;
		c.food -= 1000;
		c.order = std::max(0, c.order - 10);
		if (!ai)
			--commands;
		return true;
	}
} // namespace frontline

namespace frontline
{
	void Campaign::AdvanceCityWork()
	{
		for (int f = 0; f < 3; ++f)
			for (int other = f + 1; other < 3; ++other)
				if (truceUntil[f][other] == day)
					Note(FactionName(f) + U"・" + FactionName(other) + U"の停戦期間が終了。");
		for (auto& c : cities)
		{
			if (c.worker < 0)
				continue;
			if (generals[c.worker].faction != c.owner)
			{
				c.worker = c.helper = -1;
				c.workLeft = 0;
				continue;
			}
			if (--c.workLeft == 0)
			{
				int& level = c.work == 0   ? c.farming
				             : c.work == 1 ? c.commerce
				             : c.work == 2 ? c.order
				                           : c.logistics;
				level = std::min(100, level + WorkGain(c.worker, static_cast<Duty>(c.work), c.helper));
				if (c.helper >= 0)
				{
					ChangeBond(c.worker, c.helper, 8);
					Note(generals[c.worker].name + U"と" + generals[c.helper].name +
					     U"が共同内政を完了。親密度 +8。");
				}
				Note(c.name + U"の" + DutyName(static_cast<Duty>(c.work)) + U"が発展。担当：" +
				     generals[c.worker].name);
				c.worker = c.helper = -1;
			}
		}
	}

	void Campaign::FinishDay()
	{
		if (day % 10 == 0)
		{
			commands = 3;
		}
		if (day % 30 == 0)
		{
			for (auto& c : cities)
			{
				c.gold = std::min(100000000, c.gold + (100 + c.commerce * 15) * c.order / 100);
				c.food = std::min(100000000, c.food + (800 + c.farming * 80) * c.order / 100);
				c.order = std::max(0, c.order - (ArmyCount(c.owner) > 0 ? 3 : 1));
			}
			Note(U"月末収入。農政・商業・治安が収穫と税収を左右します。");
		}
		bool all = true, hasCity = false;
		for (const auto& c : cities)
		{
			all = all && c.owner == player;
			hasCity = hasCity || c.owner == player;
		}
		if (all)
			result = 1;
		else if (!hasCity && ArmyCount(player) == 0)
			result = 2;
	}
} // namespace frontline
