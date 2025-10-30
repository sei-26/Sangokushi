#pragma once
#include <Siv3D.hpp>

struct CityData
{
	String name;
	Vec2 pos;
	Color color;
	int gold = 500;
	int food = 400;
	int troops = 100;
	String ruler = U"不明";
};
