#pragma once
#include <Siv3D.hpp>
#include "SceneBase.hpp"
#include "CityData.hpp"

class AttackSelectScene : public SceneBase
{
private:
	CityData m_fromCity;
	Array<CityData> m_allCities;
	Array<int> m_neighbors;

public:
	AttackSelectScene(const CityData& from, const Array<CityData>& allCities);

	void update() override;
	void draw() const override;
};
