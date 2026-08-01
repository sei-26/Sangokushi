#pragma once
#include <Siv3D.hpp>
#include "SceneBase.hpp"
#include "BattleGameManager.hpp"
#include "CityData.hpp"
#include "Officer.hpp"

//==========================================================
// 戦場シーン：
//==========================================================
class BattleMapScene : public SceneBase
{
private:
	BattleGameManager manager;

	// ★ 本物の都市データを直接参照
	CityData* atkCityRef = nullptr;
	CityData* defCityRef = nullptr;

	int atkIndex = -1;
	int defIndex = -1;

	Array<CityData>* allCities = nullptr;

	Officer m_leader;
	int m_soldiers = 0;

public:
	BattleMapScene(int atkIndex,
				   int defIndex,
				   Array<CityData>* allCities,
				   const Officer& leader,
				   int soldiers);

	void update() override;
	void draw() const override;

	int getAtkIndex() const { return atkIndex; }
	int getDefIndex() const { return defIndex; }
};
