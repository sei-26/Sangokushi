#pragma once
#include <Siv3D.hpp>

struct CityData
{
	String name;
	Vec2   pos;
	ColorF color;

	int gold;
	int troops;
	int food;
	int order;   // 治安(0-100)
	String ruler;

	CityData() = default;

	CityData(const String& n, const Vec2& p, const ColorF& c,
			 int g, int t, int f, int o, const String& r)
		: name(n), pos(p), color(c),
		gold(g), troops(t), food(f), order(o), ruler(r) {
	}
};
