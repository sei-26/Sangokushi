#pragma once
#include <Siv3D.hpp>
#include <memory>
#include "SceneBase.hpp"
#include "Faction.hpp"
#include "Advisor.hpp"
#include "CityData.hpp"

class GameSceneManager
{
private:
	String m_currentName;
	std::unique_ptr<SceneBase> m_currentScene;
	int m_turn = 1;

	// ★ 全都市を管理するリスト
	Array<CityData> m_allCities;

	// 勢力を保持しておく（CityScene→戻る時に必要）
	Faction m_playerFaction;

	Advisor m_advisor;
public:
	GameSceneManager();

	void update();
	void draw() const;
	void applyTurnGrowth();
};
