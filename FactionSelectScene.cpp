#include "FactionSelectScene.hpp"

FactionSelectScene::FactionSelectScene()
{
	// 必ず Main.cpp で FontAsset::Register を済ませてある前提

	// 勢力リスト
	m_factions = {
		Faction{
			U"劉備",
			ColorF(Palette::Green),
			{},        // 空の cities
			{}         // 空の officers
		},
		Faction{
			U"曹操",
			ColorF(Palette::Red),
			{},
			{}
		},
		Faction{
			U"孫堅",
			ColorF(Palette::Yellow),
			{},
			{}
		},
		Faction{
			U"董卓",
			ColorF(Palette::Purple),
			{},
			{}
		},
		Faction{
			U"袁紹",
			ColorF(Palette::Blue),
			{},
			{}
		},
		Faction{
			U"公孫瓚",
			ColorF(Palette::Blue),
			{},
			{}
		},
	}; 
}

void FactionSelectScene::update()
{
	if (MouseL.down())
	{
		for (size_t i = 0; i < m_factions.size(); ++i)
		{
			// ボタン位置
			Rect rect(Scene::Center().x - 100,
					  230 + static_cast<int>(i) * 60,
					  200, 50);

			if (rect.mouseOver())
			{
				m_selectedIndex = static_cast<int>(i);
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

	// タイトル
	FontAsset(U"title")(U"勢力を選択してください")
		.drawAt(Scene::Center().x, 100, Palette::Yellow);

	// 勢力リスト描画
	for (size_t i = 0; i < m_factions.size(); ++i)
	{
		Rect rect(Scene::Center().x - 100,
				  230 + static_cast<int>(i) * 60,
				  200, 50);

		bool hovered = rect.mouseOver();

		rect.draw(hovered ? ColorF(m_factions[i].color, 0.8)
						  : ColorF(0.4));

		FontAsset(U"small")(m_factions[i].name)
			.drawAt(rect.center(), Palette::White);
	}
}
