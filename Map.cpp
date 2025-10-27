#include "Map.hpp"


Map::Map(int w, int h, int tileSize)
	: width(w), height(h), tileSize(tileSize), m_tiles(w* h)
{
	GenerateSimple();
}

void Map::GenerateSimple()
{
	std::mt19937_64 rng(RandomUint64());
	std::uniform_int_distribution<int> dist(0, 99);

	for (int y = 0; y < height; ++y)
	{
		for (int x = 0; x < width; ++x)
		{
			int idx = Index(x, y);
			int r = dist(rng);

			if (r < 70) m_tiles[idx].setTerrain(Tile::Terrain::Plain);
			else if (r < 90) m_tiles[idx].setTerrain(Tile::Terrain::Mountain);
			else m_tiles[idx].setTerrain(Tile::Terrain::River);

			m_tiles[idx].setOccupied(false);
		}
	}
}

void Map::FitToScreen(int screenW, int screenH, int uiHeight)
{
	const int usableH = std::max(0, screenH - uiHeight);
	int tsW = screenW / std::max(1, width);
	int tsH = (usableH > 0) ? (usableH / std::max(1, height)) : (screenH / std::max(1, height));
	tileSize = std::max(1, std::min(tsW, tsH));
}

bool Map::InBounds(int x, int y) const noexcept
{
	return (0 <= x && x < width) && (0 <= y && y < height);
}

Tile& Map::At(int x, int y)
{
	assert(InBounds(x, y));
	return m_tiles[Index(x, y)];
}

const Tile& Map::At(int x, int y) const
{
	assert(InBounds(x, y));
	return m_tiles[Index(x, y)];
}

void Map::Draw() const
{
	for (int y = 0; y < height; ++y)
	{
		for (int x = 0; x < width; ++x)
		{
			At(x, y).Draw(x, y, tileSize);
		}
	}
}
