#include "GameSceneManager.hpp"
#include "FactionSelectScene.hpp"
#include "WorldMapScene.hpp"
#include "CityScene.hpp"
#include "BattleScene.hpp"

GameSceneManager::GameSceneManager()
{
	m_currentName = U"FactionSelectScene";
	m_currentScene = std::make_unique<FactionSelectScene>();
}

void GameSceneManager::update()
{
	if (!m_currentScene) return;

	m_currentScene->update();
	if (!m_currentScene->isSceneEnd()) return;

	const String next = m_currentScene->nextSceneName();

	if (m_currentName == U"FactionSelectScene")
	{
		if (auto* fs = dynamic_cast<FactionSelectScene*>(m_currentScene.get()))
		{
			m_playerFaction = fs->getSelectedFaction();
			m_currentScene = std::make_unique<WorldMapScene>(m_playerFaction);
			m_currentName = U"WorldMapScene";
			return;
		}
	}

	if (next.starts_with(U"City:"))
	{
		String cityName = next.substr(5);
		m_currentScene = std::make_unique<CityScene>(cityName);
		m_currentName = U"CityScene";
		return;
	}

	if (next == U"WorldMapScene")
	{
		m_currentScene = std::make_unique<WorldMapScene>(m_playerFaction);
		m_currentName = U"WorldMapScene";
		return;
	}

	if (next == U"BattleScene")
	{
		m_currentScene = std::make_unique<BattleScene>();
		m_currentName = U"BattleScene";
		return;
	}


	if (next == U"WorldMapSceneFromBattle")
	{
		m_currentScene = std::make_unique<WorldMapScene>(m_playerFaction);
		m_currentName = U"WorldMapScene";
		return;
	}
}

void GameSceneManager::draw() const
{
	if (m_currentScene)
		m_currentScene->draw();
}
