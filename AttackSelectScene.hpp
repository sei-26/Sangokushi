#pragma once
#include <Siv3D.hpp>
#include "SceneBase.hpp"
#include "CityData.hpp"

class AttackSelectScene : public SceneBase
{
private:
	CityData m_fromCity;           // 出陣元の都市
	Array<CityData> m_allCities;   // 全都市（防衛都市含む）
	Optional<CityData> m_targetCity;

public:
	AttackSelectScene(const CityData& from, const Array<CityData>& all);

	void update() override;
	void draw() const override;

	CityData getFromCity() const { return m_fromCity; }
	CityData getTargetCity() const { return m_targetCity.value(); }
};
