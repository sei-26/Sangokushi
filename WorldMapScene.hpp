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

	// カットイン演出用の変数
	double m_cutInTimer = 0.0;
	String m_cutInText;
	String m_seasonText;
	Color m_seasonColor;

	// ★ セーブ・ロードボタン
	Rect m_btnSave;
	Rect m_btnLoad;

	// ★ 外交ボタン
	Rect m_btnDiplomacy;

	// パフォーマンス最適化: 都市間接続のキャッシュ（各都市の最近接2都市のインデックス）
	Array<Array<int>> m_cityConnections;
};
