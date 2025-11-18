#pragma once
#include <Siv3D.hpp>
#include "SceneBase.hpp"
#include "CityData.hpp"
#include "Faction.hpp"

class WorldMapScene : public SceneBase
{
private:
	Faction m_playerFaction;

	Array<CityData> m_cities;              // 全都市データ（コピー or 参照）
	Optional<CityData> m_selectedCity;     // 選択された都市
	bool m_hasSelection = false;

	int m_hovered = -1;

public:
	// ★ 2 引数版（必須）
	WorldMapScene(const Faction& faction, const Array<CityData>& allCities);

	void update() override;
	void draw() const override;

	bool hasSelectedCity() const { return m_hasSelection; }
	Optional<CityData> getSelectedCity() const { return m_selectedCity; }
};
