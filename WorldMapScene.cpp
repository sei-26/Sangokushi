#pragma once
#include "SceneBase.hpp"
#include "CityData.hpp"
#include "Faction.hpp"

class WorldMapScene : public SceneBase
{
private:
	Array<CityData> m_cities;
	Faction m_playerFaction;
	int m_hovered = -1;

public:
	explicit WorldMapScene(const Faction& faction)
		: m_playerFaction(faction)
	{
		m_cities = {
			{ U"洛陽", Vec2(200, 180), ColorF{Palette::Red}, 800, 600, 200, U"曹操" },
			{ U"許昌", Vec2(400, 240), ColorF{Palette::Orange}, 600, 500, 150, U"夏侯惇" },
			{ U"新野", Vec2(300, 380), ColorF{Palette::Green}, 500, 400, 100, U"劉備" },
			{ U"寿春", Vec2(500, 400), ColorF{Palette::Blue}, 700, 500, 180, U"孫堅" },
			{ U"長安", Vec2(100, 250), ColorF{Palette::Purple}, 900, 800, 250, U"董卓" },
		};
	}

	void update() override
	{
		m_hovered = -1;

		for (int i = 0; i < static_cast<int>(m_cities.size()); ++i)
		{
			const Vec2& pos = m_cities[i].pos;
			const double dist = pos.distanceFrom(Cursor::Pos());

			if (dist < 32.0)
			{
				m_hovered = i;

				// 自軍都市をクリック
				if (m_cities[i].ruler == m_playerFaction.name && MouseL.down())
				{
					m_isEnd = true;
					m_nextScene = U"City:" + m_cities[i].name;
				}
			}
		}
	}

	void draw() const override
	{
		Scene::SetBackground(ColorF{ 0.2, 0.3, 0.2 });
		Rect{ 0, 0, Scene::Width(), Scene::Height() }.draw(ColorF{ 0.1, 0.25, 0.1 });

		for (const auto& c : m_cities)
		{
			ColorF col = (c.ruler == m_playerFaction.name)
				? ColorF{ m_playerFaction.color }
			: ColorF{ c.color, 0.4 };

			Circle(c.pos, 16).draw(col);
			FontAsset(U"small")(c.name).drawAt(c.pos.x, c.pos.y - 25, Palette::White);
		}

		if (m_hovered >= 0)
		{
			const auto& c = m_cities[m_hovered];
			RectF(20, 20, 220, 100).draw(ColorF(0, 0, 0, 0.6));
			FontAsset(U"small")(Format(U"🏰 {}（{}）", c.name, c.ruler)).draw(30, 30, Palette::Yellow);
			FontAsset(U"small")(Format(U"金:{} 兵:{} 食:{}", c.gold, c.troops, c.food)).draw(30, 55, Palette::White);

			if (c.ruler == m_playerFaction.name)
			{
				FontAsset(U"small")(U"左クリックで城に入る").draw(30, 80, Palette::Skyblue);
			}
			else
			{
				FontAsset(U"small")(U"敵勢力の都市").draw(30, 80, Palette::Gray);
			}
		}

		// プレイヤー勢力のステータス
		RectF(20, Scene::Height() - 80, 220, 60).draw(ColorF(0, 0, 0, 0.5));
		FontAsset(U"small")(Format(U"🎌 勢力: {}", m_playerFaction.name)).draw(30, Scene::Height() - 70, m_playerFaction.color);
		FontAsset(U"small")(Format(U"金:{} 食糧:{}", m_playerFaction.gold, m_playerFaction.food)).draw(30, Scene::Height() - 45, Palette::White);
	}
};
