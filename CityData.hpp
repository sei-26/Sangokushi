#pragma once
#include <Siv3D.hpp>
#include "Officer.hpp" // 武将クラスがある場合

struct CityData
{
	String name;
	Point pos;
	String owner;       // 現在の所有勢力
	String initialOwner; // 初期所有者

	// ---------------------------------------------------
	// パラメータ
	// ---------------------------------------------------
	int gold = 1000;
	int food = 1000;
	int troops = 0;
	int order = 50;

	// ★ エラー修正：ここが足りていませんでした！
	int agriculture = 200; // 農業値
	int commerce = 200;    // 商業値
	int barracks = 200;    // 兵舎/技術値

	Color color = Palette::White;

	Array<Officer> officers;

	// ---------------------------------------------------
	// コンストラクタ
	// ---------------------------------------------------
	// 名前・座標・所有者だけで作る便利な機能
	CityData(String n, Point p, String o)
		: name(n)
		, pos(p)
		, owner(o)
		, initialOwner(o)
	{
		// 他の数値は初期値(200など)が自動で入ります
	}

	// デフォルトコンストラクタ
	CityData() = default;
};
