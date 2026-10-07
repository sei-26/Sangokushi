#pragma once
#include <array>
#include <cmath>
#include <vector>

// World-space grid. City indices refer to the scenario/save's city order.
// Ownership follows the city's current faction; levels persist after conquest.
class TerritoryMap
{
public:
	static constexpr int Columns = 38;
	static constexpr int Rows = 20;
	static constexpr int Count = Columns * Rows;
	static constexpr double CellWidth = 40.0;
	static constexpr double CellHeight = 41.0;
	struct CityPosition { double x, y; };
	struct Cell { int city = -1; int level = 0; };
	std::array<Cell, Count> cells{};

	void Initialize(const std::vector<CityPosition>& cities)
	{
		for (int i = 0; i < Count; ++i)
		{
			auto& cell = cells[i];
			cell = Cell{};
			double nearest = 350.0 * 350.0;
			for (int c = 0; c < static_cast<int>(cities.size()); ++c)
			{
				const double dx = CenterX(i) - cities[c].x, dy = CenterY(i) - cities[c].y;
				const double distance = dx * dx + dy * dy;
				if (distance < nearest) { nearest = distance; cell.city = c; }
			}
			if (cell.city >= 0 && nearest <= 90.0 * 90.0) cell.level = 1;
		}
	}
	static double CenterX(int i) { return (i % Columns + 0.5) * CellWidth; }
	static double CenterY(int i) { return (i / Columns + 0.5) * CellHeight; }
	static bool Valid(int i) { return i >= 0 && i < Count; }
	static int Index(double x, double y)
	{
		if (!std::isfinite(x) || !std::isfinite(y) || x < 0 || y < 0 || x >= Columns * CellWidth || y >= Rows * CellHeight) return -1;
		return static_cast<int>(y / CellHeight) * Columns + static_cast<int>(x / CellWidth);
	}
	static bool Farmland(int i) { return ((i % Columns) + (i / Columns)) % 3 != 0; }
	static int Gold(const Cell& cell, int i) { return cell.level * (Farmland(i) ? 4 : 12); }
	static int Food(const Cell& cell, int i) { return cell.level * (Farmland(i) ? 18 : 4); }
	bool CanClaim(int i) const
	{
		if (!Valid(i) || cells[i].city < 0 || cells[i].level != 0) return false;
		const int x = i % Columns, y = i / Columns;
		const auto neighbor = [&](int n) { return cells[n].city == cells[i].city && cells[n].level > 0; };
		return (x > 0 && neighbor(i - 1)) || (x + 1 < Columns && neighbor(i + 1))
			|| (y > 0 && neighbor(i - Columns)) || (y + 1 < Rows && neighbor(i + Columns));
	}
	bool Claim(int i) { if (!CanClaim(i)) return false; cells[i].level = 1; return true; }
	bool CanDevelop(int i) const { return Valid(i) && cells[i].city >= 0 && cells[i].level > 0 && cells[i].level < 3; }
	bool Develop(int i) { if (!CanDevelop(i)) return false; ++cells[i].level; return true; }
	int CityGold(int city) const
	{
		int result = 0;
		for (int i = 0; i < Count; ++i) if (cells[i].city == city) result += Gold(cells[i], i);
		return result;
	}
	int CityFood(int city) const
	{
		int result = 0;
		for (int i = 0; i < Count; ++i) if (cells[i].city == city) result += Food(cells[i], i);
		return result;
	}
};
