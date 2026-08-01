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
#include "AttackSelectScene.hpp"
#include "ArmyConfigScene.hpp"
#include "BattleMapScene.hpp"
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
			// ★ AttackSelectScene からのデータ取得を追加
			if (next == U"ArmyConfigScene")
			{
				auto* attackScene = dynamic_cast<AttackSelectScene*>(m_currentScene);
				if (attackScene)
				{
					m_attackFromIndex = attackScene->getFromIndex();
					m_attackTargetIndex = attackScene->getTargetIndex();
					Print << U"[DEBUG] AttackSelectScene データ取得: from=" << m_attackFromIndex << U" target=" << m_attackTargetIndex << U" cities.size()=" << m_cities.size();
				}
				else
				{
					Print << U"[ERROR] AttackSelectScene の dynamic_cast 失敗";
				}
			}

			// ★ ArmyConfigScene からのデータ取得を追加
			if (next == U"BattleMapScene")
			{
				auto* armyScene = dynamic_cast<ArmyConfigScene*>(m_currentScene);
				if (armyScene)
				{
					m_attackFromIndex = armyScene->getFromIndex();
					m_attackTargetIndex = armyScene->getTargetIndex();
					m_selectedLeader = armyScene->getSelectedOfficer();
					m_selectedSoldiers = armyScene->getSoldierAllocation();
				}
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
				// ★ pendingBattle から攻守の情報を取得
				if (m_gameManager->pendingBattle.isOccurring)
				{
					int atkIdx = m_gameManager->pendingBattle.atkCityIndex;
					int defIdx = m_gameManager->pendingBattle.defCityIndex;

					// 攻撃側と防御側の都市を取得
					CityData& attackerCity = m_cities[atkIdx];
					CityData& defenderCity = m_cities[defIdx];

					// プレイヤーが防御側かどうか判定
					bool isPlayerAttacker = (attackerCity.owner == m_playerFaction.name);

					// BattleSceneを作成（攻撃側/防御側を明示）
					if (isPlayerAttacker)
					{
						// プレイヤーが攻撃側（既存の攻撃戦）
						m_currentScene = new BattleScene(m_gameManager, attackerCity, defenderCity, true);
						Print << U"[攻撃戦] " << attackerCity.name << U" → " << defenderCity.name;
					}
					else
					{
						// プレイヤーが防御側（防衛戦）
						m_currentScene = new BattleScene(m_gameManager, defenderCity, attackerCity, false);
						Print << U"[防衛戦] " << defenderCity.name << U" ← " << attackerCity.name;
					}

					// 戦闘フラグをリセット
					m_gameManager->pendingBattle.isOccurring = false;
				}
				else
				{
					// pendingBattleがない場合は従来の処理（エラー回避）
					Print << U"[WARNING] pendingBattleが設定されていません";
					m_currentScene = new WorldMapScene(m_gameManager, m_playerFaction, &m_cities);
				}

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
			else if (next == U"AttackSelectScene")
			{
				if (m_selectedCityIndex >= 0 && m_selectedCityIndex < (int)m_cities.size())
				{
					m_currentScene = new AttackSelectScene(m_selectedCityIndex, &m_cities);
					m_currentSceneName = U"AttackSelectScene";
				}
				else
				{
					Print << U"[ERROR] AttackSelectScene への遷移に失敗";
					m_currentScene = new WorldMapScene(m_gameManager, m_playerFaction, &m_cities);
				}
			}
			else if (next == U"ArmyConfigScene")
			{
				// データは既に68-76行目で取得済み
				Print << U"[DEBUG] ArmyConfigScene 遷移チェック: from=" << m_attackFromIndex << U" target=" << m_attackTargetIndex << U" cities.size()=" << m_cities.size();

				if (m_attackFromIndex >= 0 && m_attackFromIndex < (int)m_cities.size() &&
					m_attackTargetIndex >= 0 && m_attackTargetIndex < (int)m_cities.size())
				{
					m_currentScene = new ArmyConfigScene(m_attackFromIndex, m_attackTargetIndex, &m_cities);
					m_currentSceneName = U"ArmyConfigScene";
					Print << U"[INFO] ArmyConfigScene へ遷移成功";
				}
				else
				{
					Print << U"[ERROR] ArmyConfigScene への遷移に失敗: インデックスが範囲外";
					m_currentScene = new WorldMapScene(m_gameManager, m_playerFaction, &m_cities);

				}

			}
			else if (next == U"BattleMapScene")
			{
				// データは既に84-94行目で取得済み
				Print << U"[DEBUG] BattleMapScene 遷移チェック: from=" << m_attackFromIndex << U" target=" << m_attackTargetIndex << U" soldiers=" << m_selectedSoldiers;
				
				if (m_attackFromIndex >= 0 && m_attackFromIndex < (int)m_cities.size() &&
					m_attackTargetIndex >= 0 && m_attackTargetIndex < (int)m_cities.size())
				{
					m_currentScene = new BattleMapScene(m_attackFromIndex, m_attackTargetIndex, &m_cities, m_selectedLeader, m_selectedSoldiers);
					m_currentSceneName = U"BattleMapScene";
					m_audio->PlayBGM(AudioManager::BGMType::Battle, 2.0);
					Print << U"[INFO] BattleMapScene へ遷移成功";
				}
				else
				{
					Print << U"[ERROR] BattleMapScene への遷移に失敗: インデックスが範囲外";
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
