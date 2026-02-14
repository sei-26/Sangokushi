#include "BattleScene.hpp"

// ★ 修正：4引数バージョンのコンストラクタ
BattleScene::BattleScene(GameManager* gm, CityData& playerCity, CityData& enemyCity, bool isPlayerAttacker)
	: m_gameManager(gm)
	, m_playerCityData(&playerCity)
	, m_enemyCityData(&enemyCity)
	, m_isPlayerAttacker(isPlayerAttacker)
{
	// NULLチェック
	if (gm == nullptr)
	{
		Print << U"[ERROR] GameManagerがnullです";
		return;
	}

	Console << U"[戦闘開始] プレイヤー=" << playerCity.name
		<< U", 敵=" << enemyCity.name
		<< U", 攻撃側=" << (isPlayerAttacker ? U"プレイヤー" : U"敵");

	// プレイヤーのリーダーを取得
	Officer leader;
	if (playerCity.officers.isEmpty())
	{
		//leader = Officer(U"守備隊長", 50, 50, 50);
	}
	else
	{
		leader = playerCity.officers[0];
	}

	// 兵数（最低1は確保）
	int soldiers = Max(playerCity.troops, 1);

	// ★ 重要：InitializeBattle には常に (プレイヤー都市, 敵都市) の順で渡す
	m_battleManager.InitializeBattle(playerCity, enemyCity, leader, soldiers, isPlayerAttacker);
}

void BattleScene::update()
{
	m_battleManager.Update();

	if (m_battleManager.IsBattleFinished())
	{
		if (MouseL.down())
		{
			// ★ 修正：攻守によって正しい順序で渡す
			if (m_isPlayerAttacker)
			{
				// プレイヤーが攻撃側の場合：(攻撃者=プレイヤー, 防御者=敵)
				m_battleManager.ApplyBattleResult(*m_playerCityData, *m_enemyCityData);
			}
			else
			{
				// プレイヤーが防御側の場合：(攻撃者=敵, 防御者=プレイヤー)
				m_battleManager.ApplyBattleResult(*m_enemyCityData, *m_playerCityData);
			}

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
