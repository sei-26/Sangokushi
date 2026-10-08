#pragma once
#include <cstdlib>
namespace battle
{
	enum class Stance
	{
		Balanced,
		Assault,
		Guard
	};
	inline int AttackPercent(Stance s)
	{
		return s == Stance::Assault ? 125 : s == Stance::Guard ? 75 : 100;
	}
	inline int DamagePercent(Stance s)
	{
		return s == Stance::Assault ? 120 : s == Stance::Guard ? 75 : 100;
	}
	inline const char32_t* Name(Stance s)
	{
		return s == Stance::Assault ? U"攻勢" : s == Stance::Guard ? U"固守" : U"均衡";
	}
	// Rasterize the direct ray. Endpoints do not block their own shot.
	template <class Blocked> bool ClearRay(int x, int y, int tx, int ty, Blocked blocked)
	{
		const int dx = std::abs(tx - x), dy = -std::abs(ty - y), sx = x < tx ? 1 : -1, sy = y < ty ? 1 : -1;
		int error = dx + dy;
		while (x != tx || y != ty)
		{
			const int twice = 2 * error;
			if (twice >= dy)
			{
				error += dy;
				x += sx;
			}
			if (twice <= dx)
			{
				error += dx;
				y += sy;
			}
			if (x == tx && y == ty)
				return true;
			if (blocked(x, y))
				return false;
		}
		return true;
	}
} // namespace battle
