#pragma once
#include <Siv3D.hpp>

struct CityData
{
	// ★ メンバ
	String name;
	Vec2 pos;
	Color color;

	int gold;
	int troops;
	int food;
	int order;

	int agriculture;
	int commerce;
	int barracks;

	String owner;

	// ★ デフォルトコンストラクタ（必須）
	CityData() = default;

	// ★ 通常コンストラクタ
	CityData(
		const String& name,
		const Vec2& pos,
		const Color& color,
		int gold, int troops, int food, int order,
		const String& owner,
		int agriculture = 1,
		int commerce = 1,
		int barracks = 1
	)
		: name(name)
		, pos(pos)
		, color(color)
		, gold(gold)
		, troops(troops)
		, food(food)
		, order(order)
		, agriculture(agriculture)
		, commerce(commerce)
		, barracks(barracks)
		, owner(owner)
	{
	}
};
