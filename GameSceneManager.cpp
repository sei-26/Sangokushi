#include "GameSceneManager.hpp"
#include "FactionSelectScene.hpp"
#include "WorldMapScene.hpp"
#include "CityScene.hpp"

GameSceneManager::GameSceneManager()
{
	m_currentName = U"FactionSelectScene";
	m_currentScene = std::make_unique<FactionSelectScene>();
}

void GameSceneManager::update()
{
	if (!m_currentScene) return;
	m_currentScene->update();

	if (m_currentScene->isSceneEnd())
	{
		const String next = m_currentScene->nextSceneName();

		if (m_currentName == U"FactionSelectScene")
		{
			auto* fs = dynamic_cast<FactionSelectScene*>(m_currentScene.get());
			if (fs)
			{
				Faction selected = fs->getSelectedFaction();
				m_currentScene = std::make_unique<WorldMapScene>(selected);
				m_currentName = U"WorldMapScene";
				return;
			}
		}

		if (next.starts_with(U"City:"))
		{
			String city = next.substr(5);
			m_currentScene = std::make_unique<CityScene>(city);
			m_currentName = U"CityScene";
			return;
		}

	}
}

void GameSceneManager::draw() const
{
	if (m_currentScene)
		m_currentScene->draw();
}
