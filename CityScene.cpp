#include "CityScene.hpp"

CityScene::CityScene(const String& cityName)
	: m_cityName(cityName)
	, m_font(28)
	, m_commands({ U"農業", U"商業", U"治安", U"出陣", U"戻る" })
{
}

void CityScene::update()
{
	if (MouseL.down())
	{
		for (size_t i = 0; i < m_commands.size(); ++i)
		{
			Rect rect(100, 200 + static_cast<int>(i) * 60, 200, 50);
			if (rect.mouseOver())
			{
				const String& cmd = m_commands[i];

				if (cmd == U"農業")       m_food += Random(30, 80);
				else if (cmd == U"商業")  m_gold += Random(40, 100);
				else if (cmd == U"治安")  m_order = Min(100, m_order + Random(5, 15));
				else if (cmd == U"出陣")
				{
					m_isEnd = true;
					m_nextScene = U"BattleScene"; // これでGameSceneManagerがBattleSceneを呼ぶ
				}
				else if (cmd == U"戻る")
				{
					m_isEnd = true;
					m_nextScene = U"WorldMapScene";
				}
				return;
			}
		}
	}

	if (MouseR.down())
	{
		m_isEnd = true;
		m_nextScene = U"WorldMapScene";
	}
}

void CityScene::draw() const
{
	Scene::SetBackground(ColorF{ 0.1, 0.12, 0.16 });

	m_font(Format(U"🏯 都市：{}", m_cityName)).draw(50, 60, Palette::Yellow);
	m_font(Format(U"💰 金:{}　🌾 食糧:{}　🛡 治安:{}", m_gold, m_food, m_order))
		.draw(50, 110, Palette::White);

	for (size_t i = 0; i < m_commands.size(); ++i)
	{
		Rect rect(100, 200 + static_cast<int>(i) * 60, 200, 50);
		ColorF color = rect.mouseOver() ? Palette::Orange : Palette::Gray;
		rect.draw(color);
		m_font(m_commands[i]).drawAt(rect.center(), Palette::White);
	}
}
