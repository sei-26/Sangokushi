#include "GameManager.hpp"

void GameManager::advanceMonth(Array<CityData>& cities)
{
	// 1. 維持費と兵糧
	for (auto& city : cities)
	{
		int upkeep = city.troops / 10;
		city.food -= upkeep;
		if (city.food < 0) {
			city.food = 0;
			city.troops -= 50;
			if (city.troops < 0) city.troops = 0;
		}

		int income = city.commerce / 2;
		if (city.order < 50) income /= 2;
		city.gold += income;

		// AI勢力の兵力増加（プレイヤー以外）
		if (city.owner != U"劉備")
		{
			city.troops += 500;
			if (city.troops > 10000) city.troops = 10000;
		}
	}

	// 2. 時間経過
	month++;
	if (month > 12) {
		month = 1;
		year++;
	}

	// 3. 7月の収穫
	if (month == 7)
	{
		for (auto& city : cities)
		{
			city.food += city.agriculture * 5;
		}
	}

	// 4. 戦争イベントのリセット
	pendingBattle = PendingBattle();

	// =================================================================
	// ★ 新システム：複数勢力の戦争システム
	// =================================================================

	// まず、すべての勢力をリストアップ
	HashSet<String> allFactions;
	for (const auto& city : cities)
	{
		allFactions.insert(city.owner);
	}

	// 各勢力について、戦争を起こすかチェック
	for (const String& faction : allFactions)
	{
		// この勢力の都市で、十分な兵力がある都市を探す
		for (int i = 0; i < cities.size(); ++i)
		{
			// 攻撃側の条件：
			// - この勢力に属する
			// - 兵士が2000以上いる
			if (cities[i].owner == faction && cities[i].troops >= 2000)
			{
				// 隣接する敵勢力の都市を探す（簡易版：全都市から選択）
				for (int j = 0; j < cities.size(); ++j)
				{
					// 防御側の条件：
					// - 異なる勢力
					if (cities[j].owner != faction)
					{
						// ★ 戦争発生確率の調整
						double warProbability = 0.0;

						if (cities[j].owner == U"劉備")
						{
							// プレイヤーへの攻撃：10%の確率（低め）
							warProbability = 0.10;
						}
						else
						{
							// AI同士の戦争：15%の確率
							warProbability = 0.15;
						}

						if (RandomBool(warProbability))
						{
							// ========================================
							// プレイヤーが関わる戦争
							// ========================================
							if (cities[i].owner == U"劉備" || cities[j].owner == U"劉備")
							{
								// プレイヤーが攻撃側または防御側
								pendingBattle.isOccurring = true;
								pendingBattle.atkCityIndex = i;
								pendingBattle.defCityIndex = j;

								Print << U"[戦争発生] " << cities[i].name << U"(" << cities[i].owner
									<< U") が " << cities[j].name << U"(" << cities[j].owner << U") に侵攻！";
								return; // プレイヤー関連の戦争が発生したら即座に処理
							}
							// ========================================
							// AI同士の戦争（自動解決）
							// ========================================
							else
							{
								Print << U"[AI戦争] " << cities[i].name << U"(" << cities[i].owner
									<< U") vs " << cities[j].name << U"(" << cities[j].owner << U")";

								// 簡易戦闘シミュレーション
								int atkPower = cities[i].troops;
								int defPower = cities[j].troops;

								// 防御側に少しボーナス
								defPower = static_cast<int>(defPower * 1.2);

								// 勝敗判定
								if (atkPower > defPower)
								{
									// 攻撃側の勝利
									Print << U"  → " << cities[i].owner << U" の勝利！ "
										<< cities[j].name << U" を占領！";

									// 都市を奪う
									cities[j].owner = cities[i].owner;
									cities[j].troops = 500; // 占領兵
									cities[j].order = Max(0, cities[j].order - 50);

									// 攻撃側も損害を受ける
									cities[i].troops = Max(500, cities[i].troops - defPower / 2);
								}
								else
								{
									// 防御側の勝利
									Print << U"  → " << cities[j].owner << U" の防衛成功！";

									// 両軍とも損害
									cities[i].troops = Max(300, cities[i].troops - defPower / 3);
									cities[j].troops = Max(500, cities[j].troops - atkPower / 3);
								}

								// AI同士の戦争は1回だけ（プレイヤーには関係ない）
								// 次の月に別の戦争が起きる可能性はある
								return;
							}
						}
					}
				}
			}
		}
	}
}
