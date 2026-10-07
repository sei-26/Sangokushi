#include "Campaign.hpp"

namespace frontline
{
	int Campaign::MapWidth() const
	{
		return legacyLayout ? 36 : Width;
	}

	int Campaign::MapHeight() const
	{
		return legacyLayout ? 22 : Height;
	}

	bool Campaign::Valid(int p)
	{
		return p >= 0 && p < TileCount;
	}

	int Campaign::At(int x, int y)
	{
		return x >= 0 && x < Width && y >= 0 && y < Height ? y * Width + x : -1;
	}

	int Campaign::Distance(int a, int b)
	{
		return std::abs(a % Width - b % Width) + std::abs(a / Width - b / Width);
	}

	std::u32string Campaign::FactionName(int f)
	{
		return f == 0 ? U"劉備" : f == 1 ? U"曹操" : U"孫権";
	}

	std::u32string Campaign::ArmName(Arm a)
	{
		return a == Arm::Spear ? U"槍兵" : a == Arm::Bow ? U"弓兵" : a == Arm::Siege ? U"攻城隊" : U"騎兵";
	}

	std::u32string Campaign::DutyName(Duty d)
	{
		return d == Duty::Farming    ? U"農政"
		       : d == Duty::Commerce ? U"商業"
		       : d == Duty::Order    ? U"治安"
		                             : U"兵站";
	}

	std::vector<int> Campaign::Neighbors(int p)
	{
		std::vector<int> out;
		for (int n : {At(p % Width - 1, p / Width), At(p % Width + 1, p / Width),
		              At(p % Width, p / Width - 1), At(p % Width, p / Width + 1)})
			if (Valid(n))
				out.push_back(n);
		return out;
	}

	void Campaign::Reset(int faction)
	{
		player = std::clamp(faction, 0, 2);
		day = 0;
		commands = 3;
		result = 0;
		armies.clear();
		log.clear();
		generals.clear();
		bonds.clear();
		missions.clear();
		chronicle.clear();
		truceUntil = {};
		randomState = 89173;
		for (auto& row : regard)
			row.fill(30);
		legacyLayout = false;
		const auto ground = world::Ground();
		for (int i = 0; i < TileCount; ++i)
			tiles[i] = {ground[i], -1};
		cities.clear();
		for (const auto& c : world::Sites())
			cities.push_back({c.name, At(c.x, c.y), c.faction});
		for (int i = 0; i < TileCount; ++i)
		{
			if (tiles[i].terrain == Terrain::Sea)
				continue;
			int near = -1, dist = 100000;
			for (int c = 0; c < static_cast<int>(cities.size()); ++c)
				if (Distance(i, cities[c].tile) < dist)
				{
					dist = Distance(i, cities[c].tile);
					near = c;
				}
			if (near >= 0)
				tiles[i].owner = cities[near].owner;
		}
		for (const auto& c : cities)
		{
			tiles[c.tile].terrain = Terrain::Plain;
			tiles[c.tile].owner = c.owner;
		}
		generals = {{U"劉備", 0, 0, 82},   {U"趙雲", 0, 0, 92},   {U"張飛", 0, 1, 88}, {U"黄忠", 0, 1, 85},
		            {U"関羽", 0, 2, 94},   {U"諸葛亮", 0, 2, 90}, {U"曹操", 1, 3, 96}, {U"曹仁", 1, 3, 88},
		            {U"夏侯惇", 1, 4, 86}, {U"徐晃", 1, 4, 91},   {U"張遼", 1, 5, 95}, {U"荀彧", 1, 5, 80},
		            {U"孫権", 2, 6, 82},   {U"太史慈", 2, 6, 89}, {U"周瑜", 2, 7, 95}, {U"甘寧", 2, 7, 90},
		            {U"呂蒙", 2, 8, 91},   {U"陸遜", 2, 8, 94}};
		const int politics[] = {85, 72, 38, 52, 70, 98, 94, 66, 64, 61, 65, 97, 89, 59, 91, 43, 82, 93};
		const Duty specialties[] = {Duty::Order,    Duty::Logistics, Duty::Order,     Duty::Farming,
		                            Duty::Order,    Duty::Logistics, Duty::Commerce,  Duty::Order,
		                            Duty::Farming,  Duty::Logistics, Duty::Order,     Duty::Commerce,
		                            Duty::Commerce, Duty::Order,     Duty::Logistics, Duty::Order,
		                            Duty::Farming,  Duty::Logistics};
		for (int i = 0; i < static_cast<int>(generals.size()); ++i)
		{
			generals[i].politics = politics[i];
			generals[i].specialty = specialties[i];
		}
		generals.push_back({U"簡雍", 0, 1, 45, 0, 84, Duty::Order});
		generals.push_back({U"糜竺", 0, 0, 42, 0, 91, Duty::Commerce});
		generals.push_back({U"任峻", 1, 3, 61, 0, 88, Duty::Farming});
		generals.push_back({U"満寵", 1, 4, 81, 0, 86, Duty::Order});
		generals.push_back({U"顧雍", 2, 6, 40, 0, 94, Duty::Commerce});
		generals.push_back({U"歩騭", 2, 8, 58, 0, 87, Duty::Logistics});
		const int intelligence[] = {76, 76, 32, 60, 74, 98, 94, 68, 58, 74, 80, 96,
		                            82, 63, 96, 68, 89, 97, 78, 79, 76, 85, 88, 86};
		const int charm[] = {98, 80, 48, 66, 82, 92, 91, 68, 65, 66, 78, 90,
		                     90, 75, 88, 60, 79, 83, 94, 87, 69, 74, 88, 79};
		const Trait traits[] = {
		    Trait::Benevolent, Trait::Guardian,      Trait::Valiant,       Trait::Valiant,
		    Trait::Valiant,    Trait::Strategist,    Trait::Strategist,    Trait::Guardian,
		    Trait::Valiant,    Trait::Quartermaster, Trait::Raider,        Trait::Administrator,
		    Trait::Benevolent, Trait::Valiant,       Trait::Strategist,    Trait::Raider,
		    Trait::Strategist, Trait::Strategist,    Trait::Negotiator,    Trait::Merchant,
		    Trait::Farmer,     Trait::Guardian,      Trait::Administrator, Trait::Quartermaster};
		const Tactic tactics[] = {
		    Tactic::Rally, Tactic::Fortify, Tactic::Charge, Tactic::Volley,  Tactic::Charge, Tactic::Fire,
		    Tactic::Rally, Tactic::Fortify, Tactic::Charge, Tactic::Supply,  Tactic::Charge, Tactic::Supply,
		    Tactic::Rally, Tactic::Volley,  Tactic::Fire,   Tactic::Charge,  Tactic::Fire,   Tactic::Fire,
		    Tactic::Rally, Tactic::Supply,  Tactic::Supply, Tactic::Fortify, Tactic::Rally,  Tactic::Supply};
		for (int i = 0; i < static_cast<int>(generals.size()); ++i)
		{
			generals[i].intelligence = intelligence[i];
			generals[i].charm = charm[i];
			generals[i].trait = traits[i];
			generals[i].tactic = tactics[i];
		}
		// Initial bonds and abilities are sandbox game settings, not historical assertions.
		ChangeBond(0, 2, 50);
		ChangeBond(0, 4, 50);
		ChangeBond(2, 4, 50);
		ChangeBond(4, 5, 25);
		ChangeBond(6, 11, 35);
		ChangeBond(12, 14, 30);
		Note(U"進路を描き、補給路を守り、複数部隊で城を包囲せよ。");
		++revision;
	}

