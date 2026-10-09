#pragma once
#include <algorithm>
#include <cmath>
#include "HexGrid.hpp"
namespace world
{
	// All coordinates are world tiles or viewport-local pixels, independent of Siv3D.
	struct MapCamera
	{
		int width = 96, height = 64;
		double x = 48, y = 32, zoom = 1;
		bool hex = false, tilted = false;
		double Pitch() const
		{
			return tilted && hex ? .68 : 1.;
		}
		double ExtentX() const
		{
			return width + (hex ? .5 : 0);
		}
		double ExtentY() const
		{
			return hex ? (height - 1) * hexgrid::Row + 2 * hexgrid::Radius : height;
		}
		double Cell(double vw, double vh) const
		{
			return std::min(vw / ExtentX(), vh / (ExtentY() * Pitch())) * zoom;
		}
		void Clamp(double vw, double vh)
		{
			const double c = Cell(vw, vh), hx = std::min(ExtentX() / 2., vw / c / 2),
			             hy = std::min(ExtentY() / 2., vh / (c * Pitch()) / 2);
			x = std::clamp(x, hx, ExtentX() - hx);
			y = std::clamp(y, hy, ExtentY() - hy);
		}
		void Fit(int w, int h, bool useHex = false, bool useTilt = false)
		{
			hex = useHex;
			tilted = useTilt;
			width = w;
			height = h;
			x = ExtentX() / 2.;
			y = ExtentY() / 2.;
			zoom = 1;
		}
		void Pan(double dx, double dy, double vw, double vh)
		{
			double c = Cell(vw, vh);
			x -= dx / c;
			y -= dy / (c * Pitch());
			Clamp(vw, vh);
		}
		void Zoom(double factor, double mx, double my, double vw, double vh)
		{
			double c = Cell(vw, vh), tx = x + (mx - vw / 2) / c, ty = y + (my - vh / 2) / (c * Pitch());
			zoom = std::clamp(zoom * factor, 1., 6.);
			c = Cell(vw, vh);
			x = tx - (mx - vw / 2) / c;
			y = ty - (my - vh / 2) / (c * Pitch());
			Clamp(vw, vh);
		}
		int Tile(double mx, double my, double vw, double vh, int stride) const
		{
			if (mx < 0 || my < 0 || mx >= vw || my >= vh)
				return -1;
			double c = Cell(vw, vh);
			int tx = static_cast<int>(std::floor(x + (mx - vw / 2) / c)),
			    ty = static_cast<int>(std::floor(y + (my - vh / 2) / (c * Pitch())));
			if (hex)
			{
				const auto p = hexgrid::Pick(x + (mx - vw / 2) / c, y + (my - vh / 2) / (c * Pitch()));
				tx = p.first;
				ty = p.second;
			}
			return tx < 0 || ty < 0 || tx >= width || ty >= height ? -1 : ty * stride + tx;
		}
	};
} // namespace world
