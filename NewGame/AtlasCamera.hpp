#pragma once
#include <algorithm>
namespace chinaatlas
{
	struct Camera
	{
		double x = .5, y = .5, zoom = 1;
		void Reset()
		{
			x = y = .5;
			zoom = 1;
		}
		void Clamp()
		{
			const double half = .5 / zoom;
			x = std::clamp(x, half, 1 - half);
			y = std::clamp(y, half, 1 - half);
		}
		void Zoom(double factor, double mx, double my, double width, double height)
		{
			const double px = x + (mx - width * .5) / (width * zoom),
			             py = y + (my - height * .5) / (height * zoom);
			zoom = std::clamp(zoom * factor, 1., 6.);
			x = px - (mx - width * .5) / (width * zoom);
			y = py - (my - height * .5) / (height * zoom);
			Clamp();
		}
		void Pan(double dx, double dy, double width, double height)
		{
			x -= dx / (width * zoom);
			y -= dy / (height * zoom);
			Clamp();
		}
	};
} // namespace chinaatlas
