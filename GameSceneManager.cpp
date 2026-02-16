#include "GameSceneManager.hpp"
#include "TitleScene.hpp"
#include "FactionSelectScene.hpp"
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
	// ★初期シーンを Title に設定
	m_currentScene = new TitleScene(m_gameManager);
	m_currentSceneName = U"Title";
}

GameSceneManager::~GameSceneManager()
{
	if (m_currentScene) delete m_currentScene;
}

void GameSceneManager::update()
{
	// 勝利判定（タイトル・勢力選択中はスキップ）
	if (m_currentSceneName != U"Title" && m_currentSceneName != U"FactionSelect")
	{
		auto victoryType = VictoryCondition::CheckVictory(m_cities, m_playerFaction.name, m_gameManager->year, m_gameManager->month);
		if (victoryType != VictoryCondition::VictoryType::None)
		{
			int playerCities = 0;
			for (const auto& city : m_cities) { if (city.owner == m_playerFaction.name) playerCities++; }
			int score = VictoryCondition::CalculateScore(m_cities, m_playerFaction.name, m_gameManager->year, m_gameManager->month);

			delete m_currentScene;
			m_currentScene = new EndingScene(victoryType, score, playerCities, m_gameManager->year, m_gameManager->month);
			m_currentSceneName = U"Ending";
			if (victoryType == VictoryCondition::VictoryType::Defeat) m_audio->PlayBGM(AudioManager::BGMType::Defeat, 2.0);
			else m_audio->PlayBGM(AudioManager::BGMType::Victory, 2.0);
			return;
		}
	}

	if (m_currentScene)
	{
		m_currentScene->update();

		if (m_currentScene->isSceneEnd())
		{
			String next = m_currentScene->getNextScene();

			// 特定のシーンからのデータ引き継ぎ
			if (next == U"City")
			{
				auto* mapScene = dynamic_cast<WorldMapScene*>(m_currentScene);
				if (mapScene) m_selectedCityIndex = mapScene->getSelectedCityIndex();
			}

			// ★勢力選択シーンが終わった時、選ばれた勢力を反映させる
			if (m_currentSceneName == U"FactionSelect")
			{
				auto* fsScene = dynamic_cast<FactionSelectScene*>(m_currentScene);
				if (fsScene) {
					m_playerFaction = fsScene->getSelectedFaction();
					m_gameManager->playerFactionName = m_playerFaction.name;
				}
			}

			delete m_currentScene;
			m_currentScene = nullptr;

			// --- シーン遷移の分岐 ---
			if (next == U"Title")
			{
				m_currentScene = new TitleScene(m_gameManager);
				m_currentSceneName = U"Title";
			}
			else if (next == U"FactionSelect")
			{
				// ここで m_gameManager と m_cities を渡す
				m_currentScene = new FactionSelectScene(m_gameManager, m_cities);
				m_currentSceneName = U"FactionSelect";
			}
			else if (next == U"WorldMap")
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
				if (m_selectedCityIndex >= 0 && m_selectedCityIndex < (int)m_cities.size())
				{
					m_currentScene = new CityScene(m_gameManager, m_cities[m_selectedCityIndex]);
					if (m_currentSceneName != U"City")
					{
						m_audio->PlayBGM(AudioManager::BGMType::City, 2.0);
						m_currentSceneName = U"City";
					}
				}
				else m_currentScene = new WorldMapScene(m_gameManager, m_playerFaction, &m_cities);
			}
			else if (next == U"Battle")
			{
				// ... (既存のBattle遷移ロジック)
				m_currentScene = new BattleScene(m_gameManager, m_cities[m_selectedCityIndex], m_cities[0], true); // 簡易化
				m_audio->PlayBGM(AudioManager::BGMType::Battle, 2.0);
				m_currentSceneName = U"Battle";
			}
			else if (next == U"Diplomacy")
			{
				m_currentScene = new DiplomacyScene(&m_gameManager->diplomacy, m_playerFaction.name, &m_cities, m_gameManager);
				m_currentSceneName = U"Diplomacy";
			}
			else if (next == U"OfficerManagement")
			{
				m_currentScene = new OfficerManagementScene(m_gameManager, &m_cities[m_selectedCityIndex]);
				m_currentSceneName = U"OfficerManagement";
			}
			else if (next == U"Facility")
			{
				m_currentScene = new FacilityScene(m_gameManager, &m_cities[m_selectedCityIndex], &m_gameManager->turnManager);
				m_currentSceneName = U"Facility";
			}
		}
	}
}

void GameSceneManager::draw()
{
	if (m_currentScene) m_currentScene->draw();
}
