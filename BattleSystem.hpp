#pragma once
#include <Siv3D.hpp>
#include "GameManager.hpp"
#include "CityData.hpp"
#include "Officer.hpp"

struct BattleSetup
{
	CityData* attackerCity = nullptr;
	CityData* defenderCity = nullptr;
	Officer* attackerLeader = nullptr;
	Officer* defenderLeader = nullptr;
	int attackerSoldiers = 0;
	int defenderSoldiers = 0;
};

class BattleSystem
{
private:
	BattleSetup setup;
	GameManager battle;
	bool finished = false;

public:
	BattleSystem(const BattleSetup& s);

	void Update();
	bool IsFinished() const { return finished; }

	void ApplyResultToCities();

	GameManager& GetManager() { return battle; }
};
