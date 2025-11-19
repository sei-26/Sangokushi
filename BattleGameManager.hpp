#pragma once
#include <Siv3D.hpp>
#include "Unit.hpp"
#include "Map.hpp"
#include "Officer.hpp"
#include "CityData.hpp"

//==========================================================
// 三國志8リメイク簡易戦場：バトル管理本体
//==========================================================
class BattleGameManager
{
public:
	enum class TurnPhase
	{
		PlayerTurn,
		EnemyTurn,
		BattleEnd
	};

private:
	Map map;
	Array<Unit> units;
	TurnPhase phase = TurnPhase::PlayerTurn;

	int actingIndex = 0;
	Array<Point> moveRange;

public:
	// 初期化（都市データと主将）
	void InitializeBattle(
		const CityData& fromCity,
		const CityData& targetCity,
		const Officer& selectedLeader,
		int selectedSoldiers);
	void ApplyBattleResult(CityData& atkCity, CityData& defCity);

	// メイン更新
	void Update();

	// 描画
	void Draw() const;

	// シーン側から戦闘終了判定
	bool IsBattleFinished() const { return phase == TurnPhase::BattleEnd; }

	// 戦闘結果を都市へ反映（必要なら呼ぶ）
	bool PlayerWon() const;

private:
	// ターン関連
	void UpdatePlayerTurn();
	void UpdateEnemyTurn();

	void SelectUnit();
	void CalculateMoveRange();
	void MoveUnit();
	void TryAttack();

	void EnemyAction(int targetIdx);

	// 戦闘
	bool AreAdjacent(const Unit& a, const Unit& b) const;
	void ResolveCombat(Unit& attacker, Unit& defender);

	// AI
	int FindClosestEnemyIndex(int i) const;

	// タイル周り
	bool CanMoveTo(int x, int y) const;
	Array<Point> GetNeighbors(const Point& p) const;
	Array<Point> FindPath(const Point& start, const Point& goal) const;
};
