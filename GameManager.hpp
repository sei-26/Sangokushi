#pragma once
#include <Siv3D.hpp>
#include "Map.hpp"
#include "Unit.hpp"

class GameManager
{
private:
	Map map;
	std::vector<Unit> units;
	int selectedIndex = -1;
	int uiHeight = 80;
	bool playerTurn = true;
	int turnCount = 1;

public:
	GameManager();

	void Update();

private:
	void UpdatePlayerTurn();
	void UpdateEnemyTurn();
	void DrawUI();

	// 🔸 新要素
	bool AllUnitsActed(bool playerSide) const;
	void ResetActions();
};
