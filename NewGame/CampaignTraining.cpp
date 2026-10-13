#include "Campaign.hpp"
namespace frontline
{
	int Campaign::TrainingAt(int city) const
	{
		for (int i = 0; i < static_cast<int>(training.size()); ++i)
			if (training[i].city == city)
				return i;
		return -1;
	}
	bool Campaign::TrainTogether(int city, int first, int second, bool ai)
	{
		if (result != 0 || city < 0 || city >= static_cast<int>(cities.size()) || first < 0 || second < 0 ||
		    first == second || first >= static_cast<int>(generals.size()) ||
		    second >= static_cast<int>(generals.size()))
			return false;
		auto& c = cities[city];
		const auto available = Available(city);
		if (TrainingAt(city) >= 0 || c.gold < 300 || c.food < 500 ||
		    (!ai && (c.owner != player || commands <= 0)) || Affinity(first, second) >= 100 ||
		    std::find(available.begin(), available.end(), first) == available.end() ||
		    std::find(available.begin(), available.end(), second) == available.end())
			return false;
		c.gold -= 300;
		c.food -= 500;
		training.push_back({city, first, second, c.owner, 10});
		if (!ai)
			--commands;
		Note(c.name + U"で" + generals[first].name + U"と" + generals[second].name +
		     U"が共同訓練を開始。10日間、他の任務には任用できない。");
		return true;
	}
	bool Campaign::CancelTraining(int city)
	{
		if (city < 0 || city >= static_cast<int>(cities.size()) || cities[city].owner != player ||
		    result != 0)
			return false;
		const int i = TrainingAt(city);
		if (i < 0)
			return false;
		Note(cities[city].name + U"の共同訓練を中止。担当武将を通常任務へ戻す（返金・親密上昇なし）。");
		training.erase(training.begin() + i);
		return true;
	}
	void Campaign::CancelInvalidTraining()
	{
		training.erase(
		    std::remove_if(training.begin(), training.end(),
		                   [&](const Training& t) {
			                   const bool invalid = cities[t.city].owner != t.faction ||
			                                        generals[t.first].faction != t.faction ||
			                                        generals[t.second].faction != t.faction ||
			                                        generals[t.first].home != t.city ||
			                                        generals[t.second].home != t.city;
			                   if (invalid)
			                   {
				                   Note(cities[t.city].name +
				                        U"の共同訓練は都市・所属の変化で中断。親密度の成果は得られない。");
			                   }
			                   return invalid;
		                   }),
		    training.end());
	}
	void Campaign::AdvanceTraining()
	{
		CancelInvalidTraining();
		for (auto it = training.begin(); it != training.end();)
		{
			if (--it->left > 0)
			{
				++it;
				continue;
			}
			const int old = Affinity(it->first, it->second);
			ChangeBond(it->first, it->second, 12);
			Note(generals[it->first].name + U"と" + generals[it->second].name +
			     U"が共同訓練を完了。親密度 +12（上限100）。");
			// Keep the entry available until both names were read, then release both officers.
			if (officer::BondTier(old) < officer::BondTier(Affinity(it->first, it->second)))
				Note(U"共に鍛えた武将の連携段階が上がった。次の出陣で役割をつなごう。");
			it = training.erase(it);
		}
	}
} // namespace frontline
