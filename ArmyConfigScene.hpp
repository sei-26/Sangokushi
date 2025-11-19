#pragma once
#include <Siv3D.hpp>
#include "SceneBase.hpp"
#include "CityData.hpp"
#include "Officer.hpp"

class ArmyConfigScene : public SceneBase
{
private:
	CityData m_fromCity;
	CityData m_targetCity;

	int m_selectedOfficerIndex = 0;
	int m_soldierAllocation = 500;   // 初期出陣兵数（少なめ）

public:
	ArmyConfigScene(const CityData& from, const CityData& target);

	void update() override;
	void draw() const override;

	Officer getSelectedOfficer() const;
	int getSoldierAllocation() const;

	// 都市へのアクセスも必要なら
	CityData getFromCity() const { return m_fromCity; }
	CityData getTargetCity() const { return m_targetCity; }
};
