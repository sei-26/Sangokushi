#include "../Saveloadmanager.hpp"
#include "../NewGame/CampaignSave.hpp"
#include <cassert>
#include <iostream>

// Satisfy the engine library's entry-point reference without starting its loop.
void Main() {}

int main()
{
	SaveData original;
	original.playerFactionName = U"Test";
	original.cities.push_back(CityData(U"City", Point(400, 410), U"Test"));
	original.territory.Initialize({{400, 410}});
	original.territoryReady = true;
	original.commandsUsed = 2;
	for (int i = 0; i < TerritoryMap::Count; ++i)
		if (original.territory.CanClaim(i)) { original.territory.Claim(i); original.territory.Develop(i); break; }
	const auto restored = SaveData::fromJSON(original.toJSON());
	assert(restored && restored->territoryReady && restored->commandsUsed == 2);
	for (int i = 0; i < TerritoryMap::Count; ++i)
	{
		assert(restored->territory.cells[i].city == original.territory.cells[i].city);
		assert(restored->territory.cells[i].level == original.territory.cells[i].level);
	}
	original.territoryReady = false;
	const auto legacy = SaveData::fromJSON(original.toJSON());
	assert(legacy && !legacy->territoryReady);
	JSON invalid = original.toJSON();
	Array<JSON> badCells;
	JSON bad; bad[U"city"] = 100; bad[U"level"] = 1;
	for (int i = 0; i < TerritoryMap::Count; ++i) badCells.push_back(bad);
	invalid[U"territory"] = badCells;
	assert(!SaveData::fromJSON(invalid));
	invalid[U"territory"] = Array<JSON>{};
	assert(!SaveData::fromJSON(invalid));
	std::cout << "SaveData tests passed\n";
	frontline::Campaign campaign; campaign.Reset(0);
	assert(campaign.Develop(0,19,frontline::Duty::Commerce));
	const int army = campaign.Deploy(2,4,3000,frontline::Arm::Spear);
	assert(campaign.Order(army,campaign.cities[3].tile)); campaign.BeginTurn();
	for (int i = 0; i < 10; ++i) campaign.AdvanceDay();
	frontline::Campaign loaded;
	assert(frontline::LoadJSON(frontline::SaveJSON(campaign),loaded));
	assert(loaded.day == campaign.day && loaded.commands == campaign.commands && loaded.armies.size() == campaign.armies.size());
	assert(loaded.cities[0].worker == 19 && loaded.cities[0].workLeft == 20);
	JSON badWork = frontline::SaveJSON(campaign); badWork[U"cities"][0][U"worker"] = 4;
	assert(!frontline::LoadJSON(badWork,loaded));
	JSON oldCampaign = frontline::SaveJSON(campaign); oldCampaign[U"version"] = 1;
	Array<JSON> oldGenerals; for (int i = 0; i < 18; ++i) { JSON g; g[U"home"] = campaign.generals[i].home; g[U"readyDay"] = campaign.generals[i].readyDay; oldGenerals.push_back(g); } oldCampaign[U"generals"] = oldGenerals;
	frontline::Campaign migrated; assert(frontline::LoadJSON(oldCampaign,migrated));
	assert(migrated.generals.size() == 24 && migrated.cities[0].worker == -1 && migrated.armies.size() == campaign.armies.size());
	for (int p = 0; p < frontline::TileCount; ++p) assert(loaded.tiles[p].owner == campaign.tiles[p].owner);
	for (size_t i = 0; i < campaign.armies.size(); ++i)
		assert(loaded.armies[i].tile == campaign.armies[i].tile && loaded.armies[i].troops == campaign.armies[i].troops && loaded.armies[i].target == campaign.armies[i].target);
	JSON corrupted = frontline::SaveJSON(campaign); corrupted[U"player"] = -10;
	assert(!frontline::LoadJSON(corrupted,loaded) && loaded.day == campaign.day);
	corrupted = frontline::SaveJSON(campaign); corrupted[U"owners"] = Array<int>{};
	assert(!frontline::LoadJSON(corrupted,loaded));
	frontline::Campaign people; people.Reset(0);
	assert(people.Develop(0,19,frontline::Duty::Commerce,false,1));
	assert(people.SendMission(1,18,3,frontline::MissionKind::Diplomacy,3));
	const int personalArmy = people.Deploy(2,4,3000,frontline::Arm::Spear); assert(people.ActivateTactic(personalArmy));
	for (int i = 0; i < 5; ++i) people.AdvanceDay(); people.ChangeBond(19,1,10);
	people.truceUntil[0][2] = people.truceUntil[2][0] = 45;
	frontline::Campaign restoredPeople; assert(frontline::LoadJSON(frontline::SaveJSON(people),restoredPeople));
	assert(restoredPeople.cities[0].helper == 1 && restoredPeople.missions.size() == 1 && restoredPeople.missions[0].helper == 3 && restoredPeople.missions[0].left == 15);
	assert(restoredPeople.Affinity(19,1) == people.Affinity(19,1) && restoredPeople.truceUntil[0][2] == 45 && restoredPeople.randomState == people.randomState);
	assert(restoredPeople.armies[0].tacticLeft == people.armies[0].tacticLeft && restoredPeople.armies[0].tacticReadyDay == people.armies[0].tacticReadyDay);
	assert(restoredPeople.chronicle.size() == people.chronicle.size() && restoredPeople.chronicle.back().text == people.chronicle.back().text);
	JSON badPeople = frontline::SaveJSON(people); badPeople[U"missions"][0][U"general"] = 19; assert(!frontline::LoadJSON(badPeople,restoredPeople));
	badPeople = frontline::SaveJSON(people); badPeople[U"cities"][0][U"helper"] = 19; assert(!frontline::LoadJSON(badPeople,restoredPeople));
	badPeople = frontline::SaveJSON(people); badPeople[U"bonds"][0][U"value"] = 101; assert(!frontline::LoadJSON(badPeople,restoredPeople));
	badPeople = frontline::SaveJSON(people); badPeople[U"truces"][2] = 46; assert(!frontline::LoadJSON(badPeople,restoredPeople));
	badPeople = frontline::SaveJSON(people); badPeople[U"armies"][0][U"tacticQueued"] = true; assert(!frontline::LoadJSON(badPeople,restoredPeople));
	JSON previousFormat = frontline::SaveJSON(people); previousFormat[U"version"] = 2;
	assert(frontline::LoadJSON(previousFormat,restoredPeople) && restoredPeople.cities[0].helper == -1 && restoredPeople.missions.empty());
	frontline::Campaign evolving; evolving.Reset(0);
	for (int day = 0; day < 180 && evolving.result == 0; ++day)
	{
		if (day % 10 == 0) evolving.BeginTurn(); evolving.AdvanceDay();
		if (day % 30 == 0) { frontline::Campaign snapshot; assert(frontline::LoadJSON(frontline::SaveJSON(evolving),snapshot)); }
	}
	std::cout << "Campaign save tests passed\n";
}
