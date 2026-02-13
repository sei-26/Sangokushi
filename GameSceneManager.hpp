#pragma once
#include <Siv3D.hpp>
#include "SceneBase.hpp"
#include "CityData.hpp"
#include "Faction.hpp"
#include "GameManager.hpp"

class GameSceneManager
{
public:
	// CSV対応版：引数を受け取るコンストラクタ
	GameSceneManager(GameManager* gm, const Faction& playerFaction, const Array<CityData>& cities);
	~GameSceneManager();

	void update();
	void draw() const;
	void changeScene(String nextScene, bool stackClear = false);

private:
	SceneBase* m_currentScene;
	GameManager* m_gameManager;

	Array<CityData> m_cities;
	Faction m_playerFaction;

	int m_selectedCityIndex = 0;
};
