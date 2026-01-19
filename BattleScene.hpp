#pragma once
#include "SceneBase.hpp"
#include "GameManager.hpp"
#include "BattleGameManager.hpp" // ★これをインクルード

class BattleScene : public SceneBase
{
public:
	BattleScene(GameManager* gm);

	void update() override;
	void draw() const override;

private:
	GameManager* m_gameManager;

	// ★ 戦闘処理はすべてこのクラスに任せる
	BattleGameManager m_battleManager;
};
