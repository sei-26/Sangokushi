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

		if (city.owner != U"劉備") // プレイヤー以外
		{
			city.troops += 500; // 毎月500人増える！
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
	pendingBattle = PendingBattle();

	//全都市チェック
	for (int i = 0; i < cities.size(); ++i)
	{
		// 「敵の都市」で、かつ「兵士が3000以上」なら攻撃チャンス
		if (cities[i].owner != U"劉備" && cities[i].troops >= 1000)
		{
			// 攻撃対象（プレイヤーの都市）を探す
			for (int j = 0; j < cities.size(); ++j)
			{
				// プレイヤーの都市を見つけたら、30%の確率で攻撃！
				if (cities[j].owner == U"劉備")
				{
					if (RandomBool(0.3))
					{
						// 戦争発生！データを保存してループを抜ける
						pendingBattle.isOccurring = true;
						pendingBattle.atkCityIndex = i; // 敵（攻め）
						pendingBattle.defCityIndex = j; // 自分（守り）

						// コンソールにログを出す
						Console << U"敵襲！ " << cities[i].name << U" が " << cities[j].name << U" に侵攻！";
						return; // 今月は1回だけ戦争
					}
				}
			}
		}
	}
}
