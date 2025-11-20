#pragma once
#include <Siv3D.hpp>
#include "CityData.hpp"
#include "Officer.hpp"

struct Faction
{
	String name;                // 勢力名（劉備・曹操など）
	Color color;                // 勢力色
	Array<CityData> cities;     // 所有都市（後で使う）
	Array<Officer> officers;    // 所属武将
	String capitalName;


	Faction() = default;

	Faction(const String& n, const Color& c)
		: name(n), color(c)
	{
	}
};
