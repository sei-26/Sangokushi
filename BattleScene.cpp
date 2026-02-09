#include "BattleScene.hpp"

BattleScene::BattleScene(GameManager* gm, CityData& attacker, CityData& defender)
	: m_gameManager(gm)
	, m_attackerData(&attacker)
	, m_defenderData(&defender)
{
	Officer leader;
	if (attacker.officers.isEmpty())
	{
			leader = Officer(U"守備隊長",50,50,50);
	}
	else
	{
		leader = attacker.officers[0]; // 先頭の武将をリーダーに
	}

	int soldiers = attacker.troops;

	m_battleManager.InitializeBattle(attacker, defender, leader, soldiers, true);

}

void BattleScene::update()
{
	m_battleManager.Update();

	if (m_battleManager.IsBattleFinished())
	{
		if (MouseL.down())
		{
			m_battleManager.ApplyBattleResult(*m_attackerData, *m_defenderData);

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
