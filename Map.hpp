#pragma once
#include <Siv3D.hpp>
#include "Tile.hpp"

class Map
{
public:
	int width = 50;
	int height = 30;
	int tileSize = 64;

private:
	std::vector<Tile> m_tiles;

public:
	Map() = default;
	Map(int w, int h, int tileSize = 32);

	void GenerateSimple();
	void FitToScreen(int screenW, int screenH, int uiHeight = 0);

	bool InBounds(int x, int y) const noexcept;
	Tile& At(int x, int y);
	const Tile& At(int x, int y) const;

	void Draw() const;

private:
	int Index(int x, int y) const noexcept { return y * width + x; }
};
