#pragma once
#include <Siv3D.hpp>

class Tile
{
public:
	enum class Terrain { Plain, Mountain, River };
	Tile::Terrain terrain() const noexcept { return m_terrain; }
	bool isPassable() const noexcept { return(!m_occupied) && (m_terrain != Terrain::River); }

private:
	Terrain m_terrain = Terrain::Plain;
	bool m_occupied = false;

public:
	void setTerrain(Terrain t) { m_terrain = t; }
	void setOccupied(bool occ) { m_occupied = occ; }
	bool occupied() const noexcept { return m_occupied; }

	void Draw(int x, int y, int size = 32) const
	{
		Color color;
		switch (m_terrain)
		{
		case Terrain::Plain: color = Palette::Green; break;
		case Terrain::Mountain: color = Palette::Gray; break;
		case Terrain::River: color = Palette::Blue; break;
		}

		RectF(x * size, y * size, size, size).draw(color);

		if (m_occupied)
		{
			RectF(x * size, y * size, size, size).draw(Color(255, 0, 0, 128));
		}
	}
};
