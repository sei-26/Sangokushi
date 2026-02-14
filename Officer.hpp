#pragma once
#include <Siv3D.hpp>

// 武将データ
struct Officer
{
	String name = U"無名";
	int leadership = 50;  // 統率力（戦闘時の兵士ボーナス）
	int power = 50;       // 武力（攻撃力）
	int war = 50;         // 武力（既存コード互換用）
	int intelligence = 50; // 知力（内政ボーナス）
	int politics = 50;     // 政治力（収入・治安ボーナス）

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
};
