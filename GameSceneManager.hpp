#pragma once
#include <Siv3D.hpp>
#include "SceneBase.hpp"
#include "GameManager.hpp"
#include "Faction.hpp"
#include "CityData.hpp"
#include "AudioManager.hpp"

class GameSceneManager
{
public:
	GameSceneManager(GameManager* gm, const Faction& playerFaction, const Array<CityData>& cities, AudioManager* audio);
	~GameSceneManager();

	void update();
	void draw();

private:
	SceneBase* m_currentScene;
	GameManager* m_gameManager;
	Faction m_playerFaction;
	Array<CityData> m_cities;
	AudioManager* m_audio;
	int m_selectedCityIndex = -1;
	String m_currentSceneName = U"WorldMap";
};
