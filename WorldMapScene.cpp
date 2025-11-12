#include "WorldMapScene.hpp"

WorldMapScene::WorldMapScene(const Faction& faction)
	: m_playerFaction(faction)
{
	m_cities = {
		{ U"洛陽", Vec2(200, 180), ColorF{Palette::Red},    800, 600, 200, 80, U"曹操" },
		{ U"許昌", Vec2(400, 240), ColorF{Palette::Orange}, 600, 500, 150, 70, U"夏侯惇" },
		{ U"新野", Vec2(300, 380), ColorF{Palette::Green},  500, 400, 100, 75, U"劉備" },
		{ U"寿春", Vec2(500, 400), ColorF{Palette::Blue},   700, 500, 180, 85, U"孫堅" },
		{ U"長安", Vec2(100, 250), ColorF{Palette::Purple}, 900, 800, 250, 60, U"董卓" },
	};
}

void WorldMapScene::update()
{
	m_hovered = -1;

	for (int i = 0; i < static_cast<int>(m_cities.size()); ++i)
	{
		const Vec2& pos = m_cities[i].pos;
		if (Circle(pos, 20).mouseOver())
		{
			m_hovered = i;

			if (MouseL.down() && m_cities[i].ruler == m_playerFaction.name)
			{
				m_selectedCity = m_cities[i];
				m_isEnd = true;
				m_nextScene = U"City:" + m_cities[i].name;
				return;
			}
		}
	}

	if (MouseR.down())
	{
		m_isEnd = true;
		m_nextScene = U"FactionSelectScene";
	}
}

void WorldMapScene::draw() const
{
	Scene::SetBackground(ColorF{ 0.2, 0.3, 0.2 });

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
		RectF(20, 20, 240, 110).draw(ColorF(0, 0, 0, 0.6));
		FontAsset(U"small")(Format(U"🏰 {}（{}）", c.name, c.ruler)).draw(30, 30, Palette::Yellow);
		FontAsset(U"small")(Format(U"金:{} 兵:{} 食:{} 治安:{}", c.gold, c.troops, c.food, c.order)).draw(30, 58, Palette::White);
		if (c.ruler == m_playerFaction.name)
			FontAsset(U"small")(U"左クリックで城に入る").draw(30, 86, Palette::Skyblue);
		else
			FontAsset(U"small")(U"他勢力の都市").draw(30, 86, Palette::Gray);
	}
}
