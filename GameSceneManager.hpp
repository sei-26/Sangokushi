#pragma once
#include <Siv3D.hpp>
#include <memory>
#include "GameManager.hpp"
#include "Faction.hpp"
#include "CityData.hpp"
#include "SceneBase.hpp"
#include "TitleScene.hpp"

class GameSceneManager
{
public:
	// コンストラクタ（初期シーンをTitleにする）
	GameSceneManager(GameManager* gm, const Faction& playerFaction, const Array<CityData>& cities, AudioManager* audio);
	~GameSceneManager();

	void update();
	void draw();

	void changeScene(const String& sceneName);

private:
	SceneBase* m_currentScene = nullptr;
	GameManager* m_gameManager = nullptr;
	Faction m_playerFaction;
	Array<CityData> m_cities;
	AudioManager* m_audio = nullptr;

	int m_selectedCityIndex = -1;
	String m_currentSceneName = U"Title";

	int m_attackFromIndex = -1;
	int m_attackTargetIndex = -1;
	Officer m_selectedLeader;
	int m_selectedSoldiers = 0;
};
