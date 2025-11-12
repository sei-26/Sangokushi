#pragma once
#include <Siv3D.hpp>

struct Faction
{
	String name;
	ColorF color;
	Array<String> cities;
	Array<String> officers;

	Faction() = default;

	Faction(const String& n, const ColorF& c,
			const Array<String>& ci, const Array<String>& of)
		: name(n), color(c), cities(ci), officers(of) {
	}
};
