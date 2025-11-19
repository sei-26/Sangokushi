#pragma once
#include <Siv3D.hpp>
#include "Officer.hpp"

struct CityData
{
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

	Array<Officer> officers;   // ← ★これが重要！各都市の武将リスト

	CityData() = default;

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
