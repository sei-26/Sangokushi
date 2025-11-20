#pragma once
#include <Siv3D.hpp>
#include <memory>
#include "SceneBase.hpp"
#include "Faction.hpp"
#include "Advisor.hpp"
#include "CityData.hpp"
#include "WorldMapScene.hpp"
#include "CityScene.hpp"

class GameSceneManager
{
private:
	String m_currentName;
	std::unique_ptr<SceneBase> m_currentScene;
	int m_turn = 1;

	bool checkAllCitiesOwned() const;

	// ★ 全都市リスト
	Array<CityData> m_allCities;

	// ★ 勢力（曹操・劉備 など）
	Faction m_playerFaction;

	// ★ 勢力選択時に決まった「スタート都市名」
	//     WorldMapScene でクリア判定に使うため必須
	String m_startCityName = U"";

	Advisor m_advisor;

public:
	GameSceneManager();

	void update();
	void draw() const;
	void applyTurnGrowth();
};
