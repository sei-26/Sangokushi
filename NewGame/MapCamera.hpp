#pragma once
#include <algorithm>
#include <cmath>
namespace world
{
	// All coordinates are world tiles or viewport-local pixels, independent of Siv3D.
	struct MapCamera
	{
		int width = 96, height = 64;
		double x = 48, y = 32, zoom = 1;
		double Cell(double vw, double vh) const
		{
			return std::min(vw / width, vh / height) * zoom;
		}
		void Clamp(double vw, double vh)
		{
			const double c = Cell(vw, vh), hx = std::min(width / 2., vw / c / 2),
			             hy = std::min(height / 2., vh / c / 2);
			x = std::clamp(x, hx, width - hx);
			y = std::clamp(y, hy, height - hy);
		}
		void Fit(int w, int h)
		{
			width = w;
			height = h;
			x = w / 2.;
			y = h / 2.;
			zoom = 1;
		}
		void Pan(double dx, double dy, double vw, double vh)
		{
			double c = Cell(vw, vh);
			x -= dx / c;
			y -= dy / c;
			Clamp(vw, vh);
		}
		void Zoom(double factor, double mx, double my, double vw, double vh)
		{
			double c = Cell(vw, vh), tx = x + (mx - vw / 2) / c, ty = y + (my - vh / 2) / c;
			zoom = std::clamp(zoom * factor, 1., 6.);
			c = Cell(vw, vh);
			x = tx - (mx - vw / 2) / c;
			y = ty - (my - vh / 2) / c;
			Clamp(vw, vh);
		}
		int Tile(double mx, double my, double vw, double vh, int stride) const
		{
			if (mx < 0 || my < 0 || mx >= vw || my >= vh)
				return -1;
			double c = Cell(vw, vh);
			int tx = static_cast<int>(std::floor(x + (mx - vw / 2) / c)),
			    ty = static_cast<int>(std::floor(y + (my - vh / 2) / c));
			return tx < 0 || ty < 0 || tx >= width || ty >= height ? -1 : ty * stride + tx;
		}
	};
} // namespace world
