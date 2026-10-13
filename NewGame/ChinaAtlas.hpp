#pragma once
#include "WorldLayout.hpp"
#include "HexGrid.hpp"
#include <algorithm>
#include <array>
#include <cmath>
namespace chinaatlas
{
	struct Point
	{
		double x, y;
	};
	// Presentation anchors for the existing stable city IDs, not a historical scenario.
	inline constexpr std::array<Point, 30> Cities{
	    {{.22, .51}, {.33, .39}, {.54, .41}, {.59, .31}, {.55, .28}, {.62, .22}, {.78, .47}, {.65, .54},
	     {.54, .50}, {.25, .46}, {.32, .54}, {.42, .50}, {.26, .73}, {.17, .77}, {.41, .29}, {.29, .27},
	     {.18, .08}, {.36, .21}, {.53, .18}, {.77, .09}, {.70, .23}, {.88, .25}, {.74, .35}, {.61, .38},
	     {.52, .44}, {.58, .61}, {.48, .61}, {.73, .43}, {.87, .55}, {.34, .94}}};
	inline Point Project(double x, double y)
	{
		const Point base{(x - 5) / 85., (y - 1) / 61.};
		double sum = 0, dx = 0, dy = 0;
		const auto& sites = world::Sites();
		for (size_t i = 0; i < Cities.size(); ++i)
		{
			const double sx = x - sites[i].x, sy = y - sites[i].y, d = sx * sx + sy * sy;
			if (d < 1e-8)
				return Cities[i];
			const double w = 1 / (d * d);
			sum += w;
			dx += w * (Cities[i].x - (sites[i].x - 5) / 85.);
			dy += w * (Cities[i].y - (sites[i].y - 1) / 61.);
		}
		return {std::clamp(base.x + dx / sum, 0., 1.), std::clamp(base.y + dy / sum, 0., 1.)};
	}
} // namespace chinaatlas
