#pragma once
#include <algorithm>
#include <cmath>
#include <array>
#include <utility>

namespace hexgrid
{
	constexpr double Row = .8660254037844386, Radius = .5773502691896258;
	struct Cube
	{
		int q, r, s;
	};
	inline Cube CubeAt(int x, int y)
	{
		const int q = x - (y - (y & 1)) / 2;
		return {q, y, -q - y};
	}
	inline std::pair<int, int> Offset(double q, double r, double s)
	{
		int iq = static_cast<int>(std::round(q)), ir = static_cast<int>(std::round(r)),
		    is = static_cast<int>(std::round(s));
		const double dq = std::abs(iq - q), dr = std::abs(ir - r), ds = std::abs(is - s);
		if (dq > dr && dq > ds)
			iq = -ir - is;
		else if (dr > ds)
			ir = -iq - is;
		return {iq + (ir - (ir & 1)) / 2, ir};
	}
	inline int Distance(int x, int y, int tx, int ty)
	{
		const auto a = CubeAt(x, y), b = CubeAt(tx, ty);
		return std::max({std::abs(a.q - b.q), std::abs(a.r - b.r), std::abs(a.s - b.s)});
	}
	inline std::pair<double, double> Center(int x, int y)
	{
		return {x + .5 + (y & 1) * .5, y * Row + Radius};
	}
	inline std::pair<int, int> Pick(double x, double y)
	{
		const double r = (y - Radius) / Row, q = x - .5 - r / 2;
		return Offset(q, r, -q - r);
	}
	inline std::array<std::pair<int, int>, 6> Neighbors(int x, int y)
	{
		const int left = x - 1 + (y & 1), right = x + (y & 1);
		return {{{x - 1, y}, {x + 1, y}, {left, y - 1}, {right, y - 1}, {left, y + 1}, {right, y + 1}}};
	}
	template <class Blocked> bool ClearRay(int x, int y, int tx, int ty, Blocked blocked)
	{
		const auto a = CubeAt(x, y), b = CubeAt(tx, ty);
		const int n = Distance(x, y, tx, ty);
		for (int i = 1; i < n; ++i)
		{
			const double t = static_cast<double>(i) / n;
			const auto p = Offset(a.q + (b.q - a.q) * t + 1e-6, a.r + (b.r - a.r) * t + 1e-6,
			                      a.s + (b.s - a.s) * t - 2e-6);
			if (blocked(p.first, p.second))
				return false;
		}
		return true;
	}
} // namespace hexgrid
