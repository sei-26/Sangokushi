#include "GameManager.hpp"
#include "AIController.hpp"
#include "SaveLoadManager.hpp"
#include "LoyaltyManager.hpp"

void GameManager::advanceMonth(Array<CityData>& cities)
{
	month++;
	if (month > 12)
	{
		month = 1;
		year++;
	}

	// ★ オートセーブ（3ヶ月ごと）
	if (month % 3 == 1)
	{
		SaveData autoSaveData = CreateSaveData(cities);
		SaveLoadManager::AutoSave(autoSaveData);
	}

	// ★ 外交の月次処理
	diplomacy.AdvanceMonth();

	// ★ ターンをリセット
	turnManager.AdvanceMonth();

	// ★ 忠誠度更新
	LoyaltyManager::UpdateLoyalty(cities);

	// ★ 裏切りチェック
	auto betrayalLog = LoyaltyManager::CheckBetrayal(cities);
	for (const auto& log : betrayalLog)
	{
		Print << U"[裏切り] " << log;
	}

	// =================================================================
	// 🌾 各都市の収入・収穫処理
	// =================================================================
	for (auto& city : cities)
	{
		// 金の収入（商業値に応じて）
		int income = city.commerce / 10;
		city.gold += income;

		// 兵糧の収穫（農業値に応じて）
		int harvest = city.agriculture / 10;
		city.food += harvest;

		// 兵士の維持費（兵糧消費）
		int upkeep = city.troops / 100;
		city.food -= upkeep;

		// 兵糧が足りない場合は兵士が減る
		if (city.food < 0)
		{
			int loss = Min(city.troops / 10, -city.food * 10);
			city.troops -= loss;
			city.food = 0;
		}
	}

	// =================================================================
	// 🤖 AI勢力の自動内政
	// =================================================================
	for (auto& city : cities)
	{
		// ★ プレイヤー勢力の都市はスキップ
		if (city.owner == playerFactionName)
		{
			continue;
		}

		AIController::ExecuteAdministration(city);
	}

	// =================================================================
	// ⚔️ AI勢力の自動戦争判断
	// =================================================================
	Array<String> battleLog;

	for (int i = 0; i < cities.size(); ++i)
	{
		CityData& aiCity = cities[i];

		// ★ プレイヤー勢力の都市はスキップ（手動で戦争）
		if (aiCity.owner == playerFactionName)
		{
			continue;
		}

		// 攻撃対象を決定
		Faction dummyPlayer;
		dummyPlayer.name = playerFactionName;
		int targetIndex = AIController::DecideAttackTarget(cities, i, dummyPlayer);

		if (targetIndex >= 0)
		{
			CityData& targetCity = cities[targetIndex];

			Print << U"[AI戦争] " << aiCity.owner << U"(" << aiCity.name << U") が "
				<< targetCity.owner << U"(" << targetCity.name << U") を攻撃！";

			// 戦闘シミュレート
			bool attackerWins = AIController::SimulateBattle(aiCity, targetCity);

			if (attackerWins)
			{
				Print << U"  → " << aiCity.owner << U" の勝利！ "
					<< targetCity.name << U" を占領！";
				battleLog.push_back(aiCity.owner + U" が " + targetCity.name + U" を占領！");
			}
			else
			{
				Print << U"  → " << targetCity.owner << U" が防衛成功！";
				battleLog.push_back(targetCity.owner + U" が " + aiCity.owner + U" の攻撃を撃退！");
			}
		}
	}

	// =================================================================
	// 📰 戦争ログを表示（後でUIに表示）
	// =================================================================
	if (!battleLog.isEmpty())
	{
		Print << U"";
		Print << U"【今月の戦況】";
		for (const auto& log : battleLog)
		{
			Print << U"  " << log;
		}
	}
}
