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
}
