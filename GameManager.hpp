#pragma once
#include <Siv3D.hpp>
#include "Officer.hpp"
#include "Map.hpp"    // ← はるのプロジェクトのマップクラス
#include "Unit.hpp"   // ← はるのユニットクラス

// ========================================
// SRPG表示 × 8PK式ダメージ の融合版 GameManager
// ========================================
class GameManager
{
private:
	Map map;                 // ★ はるのマップクラス（Tileではない）
	Array<Unit> units;       // ★ ユニット
	int selectedIndex = -1;

	int turnCount = 1;
	bool battleFinished = false;

	// ★ 8PK戦闘モード変数
	bool pkMode = false;
	double pkTimer = 0.0;

public:
	GameManager();

	// バトル初期化（SRPG見た目 + 8PK戦闘処理）
	void InitializeBattle(const Officer& attacker,
						  const Officer& defender,
						  int attackerSoldiers,
						  int defenderSoldiers);

	// 8PK式じわじわダメージ
	void UpdatePKBattle();

	// SRPGの見た目を描画（マップ + ユニット）
	void DrawSRPGBoard() const;

	// 8PKステータスUI
	void DrawPKUI() const;

	// 勝敗判定
	bool IsBattleFinished() const { return battleFinished; }

	// Getter
	const Array<Unit>& GetUnits() const { return units; }
	int GetTurnCount() const { return turnCount; }
	Map& GetMap() { return map; }
};
