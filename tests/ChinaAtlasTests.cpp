#include "../NewGame/ChinaAtlas.hpp"
#include "../NewGame/AtlasCamera.hpp"
#include <cassert>
#include <iostream>
int main()
{
	const auto& sites = world::Sites();
	for (size_t i = 0; i < sites.size(); ++i)
	{
		const auto p = chinaatlas::Project(sites[i].x, sites[i].y);
		assert(std::abs(p.x - chinaatlas::Cities[i].x) < 1e-10);
		assert(std::abs(p.y - chinaatlas::Cities[i].y) < 1e-10);
	}
	for (int y = 0; y < world::Height; ++y)
		for (int x = 0; x < world::Width; ++x)
		{
			const auto p = chinaatlas::Project(x, y);
			assert(std::isfinite(p.x) && std::isfinite(p.y));
			assert(p.x >= 0 && p.x <= 1 && p.y >= 0 && p.y <= 1);
			if (x + 1 < world::Width)
			{
				const auto next = chinaatlas::Project(x + 1, y);
				assert(std::hypot(p.x - next.x, p.y - next.y) < .12);
			}
		}
	chinaatlas::Camera camera;
	const double width = 520, height = 650, mx = width * .4, my = height * .4;
	const double beforeX = camera.x + (mx - width * .5) / (width * camera.zoom);
	const double beforeY = camera.y + (my - height * .5) / (height * camera.zoom);
	camera.Zoom(2, mx, my, width, height);
	assert(std::abs(beforeX - (camera.x + (mx - width * .5) / (width * camera.zoom))) < 1e-10);
	assert(std::abs(beforeY - (camera.y + (my - height * .5) / (height * camera.zoom))) < 1e-10);
	camera.Pan(100000, -100000, width, height);
	assert(camera.x == .5 / camera.zoom && camera.y == 1 - .5 / camera.zoom);
	camera.Zoom(100, width * .5, height * .5, width, height);
	assert(camera.zoom == 6);
	camera.Zoom(.0001, width * .5, height * .5, width, height);
	assert(camera.zoom == 1 && camera.x == .5 && camera.y == .5);
	camera.Zoom(3, width * .5, height * .5, width, height);
	camera.Reset();
	assert(camera.zoom == 1 && camera.x == .5 && camera.y == .5);
	std::cout << "China atlas projection, cursor zoom, pan limits and reset passed\n";
}
