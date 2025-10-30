#include "GameSceneManager.hpp"
#include "WorldMapScene.hpp"
#include "CityScene.hpp"
#include "BattleScene.hpp"

void GameSceneManager::update()
{
	if (nextScene.starts_with(U"City:"))
	{
		String cityName = nextScene.substr(5);
		Print << U"🌆 " << cityName << U" の情報画面を開く";

		CityData data;
		data.name = cityName;
		data.gold = 500;
		data.food = 400;
		data.troops = 100;
		data.ruler = U"所属不明";
		data.color = Palette::Gray;

		setScene(std::make_unique<CityScene>(data));
	}
	else if (nextScene.starts_with(U"Battle:"))
	{
		String cityName = nextScene.substr(7);
		Print << U"🗡 " << cityName << U" から出撃！";
		setScene(std::make_unique<BattleScene>());
	}

	else if (nextScene == U"World")
	{
		setScene(std::make_unique<WorldMapScene>());
	}

	if (currentScene)
		currentScene->update();
}

void GameSceneManager::draw()
{
	if (currentScene)
		currentScene->draw();
}
