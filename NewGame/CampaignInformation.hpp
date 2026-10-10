#pragma once
#include "Campaign.hpp"
#include <cstdint>
namespace information
{
	enum class Tab
	{
		Cities,
		Officers,
		Factions,
		Armies,
		Chronicle
	};
	enum class Post
	{
		Governor,
		Ready,
		Rest,
		Army,
		Development,
		Mission,
		Transfer,
		Displaced
	};
	struct Posting
	{
		Post kind = Post::Ready;
		int index = -1, days = 0;
	};
	struct Totals
	{
		int cities = 0, officers = 0, armies = 0, regions = 0;
		std::int64_t troops = 0, gold = 0, food = 0;
	};
	inline Totals Faction(const frontline::Campaign& game, int faction)
	{
		Totals out;
		if (faction < 0 || faction > 2)
			return out;
		for (const auto& c : game.cities)
			if (c.owner == faction)
			{
				++out.cities;
				out.troops += c.troops;
				out.gold += c.gold;
				out.food += c.food;
			}
		for (const auto& g : game.generals)
			if (g.faction == faction)
				++out.officers;
		for (const auto& a : game.armies)
			if (a.faction == faction && a.troops > 0)
			{
				++out.armies;
				out.troops += a.troops;
				out.food += a.food;
				out.food += a.cargoFood;
			}
		for (const auto& r : game.regions)
			if (r.owner == faction && game.CityAt(r.tile) < 0)
				++out.regions;
		return out;
	}
	inline std::vector<int> CityMembers(const frontline::Campaign& game, int city)
	{
		std::vector<int> out;
		if (city < 0 || city >= static_cast<int>(game.cities.size()))
			return out;
		for (int i = 0; i < static_cast<int>(game.generals.size()); ++i)
			if (game.generals[i].home == city && game.generals[i].faction == game.cities[city].owner)
				out.push_back(i);
		return out;
	}
	inline Posting PostOf(const frontline::Campaign& game, int officer)
	{
		if (officer < 0 || officer >= static_cast<int>(game.generals.size()))
			return {Post::Displaced};
		for (int c = 0; c < static_cast<int>(game.cities.size()); ++c)
			if (game.Governor(c) == officer)return {Post::Governor, c};
		for (int i = 0; i < static_cast<int>(game.armies.size()); ++i)
			if (game.armies[i].troops > 0 && game.armies[i].general == officer)
				return {Post::Army, i};
		for (int i = 0; i < static_cast<int>(game.assignments.size()); ++i)
			if (game.assignments[i].general == officer)
				return {Post::Transfer, i, game.assignments[i].left};
		for (int i = 0; i < static_cast<int>(game.missions.size()); ++i)
			if (game.missions[i].general == officer || game.missions[i].helper == officer)
				return {Post::Mission, i, game.missions[i].left};
		for (int i = 0; i < static_cast<int>(game.cities.size()); ++i)
			if (game.cities[i].worker == officer ||
			    (game.cities[i].worker >= 0 && game.cities[i].helper == officer))
				return {Post::Development, i, game.cities[i].workLeft};
		const auto& g = game.generals[officer];
		if (g.readyDay > game.day)
			return {Post::Rest, -1, g.readyDay - game.day};
		if (g.home < 0 || g.home >= static_cast<int>(game.cities.size()) ||
		    game.cities[g.home].owner != g.faction)
			return {Post::Displaced};
		return {};
	}
	inline std::vector<int> Relations(const frontline::Campaign& game, int officer)
	{
		std::vector<int> out;
		if (officer < 0 || officer >= static_cast<int>(game.generals.size()))
			return out;
		for (int i = 0; i < static_cast<int>(game.generals.size()); ++i)
			if (i != officer)
				out.push_back(i);
		std::stable_sort(out.begin(), out.end(),
		                 [&](int a, int b) { return game.Affinity(officer, a) > game.Affinity(officer, b); });
		return out;
	}
	inline std::vector<int> Rows(const frontline::Campaign& game, Tab tab, int faction = -1, int sort = 0)
	{
		std::vector<int> out;
		if (faction < -1 || faction > 2)
			return out;
		const auto include = [&](int owner) { return faction < 0 || owner == faction; };
		if (tab == Tab::Cities)
		{
			for (int i = 0; i < static_cast<int>(game.cities.size()); ++i)
				if (include(game.cities[i].owner))
					out.push_back(i);
		}
		else if (tab == Tab::Officers)
		{
			for (int i = 0; i < static_cast<int>(game.generals.size()); ++i)
				if (include(game.generals[i].faction))
					out.push_back(i);
		}
		else if (tab == Tab::Factions)
		{
			for (int i = 0; i < 3; ++i)
				if (include(i))
					out.push_back(i);
		}
		else if (tab == Tab::Armies)
		{
			for (int i = 0; i < static_cast<int>(game.armies.size()); ++i)
				if (game.armies[i].troops > 0 && include(game.armies[i].faction))
					out.push_back(i);
		}
		else if (tab == Tab::Chronicle)
		{
			for (int i = static_cast<int>(game.chronicle.size()) - 1; i >= 0; --i)
				out.push_back(i);
			return out;
		}
		if (sort >= 1 && sort <= 3)
		{
			const auto value = [&](int i) -> std::int64_t {
				if (tab == Tab::Cities)
				{
					const auto& c = game.cities[i];
					return sort == 1 ? c.troops : sort == 2 ? c.gold : c.food;
				}
				if (tab == Tab::Officers)
				{
					const auto& g = game.generals[i];
					return sort == 1 ? g.leadership : sort == 2 ? g.intelligence : g.politics;
				}
				if (tab == Tab::Armies)
				{
					const auto& a = game.armies[i];
					return sort == 1 ? a.troops : sort == 2 ? a.morale : a.food;
				}
				const auto total = Faction(game, i);
				return sort == 1 ? total.troops : sort == 2 ? total.gold : total.food;
			};
			std::stable_sort(out.begin(), out.end(), [&](int a, int b) { return value(a) > value(b); });
		}
		return out;
	}
} // namespace information
