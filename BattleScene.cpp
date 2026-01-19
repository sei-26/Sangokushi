#include "BattleScene.hpp"

BattleScene::BattleScene(GameManager* gm)
	: m_gameManager(gm)
{
	// ダミーデータ
	CityData dummyAtk(U"味方拠点", Point(0, 0), U"劉備");
	CityData dummyDef(U"敵拠点", Point(0, 0), U"曹操");

	// 武将
	Officer leader(U"関羽", 95, 97, 75, 60);

	// ★ エラー修正：m_gameManagerではなく、m_battleManagerを呼び出します
	m_battleManager.InitializeBattle(dummyAtk, dummyDef, leader, 3000);
}

void BattleScene::update()
{
	m_battleManager.Update();

	if (m_battleManager.IsBattleFinished())
	{
		if (MouseL.down())
		{
			m_sceneEnd = true;
			m_nextScene = U"WorldMap";
		}
	}
}

void BattleScene::draw() const
{
	Scene::SetBackground(ColorF{ 0.1, 0.1, 0.1 });
	m_battleManager.Draw();

	if (m_battleManager.IsBattleFinished())
	{
		Scene::Rect().draw(ColorF(0, 0, 0, 0.6));
		FontAsset(U"title")(U"戦闘終了 - クリックで戻る").drawAt(Scene::Center(), Palette::White);
	}
}
