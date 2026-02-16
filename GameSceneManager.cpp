#include "GameSceneManager.hpp"
#include "WorldMapScene.hpp"
#include "CityScene.hpp"
#include "BattleScene.hpp"
#include "DiplomacyScene.hpp"
#include "OfficerManagementScene.hpp"
#include "FacilityScene.hpp"
#include "EndingScene.hpp"
#include "VictoryCondition.hpp"

GameSceneManager::GameSceneManager(GameManager* gm, const Faction& playerFaction, const Array<CityData>& cities, AudioManager* audio)
	: m_currentScene(nullptr)
	, m_gameManager(gm)
	, m_playerFaction(playerFaction)
	, m_cities(cities)
	, m_audio(audio)
{
	m_currentScene = new WorldMapScene(m_gameManager, m_playerFaction, &m_cities);
	m_audio->PlayBGM(AudioManager::BGMType::WorldMap, 2.0);
}

GameSceneManager::~GameSceneManager()
{
	if (m_currentScene) delete m_currentScene;
}

void GameSceneManager::update()
{
	// 勝利判定
	auto victoryType = VictoryCondition::CheckVictory(m_cities, m_playerFaction.name,
		m_gameManager->year, m_gameManager->month);

	if (victoryType != VictoryCondition::VictoryType::None)
	{
		int playerCities = 0;
		for (const auto& city : m_cities)
		{
			if (city.owner == m_playerFaction.name) playerCities++;
		}

		int score = VictoryCondition::CalculateScore(m_cities, m_playerFaction.name,
			m_gameManager->year, m_gameManager->month);

		delete m_currentScene;
		m_currentScene = new EndingScene(victoryType, score, playerCities,
			m_gameManager->year, m_gameManager->month);

		if (victoryType == VictoryCondition::VictoryType::Defeat)
		{
			m_audio->PlayBGM(AudioManager::BGMType::Defeat, 2.0);
		}
		else
		{
			m_audio->PlayBGM(AudioManager::BGMType::Victory, 2.0);
		}
		return;
	}

	if (m_currentScene)
	{
		m_currentScene->update();

		if (m_currentScene->isSceneEnd())
		{
			String next = m_currentScene->getNextScene();

			if (next == U"City")
			{
				auto* mapScene = dynamic_cast<WorldMapScene*>(m_currentScene);
				if (mapScene)
				{
					m_selectedCityIndex = mapScene->getSelectedCityIndex();
				}
			}

			delete m_currentScene;
			m_currentScene = nullptr;

			if (next == U"WorldMap")
			{
				m_currentScene = new WorldMapScene(m_gameManager, m_playerFaction, &m_cities);
				if (m_currentSceneName != U"WorldMap")
				{
					m_audio->PlayBGM(AudioManager::BGMType::WorldMap, 2.0);
					m_currentSceneName = U"WorldMap";
				}
			}
			else if (next == U"City")
			{
				if (m_selectedCityIndex >= 0 && m_selectedCityIndex < m_cities.size())
				{
					m_currentScene = new CityScene(m_gameManager, m_cities[m_selectedCityIndex]);
					if (m_currentSceneName != U"City")
					{
						m_audio->PlayBGM(AudioManager::BGMType::City, 2.0);
						m_currentSceneName = U"City";
					}
				}
				else
				{
					m_currentScene = new WorldMapScene(m_gameManager, m_playerFaction, &m_cities);
				}
			}
			else if (next == U"Battle")
			{
				CityData* attacker = nullptr;
				CityData* defender = nullptr;

				if (m_selectedCityIndex >= 0 && m_selectedCityIndex < m_cities.size())
				{
					attacker = &m_cities[m_selectedCityIndex];
				}

				for (auto& city : m_cities)
				{
					if (city.owner != m_playerFaction.name)
					{
						defender = &city;
						break;
					}
				}

				if (!attacker || !defender)
				{
					m_currentScene = new WorldMapScene(m_gameManager, m_playerFaction, &m_cities);
				}
				else
				{
					Officer selectedLeader;
					if (!attacker->officers.isEmpty())
					{
						selectedLeader = attacker->officers[0];
					}

					m_currentScene = new BattleScene(m_gameManager, *attacker, *defender, true);
					if (m_currentSceneName != U"Battle")
					{
						m_audio->PlayBGM(AudioManager::BGMType::Battle, 2.0);
						m_currentSceneName = U"Battle";
					}
				}
			}
			else if (next == U"Diplomacy")
			{
				m_currentScene = new DiplomacyScene(&m_gameManager->diplomacy, m_playerFaction.name, &m_cities, m_gameManager);
			}
			else if (next == U"OfficerManagement")
			{
				if (m_selectedCityIndex >= 0 && m_selectedCityIndex < m_cities.size())
				{
					m_currentScene = new OfficerManagementScene(m_gameManager, &m_cities[m_selectedCityIndex]);
				}
				else
				{
					m_currentScene = new WorldMapScene(m_gameManager, m_playerFaction, &m_cities);
				}
			}
			else if (next == U"Facility")
			{
				if (m_selectedCityIndex >= 0 && m_selectedCityIndex < m_cities.size())
				{
					m_currentScene = new FacilityScene(m_gameManager, &m_cities[m_selectedCityIndex], &m_gameManager->turnManager);
				}
				else
				{
					m_currentScene = new WorldMapScene(m_gameManager, m_playerFaction, &m_cities);
				}
			}
		}
	}
}

void GameSceneManager::draw()
{
	if (m_currentScene)
	{
		m_currentScene->draw();
	}
}
