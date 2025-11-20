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
	void InitializeBattle(
		const CityData& fromCity,
		const CityData& targetCity,
		const Officer& selectedLeader,
		int selectedSoldiers);

	void ApplyBattleResult(CityData& atkCity, CityData& defCity);

	void Update();

	void Draw() const;

	bool IsBattleFinished() const { return phase == TurnPhase::BattleEnd; }

	bool PlayerWon() const;

private:
	void UpdatePlayerTurn();
	void UpdateEnemyTurn();

	void SelectUnit();
	void CalculateMoveRange();
	void MoveUnit();
	void TryAttack();

	void EnemyAction(int targetIdx);

	bool AreAdjacent(const Unit& a, const Unit& b) const;
	void ResolveCombat(Unit& attacker, Unit& defender);

	int FindClosestEnemyIndex(int i) const;

	bool CanMoveTo(int x, int y) const;
	Array<Point> GetNeighbors(const Point& p) const;
	Array<Point> FindPath(const Point& start, const Point& goal) const;
};
