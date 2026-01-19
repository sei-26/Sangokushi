#pragma once
#include <Siv3D.hpp>
#include "SceneBase.hpp"
#include "CityData.hpp"
#include "Faction.hpp"
#include "GameManager.hpp"

class GameSceneManager
{
public:
	GameSceneManager();
	~GameSceneManager();

	void update();
	void draw() const;
	void changeScene(String nextScene, bool stackClear = false);

private:
	SceneBase* m_currentScene;
	GameManager m_gameManager; // ゲーム管理（年月など）

	Array<CityData> m_cities;
	Faction m_playerFaction;

	// ★ これを追加：選択中の都市の番号
	int m_selectedCityIndex = 0;
};
