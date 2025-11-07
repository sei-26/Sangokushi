#include "FactionSelectScene.hpp"

FactionSelectScene::FactionSelectScene()
	: m_titleFont(36)
	, m_font(28)
{
	m_factions = {
		{ U"劉備軍", Palette::Green, { U"新野" }, {} },
		{ U"曹操軍", Palette::Blue,  { U"許昌" }, {} },
		{ U"孫権軍", Palette::Red,   { U"建業" }, {} },
	};
}

void FactionSelectScene::update()
{
	if (KeyDown.down()) m_cursor = (m_cursor + 1) % m_factions.size();
	if (KeyUp.down())   m_cursor = (m_cursor + m_factions.size() - 1) % m_factions.size();

	if (KeyEnter.down())
	{
		m_selected = m_factions[m_cursor];
		m_isEnd = true;
		m_nextScene = U"WorldMapScene";
	}
}

void FactionSelectScene::draw() const
{
	Scene::SetBackground(ColorF{ 0.1, 0.1, 0.15 });

	m_titleFont(U"勢力を選択してください")
		.drawAt(Scene::Center().x, 100, Palette::Yellow);

	for (size_t i = 0; i < m_factions.size(); ++i)
	{
		const bool selected = (i == m_cursor);
		ColorF c = selected ? ColorF{ m_factions[i].color } : ColorF{ 0.4 };
		m_font(m_factions[i].name)
			.drawAt(Scene::Center().x, 250 + static_cast<int>(i) * 60, c);
	}
}
