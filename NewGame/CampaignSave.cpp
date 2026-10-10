#include "CampaignSave.hpp"

namespace frontline
{
	s3d::JSON SaveJSON(const Campaign& game)
	{
		s3d::JSON json;
		json[U"version"] = 13;
		json[U"rosterCount"] = static_cast<int>(game.generals.size());
		s3d::Array<s3d::JSON> regions;
		if (game.hexMap)
			for (const auto& r : game.regions)
			{
				s3d::JSON j;
				j[U"owner"] = r.owner;
				j[U"city"] = r.city;
				regions.push_back(j);
			}
		json[U"regions"] = regions;
		json[U"hexMap"] = game.hexMap;
		json[U"aiPlannedDay"] = s3d::Array<int>(game.aiPlannedDay.begin(), game.aiPlannedDay.end());
		json[U"aiCommands"] = s3d::Array<int>(game.aiCommands.begin(), game.aiCommands.end());
		json[U"legacyLayout"] = game.legacyLayout;
		json[U"gridWidth"] = Width;
		json[U"gridHeight"] = Height;
		json[U"player"] = game.player;
		json[U"day"] = game.day;
		json[U"commands"] = game.commands;
		s3d::Array<int> owners;
		for (const auto& tile : game.tiles)
			owners.push_back(tile.owner);
		json[U"owners"] = owners;
		s3d::Array<s3d::JSON> cities, generals, armies;
		for (const auto& c : game.cities)
		{
			s3d::JSON j;
			j[U"owner"] = c.owner;
			j[U"troops"] = c.troops;
			j[U"food"] = c.food;
			j[U"gold"] = c.gold;
			cities.push_back(j);
			auto& saved = cities.back();
			saved[U"farming"] = c.farming;
			saved[U"commerce"] = c.commerce;
			saved[U"order"] = c.order;
			saved[U"logistics"] = c.logistics;
			saved[U"worker"] = c.worker;
			saved[U"work"] = c.work;
			saved[U"workLeft"] = c.workLeft;
			saved[U"helper"] = c.helper;
			saved[U"governor"] = c.governor;
		}
		for (const auto& g : game.generals)
		{
			s3d::JSON j;
			j[U"home"] = g.home;
			j[U"readyDay"] = g.readyDay;
			generals.push_back(j);
		}
		for (const auto& a : game.armies)
		{
			s3d::JSON j;
			j[U"faction"] = a.faction;
			j[U"general"] = a.general;
			j[U"tile"] = a.tile;
			j[U"target"] = a.target;
			j[U"troops"] = a.troops;
			j[U"food"] = a.food;
			j[U"cargoFood"] = a.cargoFood;
			j[U"stance"] = static_cast<int>(a.stance);
			j[U"morale"] = a.morale;
			j[U"movement"] = a.movement;
			j[U"aiAssemblyDays"] = a.aiAssemblyDays;
			j[U"arm"] = static_cast<int>(a.arm);
			j[U"supplied"] = a.supplied;
			j[U"retreat"] = a.retreat;
			armies.push_back(j);
			auto& saved = armies.back();
			saved[U"tacticLeft"] = a.tacticLeft;
			saved[U"tacticReadyDay"] = a.tacticReadyDay;
			saved[U"tacticQueued"] = a.tacticQueued;
		}
		json[U"cities"] = cities;
		json[U"generals"] = generals;
		json[U"armies"] = armies;
		s3d::Array<s3d::JSON> bonds, missions, chronicle;
		for (const auto& b : game.bonds)
		{
			s3d::JSON j;
			j[U"a"] = b.a;
			j[U"b"] = b.b;
			j[U"value"] = b.value;
			bonds.push_back(j);
		}
		for (const auto& m : game.missions)
		{
			s3d::JSON j;
			j[U"general"] = m.general;
			j[U"helper"] = m.helper;
			j[U"city"] = m.city;
			j[U"target"] = m.target;
			j[U"faction"] = m.faction;
			j[U"targetFaction"] = m.targetFaction;
			j[U"left"] = m.left;
			j[U"kind"] = static_cast<int>(m.kind);
			missions.push_back(j);
		}
		for (const auto& event : game.chronicle)
		{
			s3d::JSON j;
			j[U"day"] = event.day;
			j[U"text"] = s3d::String(event.text.c_str());
			chronicle.push_back(j);
		}
		s3d::Array<int> truces, regard;
		for (int f = 0; f < 3; ++f)
			for (int other = 0; other < 3; ++other)
			{
				truces.push_back(game.truceUntil[f][other]);
				regard.push_back(game.regard[f][other]);
			}
		json[U"bonds"] = bonds;
		json[U"missions"] = missions;
		json[U"chronicle"] = chronicle;
		json[U"truces"] = truces;
		json[U"regard"] = regard;
		json[U"randomState"] = game.randomState;
		s3d::Array<s3d::JSON> assignments;
		for (const auto& move : game.assignments)
		{
			s3d::JSON j;
			j[U"general"] = move.general;
			j[U"from"] = move.from;
			j[U"to"] = move.to;
			j[U"faction"] = move.faction;
			j[U"left"] = move.left;
			assignments.push_back(j);
		}
		json[U"assignments"] = assignments;
		return json;
	}

	// Parse into a temporary game, so invalid saves never change the running campaign.
} // namespace frontline
