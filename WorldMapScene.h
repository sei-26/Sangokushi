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

	double m_cutInTimer;
	String m_cutInText;
	String m_seasonText;
	ColorF m_seasonColor;

	// ★ 紙テクスチャをキャッシュ（draw() が const のため mutable にする）
	mutable RenderTexture m_paperTexture;
	mutable uint32 m_lastMapWidth = 0;
	mutable uint32 m_lastMapHeight = 0;
};