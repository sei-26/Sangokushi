#pragma once
#include <array>
#include <string>
#include <vector>
#include "OfficerTraits.hpp"
#include "WorldLayout.hpp"

namespace frontline
{
	constexpr int Width = world::Width, Height = world::Height, TileCount = Width * Height;
	using Terrain = world::Terrain;
	using SupplyGrid = std::array<std::array<int, TileCount>, 3>;
	enum class Arm
	{
		Spear,
		Bow,
		Siege,
		Cavalry
	};
	enum class Duty
	{
		Farming,
		Commerce,
		Order,
		Logistics
	};
	struct Tile
	{
		Terrain terrain = Terrain::Plain;
		int owner = -1;
	};
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
	struct Bond
	{
		int a = 0, b = 0, value = 20;
	};
	enum class MissionKind
	{
		Diplomacy,
		Sabotage
	};
	struct Mission
	{
		int general = 0, helper = -1, city = 0, target = 0, faction = 0, targetFaction = 0, left = 20;
		MissionKind kind = MissionKind::Diplomacy;
	};
	struct Chronicle
	{
		int day = 0;
		std::u32string text;
	};

} // namespace frontline
