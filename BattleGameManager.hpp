#pragma once
#include <Siv3D.hpp>
#include "Unit.hpp"
#include "Map.hpp"
#include "CityData.hpp"
#include "Officer.hpp"
#include "BattleSystem.hpp" // 追加：Weather 型のためのインクルード

enum class TurnPhase
{
	PlayerTurn,
	EnemyTurn,
	BattleEnd
};

class BattleGameManager
{
public:
	Map map;
	Array<Unit> units;
	TurnPhase phase = TurnPhase::PlayerTurn;

	int actingIndex = 0;        // 行動中のユニット番号
	Array<Point> moveRange;     // 移動可能範囲

	// 初期化
	void InitializeBattle(const CityData& fromCity, const CityData& targetCity, const Officer& leader, int soldiers, bool  playerIsAtk);

	// 更新と描画
	void Update();
	void Draw() const;

	//戦闘終了判定（エラー修正）
	bool IsBattleFinished() const;

	//プレイヤーが攻撃側かどうか
	bool isPlayerAttacker = true;

	// 戦闘結果の適用
	void ApplyBattleResult(CityData& atkCity, CityData& defCity);

private:
	// 追加：天候状態を保持するメンバ
	BattleSystem::Weather m_weather = BattleSystem::Weather::Sunny;

	// 内部処理用
	void UpdatePlayerTurn();
	void UpdateEnemyTurn();

	void CalculateMoveRange();
	void MoveUnit();
	void TryAttack();

	void EnemyAction(int targetIdx);
	int FindClosestEnemyIndex(int myIdx) const;

	bool AreAdjacent(const Unit& a, const Unit& b) const;
	void ResolveCombat(Unit& atk, Unit& def);
	bool CanMoveTo(int x, int y) const;

	Array<Point> GetNeighbors(const Point& p) const;
	Array<Point> FindPath(const Point& start, const Point& goal) const;
	bool PlayerWon() const;
	bool PlayerLost() const;

	

};
