#include "Campaign.hpp"
namespace frontline
{
	cityidentity::Kind Campaign::CityKind(int city) const
	{
		return city < 0 || city >= static_cast<int>(cities.size()) || legacyLayout
		           ? cityidentity::Kind::Ordinary
		           : cityidentity::Of(cities[city].name);
	}
	int Campaign::Governor(int city) const
	{
		if (city < 0 || city >= static_cast<int>(cities.size()))
			return -1;
		const int g = cities[city].governor;
		if (g < 0 || g >= static_cast<int>(generals.size()) || generals[g].home != city ||
		    generals[g].faction != cities[city].owner || generals[g].readyDay > day)
			return -1;
		return g;
	}
	bool Campaign::AppointGovernor(int city, int general, bool ai)
	{
		if (result != 0 || city < 0 || city >= static_cast<int>(cities.size()) || general < -1 ||
		    general >= static_cast<int>(generals.size()))
			return false;
		auto& c = cities[city];
		if ((!ai && (c.owner != player || commands <= 0)) || c.governor == general)
			return false;
		if (general >= 0)
		{
			const auto free = Available(city);
			if (c.gold < 100 || std::find(free.begin(), free.end(), general) == free.end())
				return false;
			c.gold -= 100;
		}
		c.governor = general;
		if (!ai)
			--commands;
		Note(c.name + (general >= 0 ? U"の太守に" + generals[general].name + U"を任命。都市運営に専念する。"
		                            : U"の太守を解任。武将を通常任務へ戻す。"));
		return true;
	}
	int Campaign::CityIncome(int city, bool food, int candidate) const
	{
		if (city < 0 || city >= static_cast<int>(cities.size()))
			return 0;
		const auto& c = cities[city];
		int percent = 100;
		if (CityKind(city) == (food ? cityidentity::Kind::Granary : cityidentity::Kind::Market))
			percent += 40;
		const int g =
		    candidate == -2
		        ? Governor(city)
		        : (candidate >= 0 && candidate < static_cast<int>(generals.size()) ? candidate : -1);
		if (g >= 0)
		{
			const auto& officer = generals[g];
			percent += officer.politics / 5;
			if (officer.trait == Trait::Administrator ||
			    (food ? officer.trait == Trait::Farmer : officer.trait == Trait::Merchant))
				percent += 10;
		}
		return static_cast<int>(static_cast<long long>(food ? 800 + c.farming * 80 : 100 + c.commerce * 15) *
		                        c.order * percent / 10000);
	}
	int Campaign::CityLogistics(int city, int candidate) const
	{
		if (city < 0 || city >= static_cast<int>(cities.size()))
			return 0;
		int value = cities[city].logistics + (CityKind(city) == cityidentity::Kind::Supply ? 20 : 0);
		const int g =
		    candidate == -2
		        ? Governor(city)
		        : (candidate >= 0 && candidate < static_cast<int>(generals.size()) ? candidate : -1);
		if (g >= 0)
		{
			value += generals[g].politics / 5;
			if (generals[g].trait == Trait::Quartermaster || generals[g].specialty == Duty::Logistics)
				value += 10;
		}
		return std::clamp(value, 0, 150);
	}
	int Campaign::CityDamagePercent(int city, int candidate) const
	{
		int percent = CityKind(city) == cityidentity::Kind::Fortress ? 85 : 100;
		const int g =
		    candidate == -2
		        ? Governor(city)
		        : (candidate >= 0 && candidate < static_cast<int>(generals.size()) ? candidate : -1);
		if (g >= 0)
			percent -= generals[g].leadership / 10;
		return std::clamp(percent, 70, 100);
	}
	int Campaign::Recruitment(int city) const
	{
		return CityKind(city) == cityidentity::Kind::Military ? 2500 : 2000;
	}
	int Campaign::GovernorWorkBonus(int city, int general) const
	{
		const int g = Governor(city);
		return g >= 0 ? generals[g].politics / 25 + Affinity(g, general) / 20 : 0;
	}
	int Campaign::CityWorkGain(int city) const
	{
		if (city < 0 || city >= static_cast<int>(cities.size()) || cities[city].worker < 0)
			return 0;
		const auto& c = cities[city];
		int gain = WorkGain(c.worker, static_cast<Duty>(c.work), c.helper);
		gain += GovernorWorkBonus(city, c.worker);
		return gain;
	}
} // namespace frontline
