#pragma once
#include <Siv3D.hpp>
#include "SceneBase.hpp"
#include "CityData.hpp"
#include "Advisor.hpp"

class CityScene : public SceneBase
{
private:
	CityData* m_city;     // ★ コピー禁止 → ポインタで本物を参照
	Advisor m_advisor;
	Font m_font = Font(24);
	String m_message;

	RectF m_btnAgr{ 500, 100, 200, 40 };
	RectF m_btnCom{ 500, 160, 200, 40 };
	RectF m_btnTrain{ 500, 220, 200, 40 };
	RectF m_btnOrder{ 500, 280, 200, 40 };
	RectF m_btnAttack{ 500, 340, 200, 40 };
	RectF m_btnBack{ 1100, 600, 150, 50 };

	int m_cityIndex = -1;  // ★ 都市インデックスを保持

public:
	CityScene(int cityIndex, Array<CityData>* cities, const Advisor& adv);

	void update() override;
	void draw() const override;

	// ★ シーン遷移用：インデックスを返すだけで OK
	int getCityIndex() const { return m_cityIndex; }
};