	void Campaign::ResetLegacy(int faction)
	{
		Reset(faction);
		legacyLayout = true;
		for (auto& t : tiles)
			t = {Terrain::Sea, -1};
		for (int y = 0; y < 22; ++y)
			for (int x = 0; x < 36; ++x)
			{
				auto& t = tiles[At(x, y)];
				t = {Terrain::Plain, -1};
				if (x >= 33 && y < 17)
					t.terrain = Terrain::Sea;
				else if (y == 11 || (x == 19 && y < 11))
					t.terrain = Terrain::River;
				else if ((x == 10 && y > 3 && y < 18 && y != 8 && y != 14) ||
				         (y == 4 && x > 4 && x < 17 && x != 8))
					t.terrain = Terrain::Mountain;
				else if ((x * 7 + y * 11) % 13 < 3)
					t.terrain = Terrain::Forest;
			}
		for (int x : {5, 15, 26, 31})
			tiles[At(x, 11)].terrain = Terrain::Bridge;
		for (int y : {3, 7})
			tiles[At(19, y)].terrain = Terrain::Bridge;
		cities = {{U"成都", At(5, 17), 0},  {U"漢中", At(7, 13), 0},  {U"新野", At(16, 13), 0},
		          {U"許昌", At(20, 8), 1},  {U"洛陽", At(15, 6), 1},  {U"鄴", At(24, 3), 1},
		          {U"建業", At(29, 15), 2}, {U"柴桑", At(24, 18), 2}, {U"江陵", At(18, 18), 2}};
		for (int y = 0; y < 22; ++y)
			for (int x = 0; x < 36; ++x)
			{
				auto& t = tiles[At(x, y)];
				if (t.terrain == Terrain::Sea)
					continue;
				int near = -1, dist = 5;
				for (int c = 0; c < 9; ++c)
					if (Distance(At(x, y), cities[c].tile) < dist)
					{
						dist = Distance(At(x, y), cities[c].tile);
						near = c;
					}
				if (near >= 0)
					t.owner = cities[near].owner;
			}
		for (const auto& c : cities)
			tiles[c.tile] = {Terrain::Plain, c.owner};
		++revision;
	}

	void Campaign::Note(const std::u32string& text)
	{
		log.push_back(text);
		if (log.size() > 7)
			log.erase(log.begin());
		chronicle.push_back({day, text});
		if (chronicle.size() > 120)
			chronicle.erase(chronicle.begin());
	}

	int Campaign::Leader(int faction) const
	{
		for (int i = 0; i < static_cast<int>(generals.size()); ++i)
			if (generals[i].faction == faction)
				return i;
		return -1;
	}

	int Campaign::CityAt(int tile) const
	{
		for (int i = 0; i < static_cast<int>(cities.size()); ++i)
			if (cities[i].tile == tile)
				return i;
		return -1;
	}

	int Campaign::ArmyCount(int faction) const
	{
		int count = 0;
		for (const auto& a : armies)
			if (a.troops > 0 && a.faction == faction)
				++count;
		return count;
	}
} // namespace frontline
