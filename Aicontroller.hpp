#pragma once
#include <Siv3D.hpp>
#include "CityData.hpp"
#include "Faction.hpp"

// AI勢力の行動管理
class AIController
{
public:
	// AI勢力の内政を実行
	static void ExecuteAdministration(CityData& city)
	{
		// 武将ボーナス計算
		int administrationBonus = 0;
		if (!city.officers.isEmpty())
		{
			const Officer& officer = city.officers[0];
			administrationBonus = officer.GetAdministrationPower() / 10;
		}

		// 優先順位1: 徴兵（兵力が少ない場合）
		if (city.troops < 5000 && city.food >= 100)
		{
			city.food -= 100;
			city.troops += 200;
			return;
		}

		// 優先順位2: 治安（50未満なら）
		if (city.order < 50 && city.gold >= 50)
		{
			city.gold -= 50;
			int orderBonus = 5 + (administrationBonus / 2);
			city.order += orderBonus;
			if (city.order > 100) city.order = 100;
			return;
		}

		// 優先順位3: 農業開発（農業値が低い場合）
		if (city.agriculture < 100 && city.gold >= 100)
		{
			city.gold -= 100;
			city.agriculture += 10 + administrationBonus;
			return;
		}

		// 優先順位4: 商業開発（金が少ない場合）
		if (city.gold < 2000 && city.gold >= 100)
		{
			city.gold -= 100;
			city.commerce += 10 + administrationBonus;
			return;
		}

		// それ以外は徴兵
		if (city.food >= 100)
		{
			city.food -= 100;
			city.troops += 200;
		}
	}

	// AI勢力の戦争判断
	// 戻り値: 攻撃対象の都市インデックス（-1なら攻撃しない）
	static int DecideAttackTarget(
		const Array<CityData>& allCities,
		int aiCityIndex,
		const Faction& playerFaction)
	{
		const CityData& aiCity = allCities[aiCityIndex];

		// ★ 改善1：兵力が十分にある場合のみ攻撃（より慎重に）
		if (aiCity.troops < 3500)  // 2000 → 3500 に引き上げ
		{
			return -1;
		}

		// ★ 改善2：金が少ない場合は内政優先（戦争回避）
		if (aiCity.gold < 800)
		{
			return -1;
		}

		// ★ 改善3：治安が低い場合は攻撃しない（内政優先）
		if (aiCity.order < 40)
		{
			return -1;
		}

		// 隣接する敵都市を探す
		Array<int> enemyCityIndices;
		for (int i = 0; i < allCities.size(); ++i)
		{
			if (i == aiCityIndex) continue;

			const CityData& targetCity = allCities[i];

			// 同じ勢力はスキップ
			if (targetCity.owner == aiCity.owner) continue;

			// 距離計算（簡易版：座標の距離）
			double distance = aiCity.pos.distanceFrom(targetCity.pos);

			// ★ 改善4：攻撃範囲を狭める（より近い都市のみ）
			if (distance < 220)  // 300 → 220 に縮小
			{
				enemyCityIndices.push_back(i);
			}
		}

		if (enemyCityIndices.isEmpty())
		{
			return -1;
		}

		// 最も兵力差がある都市を選ぶ
		int bestTarget = -1;
		double bestRatio = 0.0;

		for (int targetIndex : enemyCityIndices)
		{
			const CityData& targetCity = allCities[targetIndex];

			// 兵力比を計算
			double ratio = static_cast<double>(aiCity.troops) / Max(targetCity.troops, 1);

			// ★ 改善5：より大きな兵力差が必要（1.5倍 → 2.0倍）
			if (ratio >= 2.0 && ratio > bestRatio)
			{
				bestRatio = ratio;
				bestTarget = targetIndex;
			}
		}

		// ★ 改善6：攻撃確率を大幅に下げる（70% → 25%）
		// さらに、プレイヤーへの攻撃はより慎重に（15%）
		double attackChance = 0.25;
		
		if (bestTarget >= 0)
		{
			const CityData& targetCity = allCities[bestTarget];
			
			// プレイヤーへの攻撃はより慎重に
			if (targetCity.owner == playerFaction.name)
			{
				attackChance = 0.15;  // プレイヤーへは15%の確率
				
				// さらに、兵力比が2.5倍以上ないと攻めない
				if (bestRatio < 2.5)
				{
					return -1;
				}
			}
			
			if (RandomBool(attackChance))
			{
				return bestTarget;
			}
		}

		return -1;
	}

	// AI同士の戦闘を簡易シミュレート
	static bool SimulateBattle(CityData& attacker, CityData& defender)
	{
		// 攻撃側の戦力
		int attackerPower = attacker.troops;
		if (!attacker.officers.isEmpty())
		{
			attackerPower += attacker.officers[0].GetCombatPower() * 10;
		}

		// 防御側の戦力（防御ボーナス+20%）
		int defenderPower = static_cast<int>(defender.troops * 1.2);
		if (!defender.officers.isEmpty())
		{
			defenderPower += defender.officers[0].GetCombatPower() * 10;
		}

		// 戦力比で勝敗判定
		bool attackerWins = (attackerPower > defenderPower);

		if (attackerWins)
		{
			// 攻撃側の勝利
			int attackerLoss = static_cast<int>(attacker.troops * 0.3);
			int defenderLoss = defender.troops;

			attacker.troops -= attackerLoss;
			defender.troops = 0;

			// 都市を奪取
			defender.owner = attacker.owner;
			defender.troops = attacker.troops / 2;  // 守備隊を残す
			attacker.troops -= defender.troops;

			// 武将も移動
			if (!attacker.officers.isEmpty())
			{
				defender.officers.clear();
				defender.officers.push_back(attacker.officers[0]);
			}

			return true;
		}
		else
		{
			// 防御側の勝利
			int attackerLoss = static_cast<int>(attacker.troops * 0.6);
			int defenderLoss = static_cast<int>(defender.troops * 0.2);

			attacker.troops -= attackerLoss;
			defender.troops -= defenderLoss;

			return false;
		}
	}
};
