#include "FactionSelectScene.hpp"

FactionSelectScene::FactionSelectScene()
	: m_titleFont(36)
	, m_font(28)
{
	m_factions = {
		{ U"劉備", ColorF{Palette::Green}, { U"新野" }, {} },
		{ U"曹操", ColorF{Palette::Blue},  { U"許昌" }, {} },
		{ U"孫堅", ColorF{Palette::Red},   { U"寿春" }, {} },
	};
}

void FactionSelectScene::update()
{
	if (MouseL.down())
	{
		for (size_t i = 0; i < m_factions.size(); ++i)
		{
			Rect rect(Scene::Center().x - 100, 230 + static_cast<int>(i) * 60, 200, 50);
			if (rect.mouseOver())
			{
				m_selected = m_factions[i];
				m_isEnd = true;
				m_nextScene = U"WorldMapScene";
				return;
			}
		}
	}
}

void FactionSelectScene::draw() const
{
	Scene::SetBackground(ColorF{ 0.1, 0.1, 0.15 });
	m_titleFont(U"勢力を選択してください").drawAt(Scene::Center().x, 100, Palette::Yellow);

	for (size_t i = 0; i < m_factions.size(); ++i)
	{
		Rect rect(Scene::Center().x - 100, 230 + static_cast<int>(i) * 60, 200, 50);
		ColorF c = rect.mouseOver() ? m_factions[i].color : ColorF{ 0.4 };
		rect.draw(c);
		m_font(m_factions[i].name).drawAt(rect.center(), Palette::White);
	}
}
