#pragma once
#include <Siv3D.hpp>
#include <memory>
#include "SceneBase.hpp"

class GameSceneManager
{
private:
	inline static std::unique_ptr<SceneBase> currentScene;
	inline static String nextScene = U"";

public:
	static void SetNextScene(const String& name)
	{
		Print << U"SetNextScene: " << name;
		nextScene = name;
	}

	static void setScene(std::unique_ptr<SceneBase> newScene)
	{
		currentScene = std::move(newScene);
		nextScene.clear();
	}

	static void update();
	static void draw();
};
