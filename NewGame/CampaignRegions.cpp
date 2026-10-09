#include "Campaign.hpp"

namespace frontline
{
	void Campaign::BuildRegions()
	{
		regions.clear();
		tileRegion.fill(-1);
		if (!hexMap)
			return;
		for (int c = 0; c < static_cast<int>(cities.size()); ++c)
		{
			regions.push_back({cities[c].name, cities[c].tile, c, cities[c].owner});
			for (int side : {-1, 1})
			{
				int best = -1, score = 100000;
				for (int p = 0; p < TileCount; ++p)
				{
					if (tiles[p].terrain != Terrain::Plain && tiles[p].terrain != Terrain::Forest)
						continue;
					const int distance = MapDistance(p, cities[c].tile);
					if (distance < 4 || distance > 7 || CityAt(p) >= 0)
						continue;
					int nearest = 0;
					for (int j = 1; j < static_cast<int>(cities.size()); ++j)
						if (MapDistance(p, cities[j].tile) < MapDistance(p, cities[nearest].tile))
							nearest = j;
					if (nearest != c)
						continue;
					bool close = false;
					for (const auto& r : regions)
						if (MapDistance(p, r.tile) < 3)
							close = true;
					if (close)
						continue;
					const int value = std::abs(p % Width - (cities[c].tile % Width + side * 5)) * 3 +
					                  std::abs(p / Width - (cities[c].tile / Width + side * 2));
					if (value < score)
					{
						score = value;
						best = p;
					}
				}
				if (best >= 0 && !Route(cities[c].tile, best, Arm::Spear).empty())
					regions.push_back(
					    {cities[c].name + (side < 0 ? U"西府" : U"東府"), best, c, cities[c].owner});
			}
		}
		for (int p = 0; p < TileCount; ++p)
		{
			if (Cost(p, Arm::Spear) >= 100000)
				continue;
			int best = -1;
			for (int r = 0; r < static_cast<int>(regions.size()); ++r)
				if (best < 0 || MapDistance(p, regions[r].tile) < MapDistance(p, regions[best].tile))
					best = r;
			tileRegion[p] = best;
		}
		for (const auto& r : regions)
			tiles[r.tile].owner = r.owner;
	}

	int Campaign::RegionAt(int tile) const
	{
		return hexMap && Valid(tile) ? tileRegion[tile] : -1;
	}

	int Campaign::RegionCoverage(int region) const
	{
		if (!hexMap || region < 0 || region >= static_cast<int>(regions.size()))
			return 0;
		int owned = 0, total = 0;
		for (int p = 0; p < TileCount; ++p)
			if (tileRegion[p] == region)
			{
				++total;
				if (tiles[p].owner == regions[region].owner)
					++owned;
			}
		return total ? owned * 100 / total : 0;
	}

	void Campaign::AdvanceRegions()
	{
		if (!hexMap || regions.empty())
			return;
		for (int index = 0; index < static_cast<int>(regions.size()); ++index)
		{
			auto& r = regions[index];
			const int city = CityAt(r.tile);
			if (city >= 0)
			{
				r.owner = cities[city].owner;
				r.city = city;
			}
			else
				for (const auto& a : armies)
				{
					if (a.troops < 1000 || a.morale < 30 || a.retreat || a.arm == Arm::Transport ||
					    a.tile != r.tile)
						continue;
					if (r.owner == a.faction || (r.owner >= 0 && !Hostile(a.faction, r.owner)))
						continue;
					bool contested = false;
					for (const auto& b : armies)
						if (b.troops > 0 && b.faction != a.faction && b.tile == r.tile)
							contested = true;
					if (contested)
						continue;
					r.owner = a.faction;
					r.city = NearestCity(r.tile, r.owner, false);
					tiles[r.tile].owner = r.owner;
					++revision;
					Note(FactionName(r.owner) + U"が" + r.name + U"を占領。地域の支配権を獲得。");
					break;
				}
			if (r.city < 0 || r.city >= static_cast<int>(cities.size()) || cities[r.city].owner != r.owner)
				r.city = NearestCity(r.tile, r.owner, false);
			// A garrison expands only through its connected foothold, three tiles per five days.
			if (day % 5 != 0)
				continue;
			bool stationed = false;
			for (const auto& a : armies)
				if (a.tile == r.tile && a.faction == r.owner && a.troops >= 1000 && a.morale >= 30 &&
				    !a.retreat && a.arm != Arm::Transport && a.path.empty())
					stationed = true;
			if (!stationed)
				continue;
			std::array<bool, TileCount> connected{};
			std::queue<int> open;
			open.push(r.tile);
			connected[r.tile] = true;
			while (!open.empty())
			{
				const int p = open.front();
				open.pop();
				for (int n : MapNeighbors(p))
					if (tileRegion[n] == index && tiles[n].owner == r.owner && !connected[n])
					{
						connected[n] = true;
						open.push(n);
					}
			}
			std::vector<int> frontier;
			for (int p = 0; p < TileCount; ++p)
			{
				if (tileRegion[p] != index || tiles[p].owner == r.owner || CityAt(p) >= 0 ||
				    (tiles[p].owner >= 0 && !Hostile(r.owner, tiles[p].owner)))
					continue;
				bool occupied = false, adjacent = false;
				for (const auto& a : armies)
					if (a.troops > 0 && a.faction != r.owner && a.tile == p)
						occupied = true;
				for (int n : MapNeighbors(p))
					if (connected[n])
						adjacent = true;
				if (!occupied && adjacent)
					frontier.push_back(p);
			}
			std::stable_sort(frontier.begin(), frontier.end(),
			                 [&](int a, int b) { return MapDistance(a, r.tile) < MapDistance(b, r.tile); });
			for (int i = 0; i < std::min(3, static_cast<int>(frontier.size())); ++i)
			{
				tiles[frontier[i]].owner = r.owner;
				++revision;
			}
		}
	}

	bool Campaign::RegionConnected(int index) const
	{
		if (!hexMap || index < 0 || index >= static_cast<int>(regions.size()))
			return false;
		const auto& r = regions[index];
		if (r.owner < 0 || r.city < 0 || r.city >= static_cast<int>(cities.size()) ||
		    cities[r.city].owner != r.owner || cities[r.city].food <= 0)
			return false;
		const auto blocked = SupplyBlockade(r.owner);
		const int origin = cities[r.city].tile;
		if (blocked[origin] || tiles[r.tile].owner != r.owner)
			return false;
		std::array<bool, TileCount> seen{};
		std::queue<int> open;
		seen[origin] = true;
		open.push(origin);
		while (!open.empty())
		{
			const int p = open.front();
			open.pop();
			if (p == r.tile)
				return true;
			for (int n : MapNeighbors(p))
				if (!seen[n] && !blocked[n] && tiles[n].owner == r.owner && Cost(n, Arm::Spear) < 100000)
				{
					seen[n] = true;
					open.push(n);
				}
		}
		return false;
	}
	void Campaign::RegionIncome()
	{
		if (!hexMap || regions.empty())
			return;

		for (int i = 0; i < static_cast<int>(regions.size()); ++i)
		{
			const auto& r = regions[i];
			if (r.owner < 0 || r.city < 0 || CityAt(r.tile) >= 0 || cities[r.city].owner != r.owner ||
			    !RegionConnected(i))
				continue;
			auto& c = cities[r.city];
			const int control = RegionCoverage(i);
			c.gold = std::min(100000000, c.gold + (80 + c.commerce * 4) * control * c.order / 10000);
			c.food = std::min(100000000, c.food + (400 + c.farming * 15) * control * c.order / 10000);
		}
	}
} // namespace frontline
