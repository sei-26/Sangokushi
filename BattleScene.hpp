#pragma once
#include "SceneBase.hpp"
#include "GameManager.hpp"

class BattleScene : public SceneBase
{
private:
	GameManager m_game;

public:
	BattleScene();

	void update() override;
	void draw() const override; // ← const のままでOK
};
