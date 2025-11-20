#pragma once
#include <Siv3D.hpp>
#include "SceneBase.hpp"
#include "CityData.hpp"
#include "Faction.hpp"

class WorldMapScene : public SceneBase
{
private:
	Array<CityData>* m_allCities;
	Faction m_playerFaction;

	int m_hovered = -1;
	int m_selectedIndex = -1;

	bool m_gameClear = false;
	double m_clearTimer = 0.0;

	// ★ 勢力選択で決まったスタート都市名
	String m_startCityName = U"";

public:
	// ★★★ 完全版コンストラクタ（必須） ★★★
	WorldMapScene(const Faction& faction,
				  Array<CityData>* allCities,
				  const String& startCityName);

	void update() override;
	void draw() const override;

	Optional<std::reference_wrapper<CityData>> getSelectedCityRef();
	int getSelectedCityIndex() const { return m_selectedIndex; }
};
