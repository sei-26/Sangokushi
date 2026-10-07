#pragma once
#include <array>
#include <cmath>

namespace world
{
	constexpr int Width = 96, Height = 64;
	enum class Terrain
	{
		Plain,
		Forest,
		Mountain,
		River,
		Bridge,
		Sea
	};
	struct Site
	{
		const char32_t* name;
		int x, y, faction;
	};
	// A China-inspired strategic geography; positions and initial ownership are fictional.
	inline const std::array<Site, 30>& Sites()
	{
		static const std::array<Site, 30> sites{
		    {{U"成都", 20, 43, 0}, {U"漢中", 29, 30, 0}, {U"新野", 49, 35, 0}, {U"許昌", 60, 26, 1},
		     {U"洛陽", 49, 23, 1}, {U"鄴", 66, 13, 1},   {U"建業", 79, 43, 2}, {U"柴桑", 65, 47, 2},
		     {U"江陵", 50, 43, 2}, {U"梓潼", 22, 37, 0}, {U"江州", 30, 46, 0}, {U"永安", 39, 44, 0},
		     {U"南中", 21, 55, 0}, {U"雲南", 12, 57, 0}, {U"長安", 35, 24, 1}, {U"天水", 23, 25, 1},
		     {U"武威", 13, 16, 1}, {U"安定", 28, 19, 1}, {U"晋陽", 52, 13, 1}, {U"北平", 77, 7, 1},
		     {U"平原", 74, 18, 1}, {U"北海", 82, 24, 1}, {U"下邳", 76, 32, 1}, {U"汝南", 61, 33, 1},
		     {U"襄陽", 49, 38, 0}, {U"長沙", 54, 51, 2}, {U"武陵", 43, 51, 2}, {U"廬江", 69, 39, 2},
		     {U"会稽", 82, 49, 2}, {U"交趾", 47, 59, 2}}};
		return sites;
	}
	inline std::array<Terrain, Width * Height> Ground()
	{
		std::array<Terrain, Width * Height> out{};
		for (int y = 0; y < Height; ++y)
			for (int x = 0; x < Width; ++x)
			{
				auto& t = out[y * Width + x];
				int coast = y < 18 ? 88 : y < 36 ? 86 : 94 - (y - 36) / 2;
				if (x >= coast)
				{
					t = Terrain::Sea;
					continue;
				}
				const bool qinling =
				    y >= 29 && y <= 31 && x >= 17 && x <= 43 && x != 29 && x != 30 && x != 39;
				const bool taihang = x >= 56 && x <= 58 && y >= 8 && y <= 24 && y != 15 && y != 16;
				const bool western = x < 10 || (x < 18 && y > 34) || (x > 12 && x < 20 && y > 21 && y < 32);
				if (qinling || taihang || western)
					t = Terrain::Mountain;
				else if ((x * 7 + y * 11) % 17 < 4 || (y > 50 && (x + y) % 3 == 0))
					t = Terrain::Forest;
			}
		// Connected river bends, with deliberately spaced crossings.
		for (int x = 17; x < 86; ++x)
		{
			int y = static_cast<int>(21 + 4 * std::sin((x - 17) * .085));
			for (int k = 0; k < 2; ++k)
				if (out[(y + k) * Width + x] != Terrain::Sea)
					out[(y + k) * Width + x] = Terrain::River;
			if (x == 29 || x == 43 || x == 57 || x == 72 || x == 81)
				for (int k = 0; k < 2; ++k)
					out[(y + k) * Width + x] = Terrain::Bridge;
		}
		for (int x = 26; x < 88; ++x)
		{
			int y = static_cast<int>(42 + 3 * std::sin((x - 26) * .10));
			for (int k = 0; k < 2; ++k)
				if (out[(y + k) * Width + x] != Terrain::Sea)
					out[(y + k) * Width + x] = Terrain::River;
			if (x == 35 || x == 47 || x == 59 || x == 73 || x == 83)
				for (int k = 0; k < 2; ++k)
					out[(y + k) * Width + x] = Terrain::Bridge;
		}
		for (const auto& c : Sites())
			out[c.y * Width + c.x] = Terrain::Plain;
		return out;
	}
} // namespace world
