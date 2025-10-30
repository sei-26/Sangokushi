#pragma once
#include <Siv3D.hpp>

class Tile
{
public:
	enum class Terrain { Plain, Forest, Mountain, River, Castle };

private:
	Terrain m_terrain = Terrain::Plain;
	bool m_occupied = false;

public:
	void setTerrain(Terrain t) { m_terrain = t; }
	void setOccupied(bool occ) { m_occupied = occ; }
	bool occupied() const noexcept { return m_occupied; }
	Terrain terrain() const noexcept { return m_terrain; }

	double defenseBonus() const noexcept
	{
		switch (m_terrain)
		{
		case Terrain::Plain:    return 1.0;
		case Terrain::Forest:   return 1.2;
		case Terrain::Mountain: return 1.5;
		case Terrain::River:    return 0.8;
		case Terrain::Castle:   return 1.8;
		default: return 1.0;
		}
	}

	bool isPassable() const noexcept
	{
		return (!m_occupied) && (m_terrain != Terrain::River);
	}

	void Draw(int x, int y, int size = 32) const
	{
		Color color = Palette::Green;
		switch (m_terrain)
		{
		case Terrain::Plain:    color = Color(90, 160, 80); break;
		case Terrain::Forest:   color = Color(30, 100, 30); break;
		case Terrain::Mountain: color = Color(120, 120, 120); break;
		case Terrain::River:    color = Color(50, 100, 255); break;
		case Terrain::Castle:   color = Color(200, 100, 50); break;
		}

		RectF(x * size, y * size, size, size).draw(color);
		if (m_occupied)
			RectF(x * size, y * size, size, size).draw(Color(255, 0, 0, 64));
	}
};
