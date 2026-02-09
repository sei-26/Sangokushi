#pragma once
#include "SceneBase.hpp"
#include "GameManager.hpp"
#include "BattleGameManager.hpp"

class BattleScene : public SceneBase
{
public:
	// ★ 修正：4引数バージョンに変更（isPlayerAttacker を追加）
	BattleScene(GameManager* gm, CityData& playerCity, CityData& enemyCity, bool isPlayerAttacker);

	void update() override;
	void draw() const override;

private:
	GameManager* m_gameManager;
	BattleGameManager m_battleManager;

	// ★ 修正：名前を明確化
	CityData* m_playerCityData;  // プレイヤーの都市
	CityData* m_enemyCityData;   // 敵の都市

	bool m_isPlayerAttacker;     // プレイヤーが攻撃側かどうか
};
