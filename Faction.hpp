#pragma once
#include <Siv3D.hpp>
#include "CityData.hpp"
#include "Unit.hpp"

struct Faction
{
	String name;
	Color color;
	Array<String> cities;
	Array<Unit> units;
	int gold = 1000;
	int food = 1000;
};
