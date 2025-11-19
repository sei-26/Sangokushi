#pragma once
#include <Siv3D.hpp>
#include "SceneBase.hpp"
#include "Advisor.hpp"

class TurnEndScene : public SceneBase
{
private:
	Font m_font = Font(32);
	Font m_small = Font(20);

	double m_alpha = 0.0;     // 背景フェード
	double m_faceAlpha = 0.0; // 立ち絵フェード

	Advisor m_advisor;
	int m_turn;

	Texture m_advisorTexture;

public:
	TurnEndScene(const Advisor& adv, int turn);

	void update() override;
	void draw() const override;
};
