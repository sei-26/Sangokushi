#pragma once
#include "SceneBase.hpp"
#include "GameManager.hpp"
#include "CityData.hpp"

class CityScene : public SceneBase
{
public:
	CityScene(GameManager* gm, CityData& city);

	void update() override;
	void draw() const override;

private:
	GameManager* m_gameManager;
	CityData* m_cityData;

	// 画面パーツ
	String m_message;
	Rect m_btnAgr;    // 農業
	Rect m_btnCom;    // 商業
	Rect m_btnTrain;  // 徴兵
	Rect m_btnOrder;  // 治安
	Rect m_btnOfficer; // ★ 人材
	Rect m_btnAttack; // 出陣
	Rect m_btnBack;   // 戻る
};
