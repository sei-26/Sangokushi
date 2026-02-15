#pragma once
#include <Siv3D.hpp>

// 武将データ
struct Officer
{
	String name = U"無名";
	int leadership = 50;   // 統率力（戦闘時の兵士ボーナス）
	int power = 50;        // 武力（攻撃力）
	int war = 50;          // 武力（既存コード互換用）
	int intelligence = 50; // 知力（内政ボーナス）
	int politics = 50;     // 政治力（収入・治安ボーナス）
	int loyalty = 50;      // 忠誠度（0〜100）

	// 追加フィールド
	String lord = U"";     // 主君/所属勢力名（LoyaltyManager で使用）
	int compatibility = 50; // 相性（0〜100）

	// デフォルトコンストラクタ
	Officer() = default;

	// 戦闘力を計算（統率 + 武力）
	int GetCombatPower() const
	{
		return leadership + power;
	}

	// 内政力を計算（知力 + 政治）
	int GetAdministrationPower() const
	{
		return intelligence + politics;
	}

	// 忠誠度に応じたテキストを返す
	String GetLoyaltyText() const
	{
		if (loyalty >= 90) return U"非常に高い";
		if (loyalty >= 75) return U"高い";
		if (loyalty >= 50) return U"普通";
		if (loyalty >= 30) return U"やや低い";
		return U"低い";
	}

	// 忠誠度に応じた色を返す（描画で使用）
	Color GetLoyaltyColor() const
	{
		if (loyalty >= 75) return Palette::Lime;
		if (loyalty >= 50) return Palette::Yellow;
		if (loyalty >= 30) return Palette::Orange;
		return Palette::Red;
	}

	// 裏切りの可能性があるかを判定する
	// - 忠誠度が極端に低ければ常に裏切り判定対象にする
	// - 中間帯では確率的に裏切る。相性が低いほど確率を上げる
	bool MayBetray() const
	{
		// 忠誠度が非常に低い場合は高確率で裏切り対象
		if (loyalty <= 20)
		{
			return true;
		}

		// 基本確率を忠誠度帯に応じて設定
		double baseChance = 0.0;
		if (loyalty <= 40) baseChance = 0.15; // やや高め
		else if (loyalty <= 60) baseChance = 0.05; // 低め
		else baseChance = 0.01; // 忠誠度高めならほぼ裏切らない

		// 相性が低いほど裏切り確率を増幅（0..1 の係数）
		double compatFactor = (100 - static_cast<double>(compatibility)) / 100.0; // 0..1

		// 最終確率（上限調整）
		double chance = baseChance + compatFactor * 0.25; // 相性次第で最大+0.25
		if (chance > 0.95) chance = 0.95;

		// Siv3D の RandomBool(double) を使用して確率判定
		return RandomBool(chance);
	}
};
