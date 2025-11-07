// GameSceneManager.hpp
#pragma once
#include <Siv3D.hpp>
#include "SceneBase.hpp"

class GameSceneManager
{
private:
	std::unique_ptr<SceneBase> m_currentScene;
	String m_currentName;

public:
	GameSceneManager();

	void update();
	void draw() const;
};
