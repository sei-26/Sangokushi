#pragma once
#include <Siv3D.hpp>

struct CityData
{
	// ★ メンバの順番（これに合わせて初期化順も揃える）
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
		, owner(owner)    // ★ 最後に owner を確実にコピー！
	{
	}
};
