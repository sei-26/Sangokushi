#pragma once
#include <Siv3D.hpp>

// 地形データ
struct Tile
{
	int type = 0; // 0:平地, 1:森, 2:山, 3:城

	double defenseBonus() const
	{
		if (type == 1) return 1.2; // 森
		if (type == 2) return 1.5; // 山
		if (type == 3) return 1.8; // 城
		return 1.0;
	}

	Color getColor() const
	{
		if (type == 1) return Palette::Darkgreen;
		if (type == 2) return Palette::Saddlebrown;
		if (type == 3) return Palette::Gray;
		return Palette::Forestgreen; // 平地
	}
};

// マップクラス
class Map
{
public:
	int width;
	int height;
	int tileSize;
	Grid<Tile> tiles; // グリッドデータ

	// コンストラクタ
	Map(int w = 20, int h = 15, int size = 60)
		: width(w), height(h), tileSize(size)
	{
		tiles.resize(w, h);
		// 地形生成
		for (auto& t : tiles)
		{
			if (RandomBool(0.1)) t.type = 1;
			else if (RandomBool(0.05)) t.type = 2;
			else t.type = 0;
		}
	}

	// 画面サイズ合わせ
	void FitToScreen(int sw, int sh, int uiHeight)
	{
		(void)uiHeight;
		if (width > 0 && height > 0)
		{
			tileSize = Min((sw - 100) / width, (sh - 100) / height);
		}
	}

	bool InBounds(int x, int y) const
	{
		return x >= 0 && x < width && y >= 0 && y < height;
	}

	const Tile& At(int x, int y) const
	{
		static Tile dummy;
		if (!InBounds(x, y)) return dummy;
		return tiles[y][x];
	}

	// ★ 重要修正：Gridの行へアクセスするためにポインタを返す
	Tile* operator[](size_t index)
	{
		return tiles[index];
	}

	const Tile* operator[](size_t index) const
	{
		return tiles[index];
	}

	// 描画
	void Draw() const
	{
		for (int y = 0; y < height; ++y)
		{
			for (int x = 0; x < width; ++x)
			{
				Rect(x * tileSize, y * tileSize, tileSize, tileSize)
					.draw(tiles[y][x].getColor())
					.drawFrame(1, ColorF(0, 0, 0, 0.2));
			}
		}
	}
};
