#pragma once
#include <Siv3D.hpp>
#include <algorithm>    // std::none_of
#include "Map.hpp"
#include "Unit.hpp"

// 勝敗結果
enum class BattleResult
{
	None,
	Victory,
	Defeat
};

class GameManager
{
private:
	Map map;
	Array<Unit> units;
	bool playerTurn = true;
	int  turnCount = 1;
	int  selectedIndex = -1;
	const int uiHeight = 80;

public:
	GameManager();

	// 毎フレーム呼ぶ
	void Update();
	void DrawUI();

	// ターン別更新
	void UpdatePlayerTurn();
	void UpdateEnemyTurn();

	// 勝敗
	bool IsBattleFinished() const;
	BattleResult GetBattleResult() const;
public:
	// 既存のpublicの最後あたりに1行追加
	const Array<Unit>& GetUnits() const { return units; }

};
