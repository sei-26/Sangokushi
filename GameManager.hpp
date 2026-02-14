#pragma once
#include <Siv3D.hpp>
#include "Faction.hpp"
#include "CityData.hpp"
#include "Unit.hpp"
#include "Officer.hpp"
#include "Saveloadmanager.hpp"

// 前方宣言
class Map;

class GameManager
{
public:
	int year = 184;
	int month = 1;
	String playerFactionName;  // ★ プレイヤー勢力名
	Stopwatch playTimer;  // ★ プレイ時間計測

	// 月を進める関数
	void advanceMonth(Array<CityData>& cities);

	int getYear() const { return year; }
	int getMonth() const { return month; }

	// ★ セーブデータを作成
	SaveData CreateSaveData(const Array<CityData>& cities) const
	{
		SaveData data;
		data.year = year;
		data.month = month;
		data.playerFactionName = playerFactionName;
		data.cities = cities;
		data.saveDateTime = DateTime::Now();
		data.playTimeSeconds = static_cast<int>(playTimer.sF());
		return data;
	}

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

	void InitializeBattle(const CityData&, const CityData&, const Officer&, int) {}

	void UpdatePKBattle() {}
	void DrawPKUI() const {}
	void DrawSRPGBoard() const {}
	bool IsBattleFinished() const { return false; }
};
