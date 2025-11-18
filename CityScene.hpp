#pragma once
#include <Siv3D.hpp>
#include "SceneBase.hpp"
#include "CityData.hpp"
#include "Advisor.hpp"

class CityScene : public SceneBase
{
private:
	CityData m_city;
	Advisor m_advisor;
	Font m_font = Font(24);
	String m_message;

	// ===== 誤判定防止：ボタンはメンバにする =====
	RectF m_btnAgr{ 500, 100, 200, 40 };
	RectF m_btnCom{ 500, 160, 200, 40 };
	RectF m_btnTrain{ 500, 220, 200, 40 };
	RectF m_btnOrder{ 500, 280, 200, 40 };
	RectF m_btnBack{ 1100, 600, 150, 50 };

public:
	CityScene(const CityData& city, const Advisor& adv);

	void update() override;
	void draw() const override;
};
