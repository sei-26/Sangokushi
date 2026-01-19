#pragma once
#include "SceneBase.hpp"
#include "GameManager.hpp"

class WorldMapScene : public SceneBase
{
public:
	WorldMapScene(GameManager* gm, const Faction& faction, Array<CityData>* allCities);

	void update() override;
	void draw() const override;

	Optional<std::reference_wrapper<CityData>> getSelectedCityRef();
	int getSelectedCityIndex() const { return m_selectedIndex; }

private:
	GameManager* m_gameManager;
	Faction m_playerFaction;
	Array<CityData>* m_allCities;

	int m_hovered;
	int m_selectedIndex;

	// ---------------------------------------------------
	// ★ カットイン演出用の変数を追加
	// ---------------------------------------------------
	double m_cutInTimer = 0.0; // 0より大きいときは演出中
	String m_cutInText;        // 表示する文字（例: "184年 1月"）
	String m_seasonText;       // 季節（例: "春"）
	Color m_seasonColor;       // 季節の色（春は桜色、冬は白など）
};
