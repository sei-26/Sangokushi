#pragma once
#include <Siv3D.hpp>
#include "SceneBase.hpp"
#include "CityData.hpp"
#include "Unit.hpp"

class ArmyConfigScene : public SceneBase
{
private:
	CityData m_city;

	int m_unitCount = 1;            // 出陣ユニット数
	Array<Unit> m_units;            // 出陣ユニット

public:
	ArmyConfigScene(const CityData& city);

	void update() override;
	void draw() const override;
};
