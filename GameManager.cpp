#include "GameManager.hpp"
#include "AIController.hpp"
#include "SaveLoadManager.hpp"
#include "LoyaltyManager.hpp"
#include "EconomyManager.hpp"
#include "AudioManager.hpp"

void GameManager::advanceMonth(Array<CityData>& cities)
{
	EnsureTerritory(cities);
	month++;
	if (month > 12)
	{
		month = 1;
		year++;
	}

	// ★ 外交の月次処理
	diplomacy.AdvanceMonth();

	// ★ ターンをリセット
	turnManager.AdvanceMonth();

	// ★ 忠誠度更新
	LoyaltyManager::UpdateLoyalty(cities);

	// ★ 裏切りチェック
	auto betrayalLog = LoyaltyManager::CheckBetrayal(cities, career.officerMode ? career.officerName : U"");
	for (const auto& log : betrayalLog)
	{
		Print << U"[裏切り] " << log;
	}

	// ★ イベントログをクリア
	eventLog.clear();

	// ★ 史実イベントチェック
	if (auto historicalEvent = historicalEvents.CheckEvent(year, month))
	{
		auto* event = historicalEvent.value();
		historicalEvents.ApplyEvent(event, cities);

		eventLog.push_back(U"[史実] " + event->title);
		eventLog.push_back(event->description);
		Print << U"[史実イベント] " << event->title;
	}

	// ★ 季節イベントチェック
	if (auto seasonEvent = SeasonEventManager::CheckSeasonEvent(year, month, cities))
	{
		auto event = seasonEvent.value();
		eventLog.push_back(U"[季節] " + event.title);
		eventLog.push_back(event.description);
		for (const auto& effect : event.effects)
		{
			eventLog.push_back(U"  " + effect);
		}
		Print << U"[季節イベント] " << event.title;
	}

	// ★ 月次収入（商業・農業）
	EconomyManager::ApplyMonthlyIncome(cities);
	for (int i = 0; i < static_cast<int>(cities.size()); ++i)
	{
		cities[i].gold += territory.CityGold(i);
		cities[i].food += territory.CityFood(i);
	}

	// ★ 兵士維持費
	EconomyManager::ApplyTroopMaintenance(cities);



	// =================================================================
	// 🤖 AI勢力の自動内政
	// =================================================================
	for (auto& city : cities)
	{
		// ★ プレイヤー勢力の都市はスキップ
		if (city.owner == playerFactionName && career.CanManageCity(city.name))
		{
			continue;
		}

		AIController::ExecuteAdministration(city);
		const int cityIndex = static_cast<int>(&city - cities.data());
		for (int i = 0; i < TerritoryMap::Count; ++i)
			if (territory.cells[i].city == cityIndex && territory.CanClaim(i) && city.gold >= 100)
			{
				territory.Claim(i);
				city.gold -= 100;
				break;
			}
	}

	// =================================================================
	// ⚔️ AI勢力の自動戦争判断
	// =================================================================
	Array<String> battleLog;

	for (int i = 0; i < cities.size(); ++i)
	{
		CityData& aiCity = cities[i];

		// ★ プレイヤー勢力の都市はスキップ（手動で戦争）
		if (aiCity.owner == playerFactionName && career.CanManageCity(aiCity.name))
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

			// ★★★ 重要：プレイヤー都市への攻撃は pendingBattle にセットして防衛戦へ ★★★
			if (targetCity.owner == playerFactionName && (!career.officerMode || targetCity.name == career.cityName))
			{
				Print << U"[防衛戦] " << aiCity.owner << U"(" << aiCity.name << U") が "
					<< targetCity.name << U" に侵攻してきた！";

				// 防衛戦をセット
				pendingBattle.isOccurring = true;
				pendingBattle.atkCityIndex = i;           // 攻撃側 = AI都市
				pendingBattle.defCityIndex = targetIndex; // 防御側 = プレイヤー都市
				
				eventLog.push_back(U"⚠️ " + aiCity.owner + U" が " + targetCity.name + U" に侵攻！");
				
				// 防衛戦は1つずつ処理するため、ここで終了
				break;
			}
			else
			{
				// AI同士の戦闘は従来通りシミュレート
				Print << U"[AI戦争] " << aiCity.owner << U"(" << aiCity.name << U") が "
					<< targetCity.owner << U"(" << targetCity.name << U") を攻撃！";

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
	// Follow the protagonist if transferred or if their city changes hands.
	if (career.officerMode && !career.officerName.isEmpty())
	{
		for (const auto& city : cities)
			for (const auto& officer : city.officers)
				if (officer.name == career.officerName)
				{
					if (city.owner != playerFactionName || city.name != career.cityName)
					{
						career.governor = false;
						career.report = U"所属または配置が変わり、一般武将として再出発します。";
					}
					playerFactionName = city.owner;
					career.cityName = city.name;
				}
	}
	if (month % 3 == 1) SaveLoadManager::AutoSave(CreateSaveData(cities));
}
