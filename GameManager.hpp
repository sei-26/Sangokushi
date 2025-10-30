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

	Array<Point> movableTiles;  // ← 青マス表示用（新追加）

public:
	GameManager();
	void Update();

private:
	void UpdatePlayerTurn();
	void UpdateEnemyTurn();
	void DrawUI();
	bool AllUnitsActed(bool playerSide) const;
	void ResetActions();
};
