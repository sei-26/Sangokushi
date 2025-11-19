#pragma once
#include <Siv3D.hpp>
#include "SceneBase.hpp"
#include "BattleGameManager.hpp"
#include "CityData.hpp"
#include "Officer.hpp"

//==========================================================
// 三國志8リメイク簡易戦場シーン
//==========================================================
class BattleMapScene : public SceneBase
{
private:
	BattleGameManager manager;   // ★戦場ロジック本体
	CityData atkCity;   // 出陣元
	CityData defCity;   // 防衛都市
	CityData* atkCityRef;
	CityData* defCityRef;

public:
	// 出撃・防衛データをまとめて受け取る
	BattleMapScene(const CityData& fromCity,
				   const CityData& targetCity,
				   const Officer& selectedLeader,
				   int selectedSoldiers);

	void update() override;
	void draw() const override;
};
