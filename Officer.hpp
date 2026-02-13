#pragma once
#include <Siv3D.hpp>

// 武将データ
struct Officer
{
	String name = U"無名";
	int leadership = 50;  // 統率力
	int power = 50;       // 武力
	int war = 50;         // 武力（既存コード互換用）

	// デフォルトコンストラクタ
	Officer() = default;

	// 4引数コンストラクタ（名前、統率力、武力、war）
	Officer(const String& n, int leadership_, int power_, int war_)
		: name(n)
		, leadership(leadership_)
		, power(power_)
		, war(war_)
	{}

	// 3引数コンストラクタ（war を省略した場合は power と同じ値を設定）
	Officer(const String& n, int leadership_, int power_)
		: name(n)
		, leadership(leadership_)
		, power(power_)
		, war(power_)
	{}
};
