#include "../TerritoryMap.hpp"
#include <cassert>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

int main()
{
	TerritoryMap map;
	map.Initialize({{400, 410}, {1000, 410}});
	assert(TerritoryMap::Index(-1, 0) == -1);
	assert(TerritoryMap::Index(1520, 0) == -1);
	assert(TerritoryMap::Index(0, 820) == -1);
	assert(TerritoryMap::Index(1519, 819) == TerritoryMap::Count - 1);
	assert(!map.Claim(-1) && !map.Develop(TerritoryMap::Count));
	assert(map.CityGold(0) > 0 && map.CityFood(1) > 0);
	int frontier = -1;
	for (int i = 0; i < TerritoryMap::Count; ++i)
		if (map.cells[i].city == 0 && map.CanClaim(i)) { frontier = i; break; }
	assert(frontier >= 0);
	const int beforeGold = map.CityGold(0), beforeFood = map.CityFood(0);
	assert(map.Claim(frontier));
	assert(!map.Claim(frontier));
	assert(map.CityGold(0) == beforeGold + TerritoryMap::Gold(map.cells[frontier], frontier));
	assert(map.CityFood(0) == beforeFood + TerritoryMap::Food(map.cells[frontier], frontier));
	assert(map.Develop(frontier) && map.Develop(frontier));
	assert(!map.Develop(frontier));
	assert(map.cells[frontier].level == 3);
	// Horizontal adjacency must not wrap between rows or cross city districts.
	map.cells.fill({-1, 0});
	map.cells[37] = {0, 1}; map.cells[38] = {0, 0};
	assert(!map.CanClaim(38));
	map.cells[39] = {1, 1};
	assert(!map.CanClaim(38));
	map.cells[39] = {0, 1};
	assert(map.CanClaim(38));
	map.Initialize({});
	assert(map.CityGold(0) == 0 && !map.Claim(0));
	// Every city in the actual scenario must have an economic foothold.
	std::ifstream csv("App/cities.csv");
	assert(csv.good());
	std::string line;
	std::getline(csv, line);
	std::vector<TerritoryMap::CityPosition> positions;
	while (std::getline(csv, line))
	{
		if (line.empty()) continue;
		std::istringstream row(line);
		std::string field;
		std::getline(row, field, ','); std::getline(row, field, ',');
		std::getline(row, field, ','); const double x = std::stod(field);
		std::getline(row, field, ','); const double y = std::stod(field);
		positions.push_back({x, y});
	}
	assert(!positions.empty());
	map.Initialize(positions);
	for (int c = 0; c < static_cast<int>(positions.size()); ++c)
		assert(map.CityGold(c) > 0 && map.CityFood(c) > 0);
	std::cout << "TerritoryMap tests passed\n";
}
