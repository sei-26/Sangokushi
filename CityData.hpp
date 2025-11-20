#pragma once
#include <Siv3D.hpp>
#include "Officer.hpp"

struct CityData
{
	String name = U"";
	Vec2 pos = Vec2{ 0,0 };
	Color color = Palette::White;

	int gold = 0;
	int troops = 0;
	int food = 0;
	int order = 0;

	int agriculture = 1;
	int commerce = 1;
	int barracks = 1;

	String owner = U"";
	String initialOwner;
	Array<Officer> officers{};

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
		, initialOwner(owner)
	{
	}
};
