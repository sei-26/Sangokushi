#include "../NewGame/CampaignSave.hpp"
#include "../NewGame/StorySave.hpp"
#include <cassert>
#include <iostream>

// Satisfy the engine library's entry-point reference without starting its loop.
void Main() {}

JSON OldStory(const hero::Story& story,int version) { JSON j=hero::SaveJSON(story);j[U"version"]=version; for(size_t n=0;n<story.units.size();++n){Array<int> values;for(int i=0;i<12;++i)values.push_back(j[U"units"][n][U"values"][i].get<int>());j[U"units"][n][U"values"]=values;}return j;}
int main()
{
	hero::Story tale; tale.Reset(); tale.Choose(0); tale.Choose(1); tale.Act(0,0,1); tale.EndTurn();
	hero::Story restoredTale; assert(hero::LoadJSON(hero::SaveJSON(tale),restoredTale));
	assert(restoredTale.turn==tale.turn && restoredTale.units[0].x==tale.units[0].x && restoredTale.decisions==tale.decisions && restoredTale.journal==tale.journal);
	for (int chapter=0;chapter<6;++chapter) {
		tale.chapter=chapter; tale.phase=2; tale.StartMission(); assert(hero::LoadJSON(hero::SaveJSON(tale),restoredTale));
		tale.Finish(false); assert(hero::LoadJSON(hero::SaveJSON(tale),restoredTale)&&restoredTale.failed); tale.Retry(); tale.Finish(true); assert(hero::LoadJSON(hero::SaveJSON(tale),restoredTale));
	}
	tale.Choose(0); assert(hero::LoadJSON(hero::SaveJSON(tale),restoredTale)&&restoredTale.phase==4);
	JSON badStory=hero::SaveJSON(tale); badStory[U"state"]=Array<int>{99}; assert(!hero::LoadJSON(badStory,restoredTale)&&restoredTale.phase==4);
	tale.chapter=3;tale.phase=2;tale.StartMission();badStory=hero::SaveJSON(tale);badStory[U"units"][0][U"values"]=Array<int>{};assert(!hero::LoadJSON(badStory,restoredTale));
	std::cout << "Story save tests passed\n";
	hero::Story eventSave;eventSave.chapter=3;eventSave.phase=2;eventSave.StartMission();eventSave.battleEvent=eventSave.eventMask=1;eventSave.turn=4;eventSave.spirit=75;
	assert(hero::LoadJSON(hero::SaveJSON(eventSave),restoredTale)&&restoredTale.battleEvent==1&&restoredTale.spirit==75);
	assert(eventSave.ResolveBattleEvent(1));eventSave.Rally();assert(hero::LoadJSON(hero::SaveJSON(eventSave),restoredTale)&&restoredTale.battleDecisions==eventSave.battleDecisions&&restoredTale.rallies==1&&restoredTale.spirit==eventSave.spirit);
	JSON oldStory=OldStory(eventSave,1);assert(hero::LoadJSON(oldStory,restoredTale)&&restoredTale.spirit==20&&restoredTale.battleEvent==0);
	badStory=hero::SaveJSON(eventSave);badStory[U"drama"]=Array<int>{101,0,1,0};assert(!hero::LoadJSON(badStory,restoredTale)&&restoredTale.spirit==20);
	std::cout << "Battle decisions, rally and legacy story saves passed\n";
	hero::Story assignment;assignment.chapter=3;assignment.phase=1;assignment.AssignPlanner(3);assignment.ChangeBond(1,3,55);assignment.Choose(0);
	assert(hero::LoadJSON(hero::SaveJSON(assignment),restoredTale)&&restoredTale.planner==3&&restoredTale.Affinity(1,3)==75&&restoredTale.units[3].acted&&restoredTale.units[0].maxHp==assignment.units[0].maxHp);
	JSON versionTwo=OldStory(assignment,2);assert(hero::LoadJSON(versionTwo,restoredTale)&&restoredTale.planner==-1&&restoredTale.Affinity(1,3)==20&&restoredTale.units[0].maxHp==assignment.units[0].maxHp);
	JSON badPair=hero::SaveJSON(assignment);badPair[U"relationships"][0][U"b"]=99;assert(!hero::LoadJSON(badPair,restoredTale)&&restoredTale.planner==-1);
	badPair=hero::SaveJSON(assignment);badPair[U"planner"]=5;assert(!hero::LoadJSON(badPair,restoredTale));
	std::cout << "Assignment, pair relationships and version 2 compatibility passed\n";
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
	// A real pre-expansion layout uses packed 36-column indices.
 frontline::Campaign oldWorld;oldWorld.ResetLegacy(0);
 const int legacyArmy=oldWorld.Deploy(2,4,3000,frontline::Arm::Spear);
 assert(oldWorld.Order(legacyArmy,oldWorld.cities[3].tile));
 JSON oldWorldJSON=frontline::SaveJSON(oldWorld);oldWorldJSON[U"version"]=3;
 Array<int> packedOwners;
 for(int y=0;y<22;++y)for(int x=0;x<36;++x)packedOwners.push_back(oldWorld.tiles[frontline::Campaign::At(x,y)].owner);
 oldWorldJSON[U"owners"]=packedOwners;
 for(size_t index=0;index<oldWorld.armies.size();++index){
  auto armyJSON=oldWorldJSON[U"armies"][index];
  const int tile=armyJSON[U"tile"].get<int>(),target=armyJSON[U"target"].get<int>();
  armyJSON[U"tile"]=tile/frontline::Width*36+tile%frontline::Width;
  armyJSON[U"target"]=target/frontline::Width*36+target%frontline::Width;
  oldWorldJSON[U"armies"][index]=armyJSON;
 }
 frontline::Campaign legacyImported;assert(frontline::LoadJSON(oldWorldJSON,legacyImported));
 assert(legacyImported.legacyLayout && legacyImported.cities.size()==9 && legacyImported.armies[0].tile==oldWorld.armies[0].tile && legacyImported.armies[0].target==oldWorld.armies[0].target);
 for(int p=0;p<frontline::TileCount;++p)assert(legacyImported.tiles[p].owner==oldWorld.tiles[p].owner && legacyImported.tiles[p].terrain==oldWorld.tiles[p].terrain);
 assert(frontline::LoadJSON(frontline::SaveJSON(legacyImported),loaded) && loaded.legacyLayout && loaded.MapWidth()==36 && loaded.armies[0].tile==oldWorld.armies[0].tile);
 for(int oldVersion:{1,2}){
  JSON older=oldWorldJSON;older[U"version"]=oldVersion;
  if(oldVersion==1){
   Array<JSON> firstOfficers;
   for(int i=0;i<18;++i){JSON officer;officer[U"home"]=oldWorld.generals[i].home;officer[U"readyDay"]=oldWorld.generals[i].readyDay;firstOfficers.push_back(officer);}
   older[U"generals"]=firstOfficers;
  }
  assert(frontline::LoadJSON(older,loaded) && loaded.legacyLayout && loaded.cities.size()==9 && loaded.armies[0].tile==oldWorld.armies[0].tile);
 }
 JSON invalidDimensions=frontline::SaveJSON(campaign);invalidDimensions[U"gridWidth"]=36;
 assert(!frontline::LoadJSON(invalidDimensions,loaded) && loaded.legacyLayout);
 assert(frontline::LoadJSON(frontline::SaveJSON(campaign),loaded) && !loaded.legacyLayout && loaded.cities.size()==30);
 frontline::Campaign placement;placement.Reset(0);assert(placement.AssignOfficer(19,9));
 placement.AdvanceDay();frontline::Campaign placementLoaded;
 assert(frontline::LoadJSON(frontline::SaveJSON(placement),placementLoaded));
 assert(placementLoaded.assignments.size()==1 && placementLoaded.assignments[0].left==placement.assignments[0].left && placementLoaded.Busy(19));
 const int remaining=placement.assignments[0].left;
 for(int d=0;d<remaining;++d){placement.AdvanceDay();placementLoaded.AdvanceDay();}
 assert(placementLoaded.assignments.empty() && placementLoaded.generals[19].home==9 && placementLoaded.generals[19].home==placement.generals[19].home);
 frontline::Campaign versionFour;versionFour.Reset(0);JSON previousWorld=frontline::SaveJSON(versionFour);previousWorld[U"version"]=4;
 assert(frontline::LoadJSON(previousWorld,placementLoaded) && placementLoaded.assignments.empty());
 placement.Reset(0);assert(placement.AssignOfficer(19,9));
 JSON badPlacement=frontline::SaveJSON(placement);badPlacement[U"assignments"][0][U"left"]=0;
 assert(!frontline::LoadJSON(badPlacement,placementLoaded) && placementLoaded.assignments.empty());
 badPlacement=frontline::SaveJSON(placement);badPlacement[U"assignments"][0][U"general"]=0;
 assert(frontline::LoadJSON(badPlacement,placementLoaded));
 badPlacement=frontline::SaveJSON(placement);badPlacement[U"assignments"][0][U"to"]=3;assert(!frontline::LoadJSON(badPlacement,placementLoaded));
 frontline::Campaign duplicate;duplicate.Reset(0);assert(duplicate.Develop(0,19,frontline::Duty::Commerce));
 badPlacement=frontline::SaveJSON(duplicate);badPlacement[U"assignments"]=frontline::SaveJSON(placement)[U"assignments"];assert(!frontline::LoadJSON(badPlacement,placementLoaded));
 badPlacement=frontline::SaveJSON(placement);badPlacement[U"assignments"][0][U"general"]=-1;assert(!frontline::LoadJSON(badPlacement,placementLoaded));
 badPlacement=frontline::SaveJSON(placement);Array<JSON> duplicateMoves;duplicateMoves.push_back(badPlacement[U"assignments"][0]);duplicateMoves.push_back(badPlacement[U"assignments"][0]);badPlacement[U"assignments"]=duplicateMoves;assert(!frontline::LoadJSON(badPlacement,placementLoaded));

 frontline::Campaign convoy;convoy.Reset(0);assert(convoy.DispatchTransport(0,19,9,10000)==0);
 for(int d=0;d<5;++d)convoy.AdvanceDay();
 frontline::Campaign convoyLoaded;
 assert(frontline::LoadJSON(frontline::SaveJSON(convoy),convoyLoaded));
 assert(convoyLoaded.Busy(19) && convoyLoaded.armies[0].cargoFood==10000 && convoyLoaded.armies[0].tile==convoy.armies[0].tile);
 for(int d=0;d<30;++d){convoy.AdvanceDay();convoyLoaded.AdvanceDay();}
 assert(convoyLoaded.armies[0].troops==0 && convoyLoaded.cities[9].food==convoy.cities[9].food && convoyLoaded.cities[0].food==convoy.cities[0].food);
 JSON badCargo=frontline::SaveJSON(convoyLoaded);badCargo[U"armies"][0][U"cargoFood"]=100;assert(!frontline::LoadJSON(badCargo,convoyLoaded));
 convoy.Reset(0);assert(convoy.DispatchTransport(0,19,9,10000)==0);
 badCargo=frontline::SaveJSON(convoy);badCargo[U"armies"][0][U"arm"]=0;assert(!frontline::LoadJSON(badCargo,convoyLoaded));
 badCargo=frontline::SaveJSON(convoy);badCargo[U"armies"][0][U"cargoFood"]=-1;assert(!frontline::LoadJSON(badCargo,convoyLoaded));
 badCargo=frontline::SaveJSON(convoy);badCargo[U"armies"][0][U"tacticQueued"]=true;assert(!frontline::LoadJSON(badCargo,convoyLoaded));
 // An interrupted road is a valid save; the cargo stays aboard and can resume.
 for(auto& tile:convoy.tiles)if(tile.terrain!=frontline::Terrain::Sea)tile.owner=-1;
 for(const auto& city:convoy.cities)convoy.tiles[city.tile].owner=city.owner;
 convoy.AdvanceDay();
 assert(frontline::LoadJSON(frontline::SaveJSON(convoy),convoyLoaded));
 assert(convoyLoaded.armies[0].path.empty() && convoyLoaded.armies[0].cargoFood==10000);
 JSON versionFive=frontline::SaveJSON(versionFour);versionFive[U"version"]=5;
 assert(frontline::LoadJSON(versionFive,convoyLoaded) && convoyLoaded.armies.empty());
 std::cout << "Convoy save round trips and version 5 compatibility passed" << std::endl;

 hero::Story depth;depth.phase=2;depth.StartMission();depth.objectiveProgress=1;depth.outcomes[1]=2;depth.civilActions[2]=2;depth.units[0].moved=true;assert(depth.Guard(0));
 assert(hero::LoadJSON(hero::SaveJSON(depth),restoredTale) && restoredTale.deepRules && restoredTale.objectiveProgress==1 && restoredTale.outcomes[1]==2 && restoredTale.units[0].moved && restoredTale.units[0].guarding);
 JSON badDepth=hero::SaveJSON(depth);badDepth[U"objectiveProgress"]=3;assert(!hero::LoadJSON(badDepth,restoredTale));
 badDepth=hero::SaveJSON(depth);badDepth[U"outcomes"]=Array<int>{};assert(!hero::LoadJSON(badDepth,restoredTale));
 assert(hero::LoadJSON(OldStory(depth,3),restoredTale) && !restoredTale.deepRules && !restoredTale.units[0].guarding && restoredTale.objectiveProgress==0);
 frontline::Campaign stanceSave;stanceSave.Reset(0);int unit=stanceSave.Deploy(0,0,3000,frontline::Arm::Bow);
 assert(stanceSave.SetStance(unit,battle::Stance::Guard));assert(frontline::LoadJSON(frontline::SaveJSON(stanceSave),loaded) && loaded.armies[unit].stance==battle::Stance::Guard);
 JSON badStance=frontline::SaveJSON(stanceSave);badStance[U"armies"][0][U"stance"]=3;assert(!frontline::LoadJSON(badStance,loaded));
 badStance=frontline::SaveJSON(stanceSave);badStance[U"version"]=6;assert(frontline::LoadJSON(badStance,loaded) && loaded.armies[0].stance==battle::Stance::Balanced);
 std::cout << "Deep story and stance saves; version 3 and 6 compatibility passed" << std::endl;

 frontline::Campaign docking;docking.Reset(0);int dock=docking.Deploy(0,0,3000,frontline::Arm::Spear);assert(docking.ActivateTactic(dock) && docking.ReturnToCity(dock,0));
 assert(frontline::LoadJSON(frontline::SaveJSON(docking),loaded) && !loaded.Busy(0) && !loaded.armies[0].tacticQueued);
 docking.Reset(0);dock=docking.Deploy(0,0,3000,frontline::Arm::Spear);assert(docking.ReturnToCity(dock,9));
 assert(frontline::LoadJSON(frontline::SaveJSON(docking),loaded) && loaded.armies[0].retreat && loaded.armies[0].target==loaded.cities[9].tile);

 frontline::Campaign aiSave;aiSave.Reset(0);aiSave.BeginTurn();frontline::Campaign aiLoaded;
 assert(frontline::LoadJSON(frontline::SaveJSON(aiSave),aiLoaded) && aiLoaded.aiPlannedDay==aiSave.aiPlannedDay && aiLoaded.aiCommands==aiSave.aiCommands);
 const auto aiNotes=aiLoaded.chronicle.size();aiLoaded.BeginTurn();assert(aiLoaded.chronicle.size()==aiNotes);
 for(int d=0;d<90;++d){if(d%10==0){aiSave.BeginTurn();aiLoaded.BeginTurn();}aiSave.AdvanceDay();aiLoaded.AdvanceDay();}
 assert(aiLoaded.armies.size()==aiSave.armies.size() && aiLoaded.aiCommands==aiSave.aiCommands && aiLoaded.randomState==aiSave.randomState);
 for(size_t c=0;c<aiSave.cities.size();++c)assert(aiLoaded.cities[c].food==aiSave.cities[c].food && aiLoaded.cities[c].gold==aiSave.cities[c].gold && aiLoaded.cities[c].owner==aiSave.cities[c].owner);
 JSON badAI=frontline::SaveJSON(aiSave);badAI[U"aiCommands"]=Array<int>{3,4,3};assert(!frontline::LoadJSON(badAI,aiLoaded));
 badAI=frontline::SaveJSON(aiSave);badAI[U"aiPlannedDay"]=Array<int>{0,aiSave.day+1,0};assert(!frontline::LoadJSON(badAI,aiLoaded));
 badAI=frontline::SaveJSON(aiSave);badAI[U"version"]=7;assert(frontline::LoadJSON(badAI,aiLoaded) && aiLoaded.aiPlannedDay[1]==-1 && aiLoaded.aiCommands[1]==3);
 // Preserve an offensive assembly timer, including the exact day of cancellation.
 frontline::Campaign assembly;assembly.Reset(0);assembly.day=1;
 int staged=assembly.Deploy(3,6,3000,frontline::Arm::Siege,true);assert(staged>=0);
 assert(assembly.Order(staged,assembly.cities[0].tile));assembly.armies[staged].aiAssemblyDays=12;
 assert(frontline::LoadJSON(frontline::SaveJSON(assembly),aiLoaded) && aiLoaded.armies[staged].aiAssemblyDays==12);
 JSON assemblyJSON=frontline::SaveJSON(assembly);assemblyJSON[U"armies"][staged][U"aiAssemblyDays"]=21;assert(!frontline::LoadJSON(assemblyJSON,aiLoaded));
 assemblyJSON=frontline::SaveJSON(assembly);assemblyJSON[U"version"]=8;assert(frontline::LoadJSON(assemblyJSON,aiLoaded) && aiLoaded.armies[staged].aiAssemblyDays==0);
 assert(frontline::LoadJSON(frontline::SaveJSON(assembly),aiLoaded));
 for(int d=0;d<8;++d){assembly.AdvanceDay();aiLoaded.AdvanceDay();}
 assert(aiLoaded.armies[staged].troops==assembly.armies[staged].troops && aiLoaded.armies[staged].aiAssemblyDays==assembly.armies[staged].aiAssemblyDays && aiLoaded.armies[staged].retreat==assembly.armies[staged].retreat);
 std::cout << "AI assembly saves and version 8 compatibility passed" << std::endl;
 frontline::Campaign hexSaved;hexSaved.Reset(0);int hexUnit=hexSaved.Deploy(0,0,3000,frontline::Arm::Spear);assert(hexUnit>=0 && hexSaved.Order(hexUnit,hexSaved.cities[3].tile));
 assert(frontline::LoadJSON(frontline::SaveJSON(hexSaved),aiLoaded) && aiLoaded.hexMap && aiLoaded.armies[hexUnit].path==hexSaved.armies[hexUnit].path);
 assert(aiLoaded.regions.size()==hexSaved.regions.size() && aiLoaded.tileRegion==hexSaved.tileRegion);
 JSON regionBad=frontline::SaveJSON(hexSaved);regionBad[U"regions"][0][U"owner"]=2;assert(!frontline::LoadJSON(regionBad,aiLoaded));
 JSON oldHex=frontline::SaveJSON(hexSaved);oldHex[U"version"]=10;assert(frontline::LoadJSON(oldHex,aiLoaded) && aiLoaded.hexMap && aiLoaded.regions.empty());
 JSON squareOld=frontline::SaveJSON(hexSaved);squareOld[U"version"]=9;assert(frontline::LoadJSON(squareOld,aiLoaded) && !aiLoaded.hexMap);
 int squareStep=aiLoaded.armies[hexUnit].tile;for(int p:aiLoaded.armies[hexUnit].path){assert(aiLoaded.MapDistance(squareStep,p)==1);squareStep=p;}
 std::cout << "Hex saves and version 9 square-map migration passed" << std::endl;
 std::cout << "AI budget persistence and deterministic continuation passed" << std::endl;
 std::cout << "Personnel save state and version 4 compatibility passed" << std::endl;
 std::cout << "Campaign save tests passed; old world migration and new world round trips passed\n";
}
