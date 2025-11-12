#pragma once
#include <Siv3D.hpp>
#include <memory>
#include "SceneBase.hpp"
#include "Faction.hpp"

class GameSceneManager
{
private:
	String m_currentName;
	std::unique_ptr<SceneBase> m_currentScene;

	// 勢力を保持しておく（CityScene→戻る時に必要）
	Faction m_playerFaction;

public:
	GameSceneManager();

	void update();
	void draw() const;
};
