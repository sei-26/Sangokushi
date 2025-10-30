#pragma once
#include "SceneBase.hpp"
#include "GameSceneManager.hpp"
#include "CityData.hpp"

class WorldMapScene : public SceneBase
{
private:
	Array<CityData> cities;
	int hoveredIndex = -1;

public:
	WorldMapScene()
	{
		cities = {
			{ U"洛陽", Vec2(200, 180), Palette::Red, 800, 600, 200, U"曹操" },
			{ U"許昌", Vec2(400, 240), Palette::Orange, 600, 500, 150, U"夏侯惇" },
			{ U"新野", Vec2(300, 380), Palette::Green, 500, 400, 100, U"劉備" },
			{ U"寿春", Vec2(500, 400), Palette::Blue, 700, 500, 180, U"孫堅" },
			{ U"長安", Vec2(100, 250), Palette::Purple, 900, 800, 250, U"董卓" },
		};
	}

	void update() override
	{
		hoveredIndex = -1;

		for (int i = 0; i < (int)cities.size(); ++i)
		{
			const Vec2 pos = cities[i].pos;
			const double dist = pos.distanceFrom(Cursor::Pos());
			if (dist < 32)
			{
				hoveredIndex = i;
				if (MouseL.down())
				{
					Print << U"🏯 城クリック: " << cities[i].name;
					GameSceneManager::SetNextScene(U"City:" + cities[i].name);
				}
			}
		}
	}

	void draw() const override
	{
		Scene::SetBackground(ColorF(0.2, 0.3, 0.2));
		Rect(0, 0, Scene::Width(), Scene::Height()).draw(ColorF(0.1, 0.25, 0.1));

		for (const auto& c : cities)
		{
			Circle(c.pos, 16).draw(c.color);
			FontAsset(U"small")(c.name).drawAt(c.pos.x, c.pos.y - 25, Palette::White);
		}

		if (hoveredIndex >= 0)
		{
			const auto& c = cities[hoveredIndex];
			RectF(20, 20, 220, 100).draw(ColorF(0, 0, 0, 0.6));
			FontAsset(U"small")(Format(U"🏰 {}（{}）", c.name, c.ruler)).draw(30, 30, Palette::Yellow);
			FontAsset(U"small")(Format(U"金:{} 兵:{} 食:{}", c.gold, c.troops, c.food)).draw(30, 55, Palette::White);
			FontAsset(U"small")(U"左クリックで城に入る").draw(30, 80, Palette::Skyblue);
		}
	}
};
