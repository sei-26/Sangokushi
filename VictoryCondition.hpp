#pragma once
#include <Siv3D.hpp>
#include "CityData.hpp"

class VictoryCondition
{
public:
	enum class VictoryType
	{
		None,
		Domination,  // 統一勝利
		TimeLimit,   // 時間切れ
		Defeat       // 敗北
	};

	// 勝利判定
	static VictoryType CheckVictory(const Array<CityData>& cities, const String& playerFaction, int year, int month)
	{
		int playerCities = 0;
		int totalCities = cities.size();

		for (const auto& city : cities)
		{
			if (city.owner == playerFaction)
			{
				playerCities++;
			}
		}

		// 統一勝利
		if (playerCities == totalCities)
		{
			return VictoryType::Domination;
		}

		// 敗北（都市0）
		if (playerCities == 0)
		{
			return VictoryType::Defeat;
		}

		// 時間切れ（220年12月）
		if (year >= 220 && month >= 12)
		{
			return VictoryType::TimeLimit;
		}

		return VictoryType::None;
	}

	// 勝利タイプ名
	static String GetVictoryName(VictoryType type)
	{
		switch (type)
		{
		case VictoryType::Domination: return U"天下統一";
		case VictoryType::TimeLimit: return U"最大勢力";
		case VictoryType::Defeat: return U"敗北";
		default: return U"";
		}
	}

	// スコア計算
	static int CalculateScore(const Array<CityData>& cities, const String& playerFaction, int year, int month)
	{
		int score = 0;

		for (const auto& city : cities)
		{
			if (city.owner == playerFaction)
			{
				score += 1000; // 都市1つ1000点
				score += city.troops / 10;
				score += city.gold / 10;
				score += city.officers.size() * 100;
			}
		}

		// 早期クリアボーナス
		int turnsUsed = (year - 200) * 12 + month;
		int turnBonus = Max(0, 10000 - turnsUsed * 50);
		score += turnBonus;

		return score;
	}
};
