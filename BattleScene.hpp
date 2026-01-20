#pragma once
#include "SceneBase.hpp"
#include "GameManager.hpp"
#include "BattleGameManager.hpp" // ★これをインクルード

class BattleScene : public SceneBase
{
public:
	BattleScene(GameManager* gm, CityData& attacker, CityData& defender);

	void update() override;
	void draw() const override;

private:
	GameManager* m_gameManager;
	BattleGameManager m_battleManager;

	CityData* m_attackerData;
	CityData* m_defenderData;
};
