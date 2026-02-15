#pragma once
#include <Siv3D.hpp>
#include "CityData.hpp"

// 経済システム
class EconomyManager
{
public:
	// 月次収入を計算・適用
	static void ApplyMonthlyIncome(Array<CityData>& cities)
	{
		for (auto& city : cities)
		{
			// 金収入 = 商業値 × 10
			int goldIncome = city.commerce * 10;
			city.gold += goldIncome;

			// 兵糧収入 = 農業値 × 10
			int foodIncome = city.agriculture * 10;
			city.food += foodIncome;

			// ログ出力
			Print << U"[収入] " << city.name << U": 金+" << goldIncome
				<< U", 糧+" << foodIncome;
		}
	}

	// 兵士維持費を適用
	static void ApplyTroopMaintenance(Array<CityData>& cities)
	{
		for (auto& city : cities)
		{
			// 兵士維持費 = 兵士数 / 10
			int maintenanceCost = city.troops / 10;

			if (city.food >= maintenanceCost)
			{
				// 兵糧がある：維持費を支払う
				city.food -= maintenanceCost;
			}
			else
			{
				// 兵糧不足：兵士が減少
				int foodShortage = maintenanceCost - city.food;
				int troopLoss = foodShortage * 5;  // 不足分×5の兵士減少

				city.food = 0;
				city.troops = Max(0, city.troops - troopLoss);
				city.order = Max(0, city.order - 10);  // 治安も低下

				Print << U"[兵糧不足] " << city.name << U": 兵-" << troopLoss
					<< U", 治安-10";
			}
		}
	}

	// 勝利条件チェック
	static bool CheckVictory(const Array<CityData>& cities, const String& playerFactionName)
	{
		// 全都市がプレイヤーの支配下か？
		for (const auto& city : cities)
		{
			if (city.owner != playerFactionName)
			{
				return false;
			}
		}
		return true;
	}

	// 敗北条件チェック
	static bool CheckDefeat(const Array<CityData>& cities, const String& playerFactionName)
	{
		// プレイヤーが都市を1つも持っていないか？
		for (const auto& city : cities)
		{
			if (city.owner == playerFactionName)
			{
				return false;
			}
		}
		return true;
	}

	// 残存勢力数を取得
	static int GetRemainingFactionCount(const Array<CityData>& cities)
	{
		HashSet<String> factions;
		for (const auto& city : cities)
		{
			factions.insert(city.owner);
		}
		return static_cast<int>(factions.size());
	}

	// 勢力の都市数を取得
	static int GetFactionCityCount(const Array<CityData>& cities, const String& factionName)
	{
		int count = 0;
		for (const auto& city : cities)
		{
			if (city.owner == factionName)
			{
				count++;
			}
		}
		return count;
	}
};
