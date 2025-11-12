#pragma once
#include "SceneBase.hpp"
#include "CityData.hpp"
#include "Faction.hpp"

class WorldMapScene : public SceneBase
{
private:
	Array<CityData> m_cities;
	Faction m_playerFaction;
	int m_hovered = -1;

	CityData m_selectedCity; // （今回は未使用。将来 CityData 丸渡し用）

public:
	explicit WorldMapScene(const Faction& faction);

	void update() override;
	void draw() const override;

	const CityData& getSelectedCity() const { return m_selectedCity; } // 将来用
};
