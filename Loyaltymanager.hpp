#pragma once
#include <Siv3D.hpp>
#include "Officer.hpp"
#include "CityData.hpp"

// 忠誠度管理システム
class LoyaltyManager
{
public:
	// 忠誠度を月次更新
	static void UpdateLoyalty(Array<CityData>& cities)
	{
		for (auto& city : cities)
		{
			for (auto& officer : city.officers)
			{
				// 主君を設定
				if (officer.lord.isEmpty())
				{
					officer.lord = city.owner;
				}

				// 相性による忠誠度変動
				if (officer.compatibility >= 80)
				{
					officer.loyalty = Min(100, officer.loyalty + 2);  // 相性良好で+2
				}
				else if (officer.compatibility <= 20)
				{
					officer.loyalty = Max(0, officer.loyalty - 2);  // 相性最悪で-2
				}
				else
				{
					// 普通の相性なら徐々に中立(70)に戻る
					if (officer.loyalty < 70)
					{
						officer.loyalty = Min(70, officer.loyalty + 1);
					}
					else if (officer.loyalty > 70)
					{
						officer.loyalty = Max(70, officer.loyalty - 1);
					}
				}
			}
		}
	}

	// 裏切りチェック
	static Array<String> CheckBetrayal(Array<CityData>& cities)
	{
		Array<String> betrayalLog;

		for (auto& city : cities)
		{
			Array<Officer> remainingOfficers;

			for (auto& officer : city.officers)
			{
				if (officer.MayBetray() && RandomBool(0.3))  // 30%の確率で裏切り
				{
					betrayalLog.push_back(officer.name + U" が " + city.owner + U" を裏切りました！");

					// 他の勢力にランダムに移動（簡易版）
					// 実装は後で改善
				}
				else
				{
					remainingOfficers.push_back(officer);
				}
			}

			city.officers = remainingOfficers;
		}

		return betrayalLog;
	}

	// 武将登用（金を使って登用）
	static bool RecruitOfficer(Officer& officer, const String& newLord, int offerGold)
	{
		// 登用成功率 = (金額 / 1000) + (100 - 忠誠度) / 2
		int baseChance = offerGold / 10;  // 金1000で100%
		int loyaltyPenalty = (100 - officer.loyalty) / 2;
		int successRate = Min(95, baseChance + loyaltyPenalty);

		if (Random(0, 99) < successRate)
		{
			// 登用成功
			officer.lord = newLord;
			officer.loyalty = 50 + Random(-10, 10);  // 初期忠誠度40〜60
			officer.compatibility = Random(30, 70);  // 相性は低め
			return true;
		}

		return false;
	}

	// 忠誠度を上げる（金を使って）
	static void RaiseLoyalty(Officer& officer, int goldAmount)
	{
		int increase = goldAmount / 100;  // 金100で+1
		officer.loyalty = Min(100, officer.loyalty + increase);
	}

	// 相性を計算（名前のハッシュから）
	static int CalculateCompatibility(const String& officer1, const String& officer2)
	{
		uint32 hash1 = officer1.hash();
		uint32 hash2 = officer2.hash();

		// ハッシュ値の差から相性を計算
		int diff = static_cast<int>(Abs(static_cast<int64>(hash1) - static_cast<int64>(hash2)));
		return 30 + (diff % 40);  // 30〜69の範囲
	}

	// 初期化時に相性を設定
	static void InitializeCompatibility(Array<CityData>& cities)
	{
		for (auto& city : cities)
		{
			for (auto& officer : city.officers)
			{
				if (officer.lord.isEmpty())
				{
					officer.lord = city.owner;
				}

				// 相性を計算
				officer.compatibility = CalculateCompatibility(officer.name, officer.lord);

				// 初期忠誠度を相性ベースで設定
				if (officer.loyalty == 100)  // まだ設定されていない場合
				{
					officer.loyalty = 70 + officer.compatibility / 3;  // 80〜93程度
					officer.loyalty = Min(100, officer.loyalty);
				}
			}
		}
	}
};
