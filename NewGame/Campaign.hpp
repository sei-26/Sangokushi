#pragma once
#include <algorithm>
#include <array>
#include <cmath>
#include <limits>
#include <queue>
#include <string>
#include <vector>
#include "OfficerTraits.hpp"

namespace frontline
{
constexpr int Width = 36, Height = 22, TileCount = Width * Height;
enum class Terrain { Plain, Forest, Mountain, River, Bridge, Sea };
enum class Arm { Spear, Bow, Siege, Cavalry };
enum class Duty { Farming, Commerce, Order, Logistics };
struct Tile { Terrain terrain = Terrain::Plain; int owner = -1; };
struct City
{
	std::u32string name;
	int tile = 0, owner = 0, troops = 10000, food = 24000, gold = 3000;
	int farming = 40, commerce = 40, order = 65, logistics = 40;
	int worker = -1, work = 0, workLeft = 0;
	int helper = -1;
};
struct General
{
	std::u32string name;
	int faction = 0, home = 0, leadership = 75, readyDay = 0;
	int politics = 60;
	Duty specialty = Duty::Order;
	int intelligence = 60, charm = 60;
	Trait trait = Trait::Administrator;
	Tactic tactic = Tactic::Charge;
};
struct Army
{
	int faction = 0, general = 0, tile = 0, target = 0, troops = 0;
	int food = 1800, morale = 100, movement = 0;
	Arm arm = Arm::Spear;
	bool supplied = true, retreat = false;
	std::vector<int> path;
	int tacticLeft = 0, tacticReadyDay = 0;
	bool tacticQueued = false;
};
struct Bond { int a = 0, b = 0, value = 20; };
enum class MissionKind { Diplomacy, Sabotage };
struct Mission { int general = 0, helper = -1, city = 0, target = 0, faction = 0, targetFaction = 0, left = 20; MissionKind kind = MissionKind::Diplomacy; };
struct Chronicle { int day = 0; std::u32string text; };

// Engine-independent simulation: one world, simultaneous daily combat,
// continuous ownership, and supply corridors that enemies can cut.
class Campaign
{
public:
	std::array<Tile, TileCount> tiles{};
	std::vector<City> cities;
	std::vector<General> generals;
	std::vector<Army> armies;
	std::vector<std::u32string> log;
	std::vector<Bond> bonds;
	std::vector<Mission> missions;
	std::vector<Chronicle> chronicle;
	std::array<std::array<int,3>,3> truceUntil{}, regard{};
	unsigned randomState = 89173;
	int player = 0, day = 0, commands = 3, result = 0;
	unsigned revision = 0;
	static constexpr int MaxArmies = 6;
	static bool Valid(int p) { return p >= 0 && p < TileCount; }
	static int At(int x, int y) { return x >= 0 && x < Width && y >= 0 && y < Height ? y * Width + x : -1; }
	static int Distance(int a, int b) { return std::abs(a % Width - b % Width) + std::abs(a / Width - b / Width); }
	static std::u32string FactionName(int f) { return f == 0 ? U"劉備" : f == 1 ? U"曹操" : U"孫権"; }
	static std::u32string ArmName(Arm a) { return a == Arm::Spear ? U"槍兵" : a == Arm::Bow ? U"弓兵" : a == Arm::Siege ? U"攻城隊" : U"騎兵"; }
	static std::u32string DutyName(Duty d) { return d == Duty::Farming ? U"農政" : d == Duty::Commerce ? U"商業" : d == Duty::Order ? U"治安" : U"兵站"; }
	static std::vector<int> Neighbors(int p)
	{
		std::vector<int> out;
		for (int n : {At(p % Width - 1, p / Width), At(p % Width + 1, p / Width), At(p % Width, p / Width - 1), At(p % Width, p / Width + 1)})
			if (Valid(n)) out.push_back(n);
		return out;
	}
	void Reset(int faction)
	{
		player = std::clamp(faction, 0, 2); day = 0; commands = 3; result = 0; armies.clear(); log.clear(); generals.clear();
		bonds.clear(); missions.clear(); chronicle.clear(); truceUntil = {}; randomState = 89173;
		for (auto& row : regard) row.fill(30);
		for (int i = 0; i < TileCount; ++i)
		{
			const int x = i % Width, y = i / Width;
			auto& t = tiles[i]; t = Tile{};
			if (x >= 33 && y < 17) t.terrain = Terrain::Sea;
			else if (y == 11 || (x == 19 && y < 11)) t.terrain = Terrain::River;
			else if ((x == 10 && y > 3 && y < 18 && y != 8 && y != 14) || (y == 4 && x > 4 && x < 17 && x != 8)) t.terrain = Terrain::Mountain;
			else if ((x * 7 + y * 11) % 13 < 3) t.terrain = Terrain::Forest;
		}
		for (int x : {5, 15, 26, 31}) tiles[At(x, 11)].terrain = Terrain::Bridge;
		for (int y : {3, 7}) tiles[At(19, y)].terrain = Terrain::Bridge;
		cities = {{U"成都", At(5,17),0}, {U"漢中",At(7,13),0}, {U"新野",At(16,13),0},
			{U"許昌",At(20,8),1}, {U"洛陽",At(15,6),1}, {U"鄴",At(24,3),1},
			{U"建業",At(29,15),2}, {U"柴桑",At(24,18),2}, {U"江陵",At(18,18),2}};
		for (int i = 0; i < TileCount; ++i)
		{
			if (tiles[i].terrain == Terrain::Sea) continue;
			int near = -1, dist = 5;
			for (int c = 0; c < static_cast<int>(cities.size()); ++c)
				if (Distance(i, cities[c].tile) < dist) { dist = Distance(i, cities[c].tile); near = c; }
			if (near >= 0) tiles[i].owner = cities[near].owner;
		}
		for (const auto& c : cities) { tiles[c.tile].terrain = Terrain::Plain; tiles[c.tile].owner = c.owner; }
		generals = {{U"劉備",0,0,82}, {U"趙雲",0,0,92}, {U"張飛",0,1,88}, {U"黄忠",0,1,85}, {U"関羽",0,2,94}, {U"諸葛亮",0,2,90},
			{U"曹操",1,3,96}, {U"曹仁",1,3,88}, {U"夏侯惇",1,4,86}, {U"徐晃",1,4,91}, {U"張遼",1,5,95}, {U"荀彧",1,5,80},
			{U"孫権",2,6,82}, {U"太史慈",2,6,89}, {U"周瑜",2,7,95}, {U"甘寧",2,7,90}, {U"呂蒙",2,8,91}, {U"陸遜",2,8,94}};
		const int politics[] = {85,72,38,52,70,98,94,66,64,61,65,97,89,59,91,43,82,93};
		const Duty specialties[] = {Duty::Order,Duty::Logistics,Duty::Order,Duty::Farming,Duty::Order,Duty::Logistics,
			Duty::Commerce,Duty::Order,Duty::Farming,Duty::Logistics,Duty::Order,Duty::Commerce,
			Duty::Commerce,Duty::Order,Duty::Logistics,Duty::Order,Duty::Farming,Duty::Logistics};
		for (int i = 0; i < static_cast<int>(generals.size()); ++i) { generals[i].politics = politics[i]; generals[i].specialty = specialties[i]; }
		generals.push_back({U"簡雍",0,1,45,0,84,Duty::Order});
		generals.push_back({U"糜竺",0,0,42,0,91,Duty::Commerce});
		generals.push_back({U"任峻",1,3,61,0,88,Duty::Farming});
		generals.push_back({U"満寵",1,4,81,0,86,Duty::Order});
		generals.push_back({U"顧雍",2,6,40,0,94,Duty::Commerce});
		generals.push_back({U"歩騭",2,8,58,0,87,Duty::Logistics});
		const int intelligence[] = {76,76,32,60,74,98,94,68,58,74,80,96,82,63,96,68,89,97,78,79,76,85,88,86};
		const int charm[] = {98,80,48,66,82,92,91,68,65,66,78,90,90,75,88,60,79,83,94,87,69,74,88,79};
		const Trait traits[] = {Trait::Benevolent,Trait::Guardian,Trait::Valiant,Trait::Valiant,Trait::Valiant,Trait::Strategist,
			Trait::Strategist,Trait::Guardian,Trait::Valiant,Trait::Quartermaster,Trait::Raider,Trait::Administrator,
			Trait::Benevolent,Trait::Valiant,Trait::Strategist,Trait::Raider,Trait::Strategist,Trait::Strategist,
			Trait::Negotiator,Trait::Merchant,Trait::Farmer,Trait::Guardian,Trait::Administrator,Trait::Quartermaster};
		const Tactic tactics[] = {Tactic::Rally,Tactic::Fortify,Tactic::Charge,Tactic::Volley,Tactic::Charge,Tactic::Fire,
			Tactic::Rally,Tactic::Fortify,Tactic::Charge,Tactic::Supply,Tactic::Charge,Tactic::Supply,
			Tactic::Rally,Tactic::Volley,Tactic::Fire,Tactic::Charge,Tactic::Fire,Tactic::Fire,
			Tactic::Rally,Tactic::Supply,Tactic::Supply,Tactic::Fortify,Tactic::Rally,Tactic::Supply};
		for (int i = 0; i < static_cast<int>(generals.size()); ++i) { generals[i].intelligence = intelligence[i]; generals[i].charm = charm[i]; generals[i].trait = traits[i]; generals[i].tactic = tactics[i]; }
		// Initial bonds and abilities are sandbox game settings, not historical assertions.
		ChangeBond(0,2,50); ChangeBond(0,4,50); ChangeBond(2,4,50); ChangeBond(4,5,25); ChangeBond(6,11,35); ChangeBond(12,14,30);
		Note(U"進路を描き、補給路を守り、複数部隊で城を包囲せよ。"); ++revision;
	}
	void Note(const std::u32string& text)
	{
		log.push_back(text); if (log.size() > 7) log.erase(log.begin());
		chronicle.push_back({day,text}); if (chronicle.size() > 120) chronicle.erase(chronicle.begin());
	}
	int Affinity(int a, int b) const
	{
		if (a < 0 || b < 0 || a >= static_cast<int>(generals.size()) || b >= static_cast<int>(generals.size()) || a == b) return 0;
		if (a > b) std::swap(a,b);
		for (const auto& bond : bonds) if (bond.a == a && bond.b == b) return bond.value;
		return generals[a].faction == generals[b].faction ? 20 : 0;
	}
	void ChangeBond(int a, int b, int amount)
	{
		if (a < 0 || b < 0 || a == b || a >= static_cast<int>(generals.size()) || b >= static_cast<int>(generals.size())) return;
		if (a > b) std::swap(a,b);
		for (auto& bond : bonds) if (bond.a == a && bond.b == b) { bond.value = std::clamp(bond.value + amount,0,100); return; }
		bonds.push_back({a,b,std::clamp(Affinity(a,b) + amount,0,100)});
	}
	bool Hostile(int a, int b) const { return a != b && day >= truceUntil[a][b]; }
	int Leader(int faction) const { for (int i = 0; i < static_cast<int>(generals.size()); ++i) if (generals[i].faction == faction) return i; return -1; }
	int CityAt(int tile) const { for (int i = 0; i < static_cast<int>(cities.size()); ++i) if (cities[i].tile == tile) return i; return -1; }
	bool Busy(int general) const
	{
		for (const auto& c : cities) if (c.worker == general || (c.worker >= 0 && c.helper == general)) return true;
		for (const auto& m : missions) if (m.general == general || m.helper == general) return true;
		for (const auto& a : armies) if (a.troops > 0 && a.general == general) return true; return false;
	}
	int WorkGain(int general, Duty duty, int helper = -1) const
	{
		const auto& g = generals[general]; int bonus = 0;
		if (g.trait == Trait::Administrator) bonus = 3;
		if ((g.trait == Trait::Farmer && duty == Duty::Farming) || (g.trait == Trait::Merchant && duty == Duty::Commerce) || (g.trait == Trait::Quartermaster && duty == Duty::Logistics)) bonus = 6;
		if (duty == Duty::Order) { if (g.trait == Trait::Benevolent) bonus = 5; if (g.trait == Trait::Negotiator) bonus = 3; }
		return 6 + g.politics / 10 + (g.specialty == duty ? 8 : 0) + bonus + (helper >= 0 ? generals[helper].politics / 20 + Affinity(general,helper) / 10 : 0);
	}
	bool Develop(int city, int general, Duty duty, bool ai = false, int helper = -1)
	{
		if (city < 0 || city >= static_cast<int>(cities.size()) || general < 0 || general >= static_cast<int>(generals.size()) || static_cast<int>(duty) < 0 || static_cast<int>(duty) > 3) return false;
		auto& c = cities[city]; const auto available = Available(city);
		if (c.worker >= 0 || c.gold < 500 || (!ai && (c.owner != player || commands <= 0)) || std::find(available.begin(),available.end(),general) == available.end()) return false;
		if (helper != -1 && (helper == general || std::find(available.begin(),available.end(),helper) == available.end())) return false;
		const int level = duty == Duty::Farming ? c.farming : duty == Duty::Commerce ? c.commerce : duty == Duty::Order ? c.order : c.logistics;
		if (level >= 100) return false;
		c.gold -= 500; c.worker = general; c.helper = helper; c.work = static_cast<int>(duty); c.workLeft = 30;
		if (!ai) --commands;
		Note(generals[general].name + U"が" + c.name + U"の" + DutyName(duty) + U"を担当（30日）。"); return true;
	}
	bool CancelWork(int city)
	{
		if (city < 0 || city >= static_cast<int>(cities.size()) || cities[city].owner != player || cities[city].worker < 0) return false;
		cities[city].worker = cities[city].helper = -1; cities[city].workLeft = 0; Note(U"内政を中止。担当武将を軍務に戻しました。"); return true;
	}
	int ArmyCount(int faction) const { int count = 0; for (const auto& a : armies) if (a.troops > 0 && a.faction == faction) ++count; return count; }
	std::vector<int> Available(int city) const
	{
		std::vector<int> out;
		if (city < 0 || city >= static_cast<int>(cities.size())) return out;
		for (int i = 0; i < static_cast<int>(generals.size()); ++i)
			if (generals[i].home == city && generals[i].faction == cities[city].owner && generals[i].readyDay <= day && !Busy(i)) out.push_back(i);
		return out;
	}
	int MissionChance(int general, int target, MissionKind kind, int helper = -1) const
	{
		const auto& g = generals[general]; int chance;
		if (kind == MissionKind::Diplomacy)
		{
			chance = 20 + g.charm / 2 + regard[g.faction][cities[target].owner] / 5 + Affinity(general,Leader(cities[target].owner)) / 10;
			if (g.trait == Trait::Negotiator) chance += 18; if (g.trait == Trait::Merchant) chance += 8;
		}
		else
		{
			int defense = 35;
			for (int i = 0; i < static_cast<int>(generals.size()); ++i)
				if (generals[i].home == target && generals[i].faction == cities[target].owner)
				{
					const bool away = std::any_of(armies.begin(),armies.end(),[&](const Army& a) { return a.troops > 0 && a.general == i; })
						|| std::any_of(missions.begin(),missions.end(),[&](const Mission& m) { return m.general == i || m.helper == i; });
					if (!away) defense = std::max(defense,generals[i].intelligence + (generals[i].trait == Trait::Guardian ? 15 : generals[i].trait == Trait::Strategist ? 10 : 0));
				}
			chance = 35 + g.intelligence / 2 - defense / 3;
			if (g.trait == Trait::Strategist) chance += 15; if (g.trait == Trait::Raider) chance += 10;
		}
		if (helper >= 0) chance += (kind == MissionKind::Diplomacy ? generals[helper].charm : generals[helper].intelligence) / 20 + Affinity(general,helper) / 10;
		return std::clamp(chance,10,90);
	}
	bool SendMission(int city, int general, int target, MissionKind kind, int helper = -1, bool ai = false)
	{
		if (city < 0 || city >= static_cast<int>(cities.size()) || target < 0 || target >= static_cast<int>(cities.size()) || static_cast<int>(kind) < 0 || static_cast<int>(kind) > 1) return false;
		const auto staff = Available(city); auto& c = cities[city];
		if ((!ai && (c.owner != player || commands <= 0)) || c.gold < 300 || c.owner == cities[target].owner
			|| std::find(staff.begin(),staff.end(),general) == staff.end()
			|| (helper != -1 && (helper == general || std::find(staff.begin(),staff.end(),helper) == staff.end()))
			|| (kind == MissionKind::Sabotage && !Hostile(c.owner,cities[target].owner))) return false;
		c.gold -= 300; if (!ai) --commands;
		missions.push_back({general,helper,city,target,c.owner,cities[target].owner,kind == MissionKind::Diplomacy ? 20 : 30,kind});
		Note(generals[general].name + U"が" + cities[target].name + (kind == MissionKind::Diplomacy ? U"へ停戦交渉に出発。" : U"へ兵糧攪乱の工作に出発。")); return true;
	}
	unsigned Roll() { randomState ^= randomState << 13; randomState ^= randomState >> 17; randomState ^= randomState << 5; return randomState % 100; }
	void ResolveMissions()
	{
		for (auto it = missions.begin(); it != missions.end();)
		{
			const Mission m = *it;
			if (cities[m.city].owner != m.faction || cities[m.target].owner != m.targetFaction || (m.kind == MissionKind::Sabotage && !Hostile(m.faction,m.targetFaction)))
			{ Note(generals[m.general].name + U"の任務を中止。勢力・停戦状況が変化しました。"); it = missions.erase(it); continue; }
			if (--it->left > 0) { ++it; continue; }
			const bool success = static_cast<int>(Roll()) < MissionChance(m.general,m.target,m.kind,m.helper);
			if (success && m.kind == MissionKind::Diplomacy)
			{
				truceUntil[m.faction][m.targetFaction] = truceUntil[m.targetFaction][m.faction] = day + 60;
				regard[m.faction][m.targetFaction] = regard[m.targetFaction][m.faction] = std::min(100,regard[m.faction][m.targetFaction] + 15);
				ChangeBond(m.general,Leader(m.targetFaction),10);
				for (auto& a : armies)
				{
					const int target = CityAt(a.target);
					if (a.troops > 0 && target >= 0 && a.faction != cities[target].owner && !Hostile(a.faction,cities[target].owner)) { a.path.clear(); a.target = a.tile; }
				}
				Note(generals[m.general].name + U"の交渉が成立。" + FactionName(m.faction) + U"・" + FactionName(m.targetFaction) + U"が60日間の停戦。");
			}
			else if (success)
			{
				auto& target = cities[m.target]; target.food = std::max(0,target.food - std::min(3000,target.food / 5)); target.order = std::max(0,target.order - 15);
				regard[m.faction][m.targetFaction] = regard[m.targetFaction][m.faction] = std::max(0,regard[m.faction][m.targetFaction] - 15);
				ChangeBond(m.general,Leader(m.targetFaction),-5);
				Note(generals[m.general].name + U"の工作が成功。" + target.name + U"の兵糧と治安が低下。");
			}
			else
			{
				generals[m.general].readyDay = day + 10; if (m.helper >= 0) generals[m.helper].readyDay = day + 10;
				Note(generals[m.general].name + U"の任務が失敗。再任用まで10日必要です。");
			}
			ChangeBond(m.general,m.helper,success ? 6 : 2); it = missions.erase(it);
		}
	}
	int SupportBond(int army) const
	{
		const auto& a = armies[army]; int best = 0;
		for (int i = 0; i < static_cast<int>(armies.size()); ++i)
			if (i != army && armies[i].troops > 0 && armies[i].faction == a.faction && Distance(a.tile,armies[i].tile) <= 2) best = std::max(best,Affinity(a.general,armies[i].general));
		return best;
	}
	bool ActivateTactic(int index, bool ai = false)
	{
		if (index < 0 || index >= static_cast<int>(armies.size())) return false;
		auto& a = armies[index]; if (a.troops <= 0 || (!ai && a.faction != player) || a.tacticQueued || a.tacticReadyDay > day || a.morale < 30) return false;
		const auto t = generals[a.general].tactic;
		if (t == Tactic::Volley && a.arm != Arm::Bow) return false;
		if (t == Tactic::Fire)
		{
			const int city = CityAt(a.target);
			bool target = city >= 0 && Hostile(a.faction,cities[city].owner) && Distance(a.tile,a.target) <= 1;
			for (const auto& b : armies) if (b.troops > 0 && Hostile(a.faction,b.faction) && Distance(a.tile,b.tile) <= 2) target = true;
			if (!target) return false;
		}
		if (t == Tactic::Supply) { const int source = Supply(a.faction)[a.tile]; if (source < 0 || cities[source].food < 300) return false; }
		a.tacticQueued = true; return true;
	}
	int Cost(int tile, Arm arm) const
	{
		if (!Valid(tile) || tiles[tile].terrain == Terrain::Sea) return 100000;
		switch (tiles[tile].terrain)
		{
		case Terrain::Mountain: return arm == Arm::Siege ? 9 : 6;
		case Terrain::Forest: return arm == Arm::Cavalry || arm == Arm::Siege ? 6 : 4;
		case Terrain::River: return 9;
		default: return 3;
		}
	}
	std::vector<int> Route(int from, int to, Arm arm, int faction = -1) const
	{
		if (!Valid(from) || !Valid(to) || Cost(to, arm) >= 100000) return {};
		std::array<int, TileCount> dist, parent; dist.fill(1000000); parent.fill(-1);
		using Entry = std::pair<int,int>;
		std::priority_queue<Entry, std::vector<Entry>, std::greater<Entry>> q;
		dist[from] = 0; q.push({0,from});
		while (!q.empty())
		{
			const auto [distance, p] = q.top(); q.pop();
			if (distance != dist[p]) continue;
			if (p == to) break;
			for (int n : Neighbors(p))
			{
				const int city = CityAt(n);
				if (faction >= 0 && city >= 0 && cities[city].owner != faction && n != to) continue;
				const int next = distance + Cost(n, arm);
				if (next < dist[n] && Cost(n, arm) < 100000) { dist[n] = next; parent[n] = p; q.push({next,n}); }
			}
		}
		if (dist[to] == 1000000) return {};
		std::vector<int> path;
		for (int p = to; p != from; p = parent[p]) path.push_back(p);
		std::reverse(path.begin(), path.end()); return path;
	}
	bool Order(int index, int target, bool retreat = false)
	{
		if (index < 0 || index >= static_cast<int>(armies.size()) || armies[index].troops <= 0 || !Valid(target)) return false;
		const int city = CityAt(target);
		if (city >= 0 && cities[city].owner != armies[index].faction && !Hostile(armies[index].faction,cities[city].owner)) return false;
		auto path = Route(armies[index].tile, target, armies[index].arm, armies[index].faction);
		if (path.empty() && armies[index].tile != target) return false;
		auto& a = armies[index]; a.target = target; a.path = std::move(path); a.retreat = retreat; return true;
	}
	int Deploy(int city, int general, int soldiers, Arm arm, bool ai = false)
	{
		if (city < 0 || city >= static_cast<int>(cities.size()) || general < 0 || general >= static_cast<int>(generals.size()) || soldiers < 1000 || soldiers > 6000) return -1;
		auto& c = cities[city]; const auto available = Available(city);
		if ((!ai && (c.owner != player || commands <= 0)) || std::find(available.begin(), available.end(), general) == available.end()
			|| c.troops < soldiers + 1000 || c.gold < soldiers / 10 || c.food < soldiers / 2 || ArmyCount(c.owner) >= MaxArmies) return -1;
		c.troops -= soldiers; c.gold -= soldiers / 10; c.food -= soldiers / 2;
		Army a; a.faction = c.owner; a.general = general; a.tile = c.tile; a.target = c.tile; a.troops = soldiers; a.arm = arm; a.food = soldiers / 2;
		armies.push_back(a); if (!ai) --commands;
		Note(generals[general].name + U"が出陣。進路を指示せよ。"); return static_cast<int>(armies.size()) - 1;
	}
	bool Recruit(int city, bool ai = false)
	{
		if (city < 0 || city >= static_cast<int>(cities.size())) return false;
		auto& c = cities[city];
		if ((!ai && (c.owner != player || commands <= 0)) || c.gold < 300 || c.food < 1000 || c.troops > 16000 || c.order < 35) return false;
		c.troops += 2000; c.gold -= 300; c.food -= 1000; c.order = std::max(0,c.order - 10); if (!ai) --commands; return true;
	}
	int NearestCity(int tile, int faction, bool enemy) const
	{
		int best = -1, distance = 100000;
		for (int i = 0; i < static_cast<int>(cities.size()); ++i)
			if ((enemy ? Hostile(cities[i].owner,faction) : cities[i].owner == faction) && Distance(tile,cities[i].tile) < distance)
			{ distance = Distance(tile,cities[i].tile); best = i; }
		return best;
	}
	void BeginTurn()
	{
		if (result != 0) return;
		for (int f = 0; f < 3; ++f)
		{
			if (f == player) continue;
			for (int c = 0; c < static_cast<int>(cities.size()); ++c)
			{
				if (cities[c].owner == f && cities[c].worker < 0)
				{
					const auto staff = Available(c);
					const auto duty = static_cast<Duty>((day / 30 + c) % 4);
					if (!staff.empty()) { const int g = *std::max_element(staff.begin(),staff.end(),[&](int a,int b) { return WorkGain(a,duty) * 10 - generals[a].leadership / 3 < WorkGain(b,duty) * 10 - generals[b].leadership / 3; }); Develop(c,g,duty,true); }
				}
				if (cities[c].owner == f && day >= 60 && day % 60 == 0)
				{
					const auto staff = Available(c); const int enemy = NearestCity(cities[c].tile,f,true);
					int agent = -1;
					for (int g : staff) if ((generals[g].trait == Trait::Strategist || generals[g].trait == Trait::Raider) && (agent < 0 || generals[g].intelligence > generals[agent].intelligence)) agent = g;
					if (agent >= 0 && enemy >= 0) SendMission(c,agent,enemy,MissionKind::Sabotage,-1,true);
				}
				if (day < 60 || day % 30 != 0) continue;
				if (cities[c].owner == f && cities[c].troops < 7000) Recruit(c,true);
				if (cities[c].owner != f || ArmyCount(f) >= 3 || cities[c].troops < 7000) continue;
				const auto available = Available(c); if (available.empty()) continue;
				const int enemy = NearestCity(cities[c].tile,f,true); if (enemy < 0) continue;
				const int a = Deploy(c,available.front(),4000,(day / 10 + c) % 2 ? Arm::Spear : Arm::Siege,true);
				if (a >= 0) Order(a,cities[enemy].tile);
			}
		}
	}
	std::array<int,TileCount> Supply(int faction) const
	{
		std::array<int,TileCount> source; source.fill(-1);
		std::array<bool,TileCount> blocked{};
		for (const auto& a : armies) if (a.troops > 0 && Hostile(a.faction,faction)) blocked[a.tile] = true;
		std::queue<int> q;
		for (int c = 0; c < static_cast<int>(cities.size()); ++c)
			if (cities[c].owner == faction && cities[c].food > 0 && !blocked[cities[c].tile]) { source[cities[c].tile] = c; q.push(cities[c].tile); }
		while (!q.empty())
		{
			const int p = q.front(); q.pop();
			for (int n : Neighbors(p))
				if (source[n] < 0 && !blocked[n] && tiles[n].owner == faction && Cost(n,Arm::Spear) < 100000)
				{ source[n] = source[p]; q.push(n); }
		}
		return source;
	}
	void Return(Army& a, int city)
	{
		cities[city].troops += a.troops; cities[city].food += a.food;
		generals[a.general].home = city; Note(generals[a.general].name + U"が" + cities[city].name + U"に帰還。"); a.troops = 0;
	}
	int Fronts(int tile, int faction) const
	{
		int count = 0;
		for (int n : Neighbors(tile))
			if (std::any_of(armies.begin(),armies.end(),[&](const Army& a) { return a.troops > 0 && a.faction == faction && a.tile == n; })) ++count;
		return count;
	}
	void AdvanceDay()
	{
		if (result != 0) return;
		++day;
		ResolveMissions();
		for (int f = 0; f < 3; ++f) for (int other = f + 1; other < 3; ++other)
			if (truceUntil[f][other] == day) Note(FactionName(f) + U"・" + FactionName(other) + U"の停戦期間が終了。");
		for (auto& c : cities)
		{
			if (c.worker < 0) continue;
			if (generals[c.worker].faction != c.owner) { c.worker = c.helper = -1; c.workLeft = 0; continue; }
			if (--c.workLeft == 0)
			{
				int& level = c.work == 0 ? c.farming : c.work == 1 ? c.commerce : c.work == 2 ? c.order : c.logistics;
				level = std::min(100,level + WorkGain(c.worker,static_cast<Duty>(c.work),c.helper));
				if (c.helper >= 0) { ChangeBond(c.worker,c.helper,8); Note(generals[c.worker].name + U"と" + generals[c.helper].name + U"が共同内政を完了。親密度 +8。"); }
				Note(c.name + U"の" + DutyName(static_cast<Duty>(c.work)) + U"が発展。担当：" + generals[c.worker].name); c.worker = c.helper = -1;
			}
		}
		std::array<std::array<int,TileCount>,3> supply;
		for (int f = 0; f < 3; ++f) supply[f] = Supply(f);
		for (auto& a : armies)
		{
			if (a.troops <= 0) continue;
			const int source = supply[a.faction][a.tile];
			const int efficiency = source >= 0 ? cities[source].logistics / 5 : 0;
			const int need = std::max(20,a.troops / (40 + efficiency + (generals[a.general].specialty == Duty::Logistics ? 10 : 0) + (generals[a.general].trait == Trait::Quartermaster ? 10 : 0)));
			a.supplied = source >= 0 && cities[source].food >= need;
			if (a.supplied) { cities[source].food -= need; a.morale = std::min(100,a.morale + 2); }
			else if (a.food >= need) { a.food -= need; a.morale = std::max(0,a.morale - 1); }
			else { a.food = 0; a.morale = std::max(0,a.morale - 8); a.troops = std::max(0,a.troops - std::max(20,a.troops / 30)); }
			if (a.troops <= 0) { a.tacticQueued = false; generals[a.general].readyDay = day + 20; Note(generals[a.general].name + U"隊が兵糧不足で崩壊。"); continue; }
			if (a.morale < 25 && !a.retreat)
			{
				const int home = NearestCity(a.tile,a.faction,false);
				if (home >= 0) { const int index = static_cast<int>(&a - armies.data()); Order(index,cities[home].tile,true); Note(generals[a.general].name + U"隊が撤退を開始。"); }
			}
		}
		// Calculate all casualties from the same snapshot, then apply together.
		std::vector<int> losses(armies.size()), cityLoss(cities.size());
		std::vector<bool> fighting(armies.size());
		std::vector<int> moraleGain(armies.size());
		for (int i = 0; i < static_cast<int>(armies.size()); ++i)
		{
			auto& a = armies[i]; if (a.troops <= 0) continue;
			if (a.tacticLeft > 0) --a.tacticLeft;
			if (a.faction != player && !a.tacticQueued)
			{
				bool nearby = false; for (const auto& b : armies) if (b.troops > 0 && Hostile(a.faction,b.faction) && Distance(a.tile,b.tile) <= 2) nearby = true;
				const int c = CityAt(a.target); if (c >= 0 && Hostile(a.faction,cities[c].owner) && Distance(a.tile,a.target) <= 1) nearby = true;
				if (nearby) ActivateTactic(i,true);
			}
			if (!a.tacticQueued) continue;
			a.tacticQueued = false;
			const auto& g = generals[a.general]; bool fired = true;
			if (g.tactic == Tactic::Rally)
			{
				for (int j = 0; j < static_cast<int>(armies.size()); ++j) if (armies[j].troops > 0 && armies[j].faction == a.faction && Distance(a.tile,armies[j].tile) <= 2) moraleGain[j] += 20;
			}
			else if (g.tactic == Tactic::Fire)
			{
				int victim = -1, nearest = 3;
				for (int j = 0; j < static_cast<int>(armies.size()); ++j) if (armies[j].troops > 0 && Hostile(a.faction,armies[j].faction) && Distance(a.tile,armies[j].tile) < nearest) { nearest = Distance(a.tile,armies[j].tile); victim = j; }
				const int c = CityAt(a.target); const int damage = 120 + g.intelligence * 2;
				if (victim >= 0) losses[victim] += damage;
				else if (c >= 0 && Hostile(a.faction,cities[c].owner) && Distance(a.tile,a.target) <= 1) cityLoss[c] += damage;
				else fired = false;
			}
			else if (g.tactic == Tactic::Supply)
			{
				const int c = supply[a.faction][a.tile];
				if (c >= 0 && cities[c].food >= 300) { cities[c].food -= 300; a.food += 300; moraleGain[i] += 30; } else fired = false;
			}
			else a.tacticLeft = 5;
			if (fired) { a.morale = std::max(0,a.morale - 15); a.tacticReadyDay = day + 30; Note(g.name + U"が戦法「" + TacticName(g.tactic) + U"」を発動！"); }
			else Note(g.name + U"の戦法は対象・補給の変化で発動できませんでした。");
		}
		for (int i = 0; i < static_cast<int>(armies.size()); ++i) armies[i].morale = std::min(100,armies[i].morale + moraleGain[i] + (SupportBond(i) >= 60 ? 1 : 0));
		for (int i = 0; i < static_cast<int>(armies.size()); ++i)
		{
			const auto& a = armies[i]; if (a.troops <= 0) continue;
			const auto& officer = generals[a.general];
			const int range = a.arm == Arm::Bow ? (a.tacticLeft > 0 && officer.tactic == Tactic::Volley ? 3 : 2) : 1;
			int target = -1, distance = 100000;
			for (int j = 0; j < static_cast<int>(armies.size()); ++j)
				if (armies[j].troops > 0 && Hostile(armies[j].faction,a.faction) && Distance(a.tile,armies[j].tile) <= range && Distance(a.tile,armies[j].tile) < distance)
				{ target = j; distance = Distance(a.tile,armies[j].tile); }
			const double morale = 0.3 + a.morale / 140.0;
			const int bond = SupportBond(i);
			const double coordination = bond >= 40 ? 1 + (bond - 40 + (officer.trait == Trait::Benevolent ? 10 : 0)) / 200.0 : 1.0;
			const double tactic = a.tacticLeft > 0 ? (officer.tactic == Tactic::Charge ? 1.35 : officer.tactic == Tactic::Volley ? 1.2 : 1.0) : 1.0;
			const double power = (a.troops / 22.0 + officer.leadership * 1.3) * morale * 0.55 * coordination * tactic;
			if (target >= 0)
			{
				const auto& b = armies[target]; const int flank = Fronts(b.tile,a.faction);
				double defense = tiles[b.tile].terrain == Terrain::Mountain ? 1.5 : tiles[b.tile].terrain == Terrain::Forest ? 1.25 : 1.0;
				if (tiles[b.tile].terrain == Terrain::River) defense = 0.8;
				double multiplier = a.arm == Arm::Siege ? 0.5 : 1.0;
				if (officer.trait == Trait::Valiant) multiplier *= 1.12;
				if (officer.trait == Trait::Raider && tiles[a.tile].terrain == Terrain::Forest) multiplier *= 1.18;
				if (a.arm == Arm::Cavalry && tiles[a.tile].terrain == Terrain::Plain) multiplier = 1.2;
				losses[target] += std::max(1,static_cast<int>(power * multiplier * (1.0 + std::min(3,std::max(0,flank-1)) * 0.15) / defense));
				fighting[i] = true; if (distance == 1) fighting[target] = true;
			}
			else if (!a.retreat)
			{
				const int c = CityAt(a.target);
				if (c >= 0 && Hostile(cities[c].owner,a.faction) && Distance(a.tile,cities[c].tile) <= 1)
				{
					const int siege = Fronts(cities[c].tile,a.faction);
					cityLoss[c] += std::max(1,static_cast<int>(power * (a.arm == Arm::Siege ? 2.0 : 0.6) * (1 + std::min(3,std::max(0,siege-1)) * 0.2)));
					losses[i] += std::min(cities[c].troops / 100 + 15,70); fighting[i] = true;
				}
			}
		}
		if (day % 10 == 0)
			for (int i = 0; i < static_cast<int>(armies.size()); ++i) for (int j = i + 1; j < static_cast<int>(armies.size()); ++j)
				if (armies[i].troops > 0 && armies[j].troops > 0 && fighting[i] && fighting[j] && armies[i].faction == armies[j].faction && Distance(armies[i].tile,armies[j].tile) <= 2) ChangeBond(armies[i].general,armies[j].general,2);
		for (int i = 0; i < static_cast<int>(armies.size()); ++i)
		{
			auto& a = armies[i]; if (a.troops <= 0) continue;
			const auto& officer = generals[a.general];
			if (officer.trait == Trait::Guardian) losses[i] = static_cast<int>(losses[i] * 0.88);
			if (a.tacticLeft > 0 && officer.tactic == Tactic::Fortify) losses[i] = static_cast<int>(losses[i] * 0.7);
			a.troops = std::max(0,a.troops - losses[i]);
			if (losses[i] > 0) a.morale = std::max(0,a.morale - 2);
			if (a.troops == 0) { a.tacticQueued = false; generals[a.general].readyDay = day + 20; Note(generals[a.general].name + U"隊が敗走。"); }
		}
		for (int c = 0; c < static_cast<int>(cities.size()); ++c)
		{
			cities[c].troops = std::max(0,cities[c].troops - cityLoss[c]);
			if (cityLoss[c] == 0 || cities[c].troops > 0) continue;
			int winner = -1;
			for (int i = 0; i < static_cast<int>(armies.size()); ++i)
					if (armies[i].troops > 0 && !armies[i].retreat && armies[i].target == cities[c].tile && Hostile(armies[i].faction,cities[c].owner) && Distance(armies[i].tile,cities[c].tile) <= 1
					&& (winner < 0 || armies[i].troops > armies[winner].troops)) winner = i;
			if (winner >= 0)
			{
				const int previousOwner = cities[c].owner;
				auto& a = armies[winner]; cities[c].owner = a.faction; tiles[cities[c].tile].owner = a.faction;
				cities[c].worker = cities[c].helper = -1; cities[c].workLeft = 0; cities[c].order = std::max(20,cities[c].order - 25);
				const int garrison = std::min(1000,a.troops / 3); cities[c].troops = garrison; a.troops -= garrison;
				Note(FactionName(a.faction) + U"軍が" + cities[c].name + U"を攻略！"); ++revision;
				missions.erase(std::remove_if(missions.begin(),missions.end(),[&](const Mission& m) { if (m.city != c && m.target != c) return false; Note(generals[m.general].name + U"の任務は都市の陥落により中止。"); return true; }),missions.end());
				const int refuge = NearestCity(cities[c].tile,previousOwner,false);
				if (refuge >= 0) for (int g = 0; g < static_cast<int>(generals.size()); ++g)
					if (generals[g].home == c && generals[g].faction == previousOwner && !Busy(g)) { generals[g].home = refuge; generals[g].readyDay = day + 10; Note(generals[g].name + U"が" + cities[refuge].name + U"へ避難。"); }
			}
		}
		// Resolve movement after combat. Opposing forces cannot pass through one another.
		for (int i = 0; i < static_cast<int>(armies.size()); ++i)
		{
			auto& a = armies[i]; if (a.troops <= 0) continue;
			if (!a.retreat && fighting[i]) continue;
			a.movement = std::min(12,a.movement + (a.arm == Arm::Cavalry ? 4 : a.arm == Arm::Siege ? 2 : 3));
			if (a.path.empty()) continue;
			const int next = a.path.front(), city = CityAt(next);
			if (a.movement < Cost(next,a.arm) || (city >= 0 && cities[city].owner != a.faction)) continue;
			bool blocked = false;
			for (const auto& b : armies) if (b.troops > 0 && b.faction != a.faction && b.tile == next) blocked = true;
			if (blocked) continue;
			a.movement -= Cost(next,a.arm); a.tile = next; a.path.erase(a.path.begin());
			if (tiles[next].owner != a.faction && (tiles[next].owner < 0 || Hostile(tiles[next].owner,a.faction))) { tiles[next].owner = a.faction; ++revision; }
			if (city >= 0 && next == a.target) Return(a,city);
		}
		if (day % 10 == 0)
		{
			commands = 3;
		}
		if (day % 30 == 0)
		{
			for (auto& c : cities)
			{
				c.gold = std::min(100000000,c.gold + (100 + c.commerce * 15) * c.order / 100);
				c.food = std::min(100000000,c.food + (800 + c.farming * 80) * c.order / 100);
				c.order = std::max(0,c.order - (ArmyCount(c.owner) > 0 ? 3 : 1));
			}
			Note(U"月末収入。農政・商業・治安が収穫と税収を左右します。");
		}
		bool all = true, hasCity = false;
		for (const auto& c : cities) { all = all && c.owner == player; hasCity = hasCity || c.owner == player; }
		if (all) result = 1;
		else if (!hasCity && ArmyCount(player) == 0) result = 2;
	}
};
}
