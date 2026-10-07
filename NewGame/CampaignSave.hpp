#pragma once
#include <Siv3D.hpp>
#include "Campaign.hpp"

namespace frontline
{
inline s3d::JSON SaveJSON(const Campaign& game)
{
	s3d::JSON json;
	json[U"version"] = 3; json[U"player"] = game.player; json[U"day"] = game.day; json[U"commands"] = game.commands;
	s3d::Array<int> owners; for (const auto& tile : game.tiles) owners.push_back(tile.owner); json[U"owners"] = owners;
	s3d::Array<s3d::JSON> cities, generals, armies;
	for (const auto& c : game.cities)
	{
		s3d::JSON j; j[U"owner"] = c.owner; j[U"troops"] = c.troops; j[U"food"] = c.food; j[U"gold"] = c.gold; cities.push_back(j);
		auto& saved = cities.back(); saved[U"farming"] = c.farming; saved[U"commerce"] = c.commerce; saved[U"order"] = c.order; saved[U"logistics"] = c.logistics;
		saved[U"worker"] = c.worker; saved[U"work"] = c.work; saved[U"workLeft"] = c.workLeft;
		saved[U"helper"] = c.helper;
	}
	for (const auto& g : game.generals)
	{
		s3d::JSON j; j[U"home"] = g.home; j[U"readyDay"] = g.readyDay; generals.push_back(j);
	}
	for (const auto& a : game.armies)
	{
		s3d::JSON j;
		j[U"faction"] = a.faction; j[U"general"] = a.general; j[U"tile"] = a.tile; j[U"target"] = a.target; j[U"troops"] = a.troops;
		j[U"food"] = a.food; j[U"morale"] = a.morale; j[U"movement"] = a.movement; j[U"arm"] = static_cast<int>(a.arm);
		j[U"supplied"] = a.supplied; j[U"retreat"] = a.retreat; armies.push_back(j);
		auto& saved = armies.back(); saved[U"tacticLeft"] = a.tacticLeft; saved[U"tacticReadyDay"] = a.tacticReadyDay; saved[U"tacticQueued"] = a.tacticQueued;
	}
	json[U"cities"] = cities; json[U"generals"] = generals; json[U"armies"] = armies;
	s3d::Array<s3d::JSON> bonds, missions, chronicle;
	for (const auto& b : game.bonds) { s3d::JSON j; j[U"a"] = b.a; j[U"b"] = b.b; j[U"value"] = b.value; bonds.push_back(j); }
	for (const auto& m : game.missions)
	{
		s3d::JSON j; j[U"general"] = m.general; j[U"helper"] = m.helper; j[U"city"] = m.city; j[U"target"] = m.target;
		j[U"faction"] = m.faction; j[U"targetFaction"] = m.targetFaction; j[U"left"] = m.left; j[U"kind"] = static_cast<int>(m.kind); missions.push_back(j);
	}
	for (const auto& event : game.chronicle) { s3d::JSON j; j[U"day"] = event.day; j[U"text"] = s3d::String(event.text.c_str()); chronicle.push_back(j); }
	s3d::Array<int> truces, regard; for (int f = 0; f < 3; ++f) for (int other = 0; other < 3; ++other) { truces.push_back(game.truceUntil[f][other]); regard.push_back(game.regard[f][other]); }
	json[U"bonds"] = bonds; json[U"missions"] = missions; json[U"chronicle"] = chronicle; json[U"truces"] = truces; json[U"regard"] = regard; json[U"randomState"] = game.randomState;
	return json;
}

// Parse into a temporary game, so invalid saves never change the running campaign.
inline bool LoadJSON(const s3d::JSON& json, Campaign& game)
{
	try
	{
		const int version = json[U"version"].get<int>(); if (version < 1 || version > 3) return false;
		const int player = json[U"player"].get<int>(), day = json[U"day"].get<int>(), commands = json[U"commands"].get<int>();
		if (player < 0 || player > 2 || day < 0 || day > 100000 || commands < 0 || commands > 3) return false;
		Campaign loaded; loaded.Reset(player); loaded.day = day; loaded.commands = commands;
		int index = 0;
		for (const auto& value : json[U"owners"].arrayView())
		{
			if (index >= TileCount) return false;
			const int owner = value.get<int>(); if (owner < -1 || owner > 2) return false;
			if (loaded.tiles[index].terrain == Terrain::Sea && owner != -1) return false;
			loaded.tiles[index++].owner = owner;
		}
		if (index != TileCount) return false;
		index = 0;
		for (const auto& j : json[U"cities"].arrayView())
		{
			if (index >= static_cast<int>(loaded.cities.size())) return false;
			auto& c = loaded.cities[index++]; c.owner = j[U"owner"].get<int>(); c.troops = j[U"troops"].get<int>(); c.food = j[U"food"].get<int>(); c.gold = j[U"gold"].get<int>();
			if (c.owner < 0 || c.owner > 2 || c.troops < 0 || c.troops > 1000000 || c.food < 0 || c.food > 100000000 || c.gold < 0 || c.gold > 100000000 || loaded.tiles[c.tile].owner != c.owner) return false;
			if (version >= 2)
			{
				c.farming = j[U"farming"].get<int>(); c.commerce = j[U"commerce"].get<int>(); c.order = j[U"order"].get<int>(); c.logistics = j[U"logistics"].get<int>();
				c.worker = j[U"worker"].get<int>(); c.work = j[U"work"].get<int>(); c.workLeft = j[U"workLeft"].get<int>();
				if (c.farming < 0 || c.farming > 100 || c.commerce < 0 || c.commerce > 100 || c.order < 0 || c.order > 100 || c.logistics < 0 || c.logistics > 100
					|| c.worker < -1 || c.worker >= static_cast<int>(loaded.generals.size()) || c.work < 0 || c.work > 3 || c.workLeft < 0 || c.workLeft > 30 || ((c.worker == -1) != (c.workLeft == 0))) return false;
				if (version >= 3) { c.helper = j[U"helper"].get<int>(); if (c.helper < -1 || c.helper >= static_cast<int>(loaded.generals.size()) || (c.helper >= 0 && (c.worker < 0 || c.helper == c.worker))) return false; }
			}
		}
		if (index != static_cast<int>(loaded.cities.size())) return false;
		index = 0;
		for (const auto& j : json[U"generals"].arrayView())
		{
			if (index >= static_cast<int>(loaded.generals.size())) return false;
			auto& g = loaded.generals[index++]; g.home = j[U"home"].get<int>(); g.readyDay = j[U"readyDay"].get<int>();
			if (g.home < 0 || g.home >= static_cast<int>(loaded.cities.size()) || g.readyDay < 0 || g.readyDay > day + 100) return false;
		}
		if (index != (version == 1 ? 18 : static_cast<int>(loaded.generals.size()))) return false;
		std::vector<bool> assigned(loaded.generals.size());
		for (int c = 0; c < static_cast<int>(loaded.cities.size()); ++c)
		{
			const int g = loaded.cities[c].worker; if (g < 0) continue;
			if (assigned[g] || loaded.generals[g].faction != loaded.cities[c].owner || loaded.generals[g].home != c || loaded.generals[g].readyDay > day) return false;
			assigned[g] = true;
			const int h = loaded.cities[c].helper;
			if (h >= 0) { if (assigned[h] || loaded.generals[h].faction != loaded.cities[c].owner || loaded.generals[h].home != c || loaded.generals[h].readyDay > day) return false; assigned[h] = true; }
		}
		if (version >= 3)
		{
			loaded.randomState = json[U"randomState"].get<unsigned>(); if (loaded.randomState == 0) return false;
			index = 0; for (const auto& value : json[U"truces"].arrayView()) { if (index >= 9) return false; const int until = value.get<int>(); if (until < 0 || until > day + 60) return false; loaded.truceUntil[index / 3][index % 3] = until; ++index; } if (index != 9) return false;
			index = 0; for (const auto& value : json[U"regard"].arrayView()) { if (index >= 9) return false; const int regard = value.get<int>(); if (regard < 0 || regard > 100) return false; loaded.regard[index / 3][index % 3] = regard; ++index; } if (index != 9) return false;
			for (int f = 0; f < 3; ++f) { if (loaded.truceUntil[f][f] != 0) return false; for (int other = 0; other < 3; ++other) if (loaded.truceUntil[f][other] != loaded.truceUntil[other][f] || loaded.regard[f][other] != loaded.regard[other][f]) return false; }
			loaded.bonds.clear();
			for (const auto& j : json[U"bonds"].arrayView())
			{
				Bond b{j[U"a"].get<int>(),j[U"b"].get<int>(),j[U"value"].get<int>()};
				if (loaded.bonds.size() >= 276 || b.a < 0 || b.b <= b.a || b.b >= static_cast<int>(loaded.generals.size()) || b.value < 0 || b.value > 100) return false;
				for (const auto& existing : loaded.bonds) if (existing.a == b.a && existing.b == b.b) return false; loaded.bonds.push_back(b);
			}
			for (const auto& j : json[U"missions"].arrayView())
			{
				Mission m; m.general = j[U"general"].get<int>(); m.helper = j[U"helper"].get<int>(); m.city = j[U"city"].get<int>(); m.target = j[U"target"].get<int>();
				m.faction = j[U"faction"].get<int>(); m.targetFaction = j[U"targetFaction"].get<int>(); m.left = j[U"left"].get<int>(); const int kind = j[U"kind"].get<int>();
				if (loaded.missions.size() >= loaded.generals.size() || m.general < 0 || m.general >= static_cast<int>(loaded.generals.size()) || m.helper < -1 || m.helper >= static_cast<int>(loaded.generals.size()) || m.helper == m.general
					|| m.city < 0 || m.city >= static_cast<int>(loaded.cities.size()) || m.target < 0 || m.target >= static_cast<int>(loaded.cities.size()) || m.faction < 0 || m.faction > 2 || m.targetFaction < 0 || m.targetFaction > 2 || m.faction == m.targetFaction || kind < 0 || kind > 1 || m.left <= 0 || m.left > (kind == 0 ? 20 : 30)) return false;
				m.kind = static_cast<MissionKind>(kind);
				if (loaded.cities[m.city].owner != m.faction || loaded.cities[m.target].owner != m.targetFaction || (m.kind == MissionKind::Sabotage && !loaded.Hostile(m.faction,m.targetFaction))) return false;
				for (int g : {m.general,m.helper}) if (g >= 0) { if (assigned[g] || loaded.generals[g].home != m.city || loaded.generals[g].faction != m.faction || loaded.generals[g].readyDay > day) return false; assigned[g] = true; }
				loaded.missions.push_back(m);
			}
			loaded.chronicle.clear(); int previous = -1;
			for (const auto& j : json[U"chronicle"].arrayView()) { const int eventDay = j[U"day"].get<int>(); const auto eventText = j[U"text"].get<s3d::String>(); if (loaded.chronicle.size() >= 120 || eventDay < 0 || eventDay < previous || eventDay > day || eventText.isEmpty() || eventText.size() > 300) return false; loaded.chronicle.push_back({eventDay,eventText.toUTF32()}); previous = eventDay; }
		}
		for (const auto& j : json[U"armies"].arrayView())
		{
			if (loaded.armies.size() >= 10000) return false;
			Army a; a.faction = j[U"faction"].get<int>(); a.general = j[U"general"].get<int>(); a.tile = j[U"tile"].get<int>(); a.target = j[U"target"].get<int>(); a.troops = j[U"troops"].get<int>();
			a.food = j[U"food"].get<int>(); a.morale = j[U"morale"].get<int>(); a.movement = j[U"movement"].get<int>();
			const int arm = j[U"arm"].get<int>(); a.supplied = j[U"supplied"].get<bool>(); a.retreat = j[U"retreat"].get<bool>();
			if (a.faction < 0 || a.faction > 2 || a.general < 0 || a.general >= static_cast<int>(loaded.generals.size()) || !Campaign::Valid(a.tile) || !Campaign::Valid(a.target)
				|| a.troops < 0 || a.troops > 1000000 || a.food < 0 || a.food > 1000000 || a.morale < 0 || a.morale > 100 || a.movement < 0 || a.movement > 12 || arm < 0 || arm > 3) return false;
			if (version == 1 && a.general >= 18) return false;
			a.arm = static_cast<Arm>(arm);
			if (version >= 3)
			{
				a.tacticLeft = j[U"tacticLeft"].get<int>(); a.tacticReadyDay = j[U"tacticReadyDay"].get<int>(); a.tacticQueued = j[U"tacticQueued"].get<bool>();
				if (a.tacticLeft < 0 || a.tacticLeft > 5 || a.tacticReadyDay < 0 || a.tacticReadyDay > day + 30 || (a.tacticQueued && (a.troops <= 0 || a.tacticReadyDay > day || a.morale < 30))) return false;
			}
			if (loaded.Cost(a.tile,a.arm) >= 100000 || loaded.Cost(a.target,a.arm) >= 100000 || a.faction != loaded.generals[a.general].faction) return false;
			if (a.troops > 0) { if (assigned[a.general] || loaded.ArmyCount(a.faction) >= Campaign::MaxArmies) return false; assigned[a.general] = true; }
			loaded.armies.push_back(a);
			if (a.troops > 0 && !loaded.Order(static_cast<int>(loaded.armies.size()) - 1,a.target,a.retreat)) return false;
		}
		bool all = true, any = false; for (const auto& c : loaded.cities) { all = all && c.owner == player; any = any || c.owner == player; }
		loaded.result = all ? 1 : !any && loaded.ArmyCount(player) == 0 ? 2 : 0;
		loaded.log.clear(); loaded.log.push_back(U"戦況を復元しました。進路と補給を確認してください。"); ++loaded.revision;
		game = std::move(loaded); return true;
	}
	catch (...) { return false; }
}
}
