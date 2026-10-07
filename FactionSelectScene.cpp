#include "FactionSelectScene.hpp"

void FactionSelectScene::update()
{
	if (Rect(Scene::Center().x - 210, 155, 420, 50).leftClicked())
	{
		m_gameManager->career = PlayerCareer{};
		m_gameManager->career.officerMode = !m_officerMode;
		m_officerMode = !m_officerMode;
		return;
	}
	if (s3d::MouseL.down())
	{
		for (size_t i = 0; i < m_factions.size(); ++i)
		{
			// ボタン位置の判定
			s3d::Rect rect(s3d::Scene::Center().x - 100, 230 + static_cast<int>(i) * 60, 200, 50);

			if (rect.mouseOver())
			{
				m_selectedIndex = static_cast<int>(i);
				m_gameManager->career = PlayerCareer{};
				m_gameManager->career.officerMode = m_officerMode;
				m_gameManager->territoryReady = false;
				if (m_gameManager->pAudio) m_gameManager->pAudio->PlaySE(AudioManager::SEType::Select);

				m_sceneEnd = true;
				m_nextScene = m_officerMode ? U"Career" : U"WorldMap";
				return;
			}
		}
	}
}

void FactionSelectScene::draw() const
{
	Rect(Scene::Center().x - 210, 155, 420, 50).draw(ColorF(0.25, 0.3, 0.4));
	FontAsset(U"menu")(m_officerMode ? U"開始：一般武将（クリックで切替）" : U"開始：君主（クリックで切替）").drawAt(Scene::Center().x, 180);
	s3d::Scene::SetBackground(s3d::ColorF{ 0.1, 0.1, 0.15 });

	s3d::FontAsset(U"title")(U"勢力を選択してください")
		.drawAt(s3d::Scene::Center().x, 100, s3d::Palette::Yellow);

	for (size_t i = 0; i < m_factions.size(); ++i)
	{
		s3d::Rect rect(s3d::Scene::Center().x - 100, 230 + static_cast<int>(i) * 60, 200, 50);
		bool hovered = rect.mouseOver();

		rect.draw(hovered ? s3d::ColorF(m_factions[i].color, 0.8) : s3d::ColorF(0.4));
		rect.drawFrame(1, s3d::Palette::White);

		s3d::FontAsset(U"small")(m_factions[i].name).drawAt(rect.center(), s3d::Palette::White);
	}
}
