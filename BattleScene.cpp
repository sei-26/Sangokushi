#include "BattleScene.hpp"

BattleScene::BattleScene()
{
}

void BattleScene::update()
{
	m_game.Update();

	if (m_game.IsBattleFinished())
	{
		m_isEnd = true;
		m_nextScene = U"WorldMapSceneFromBattle";
	}
}

void BattleScene::draw() const
{
	Scene::SetBackground(ColorF{ 0.05, 0.05, 0.1 });

	// ✅ GameManager::DrawUI() を const 対応に変更
	const_cast<GameManager&>(m_game).DrawUI();
}
