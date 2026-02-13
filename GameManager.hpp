#pragma once
#include <Siv3D.hpp>
#include "Faction.hpp"
#include "CityData.hpp"
#include "Unit_Visual.hpp"
#include "Officer.hpp"

// 前方宣言
class Map;

class GameManager
{
public:
	int year = 184;
	int month = 1;

	// 月を進める関数
	void advanceMonth(Array<CityData>& cities);

	int getYear() const { return year; }
	int getMonth() const { return month; }

	struct PendingBattle
	{
		bool isOccurring = false;
		int atkCityIndex = -1;
		int defCityIndex = -1;
	};

	PendingBattle pendingBattle;

	// ------------------------------------------------------------------
	// 互換性用ダミー機能
	// ------------------------------------------------------------------

	Array<Unit> m_dummyUnits;
	Array<Unit>& GetUnits() { return m_dummyUnits; }

	// ★ エラー修正：引数の型を正しく合わせました
	// (const CityData&, const CityData&, const Officer&, int)
	void InitializeBattle(const CityData&, const CityData&, const Officer&, int) {}

	void UpdatePKBattle() {}
	void DrawPKUI() const {}
	void DrawSRPGBoard() const {}
	bool IsBattleFinished() const { return false; }

};
