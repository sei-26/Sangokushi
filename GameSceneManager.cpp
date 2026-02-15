#include "GameSceneManager.hpp"
#include "WorldMapScene.hpp"
#include "CityScene.hpp"
#include "BattleScene.hpp"
#include "DiplomacyScene.hpp"

GameSceneManager::GameSceneManager(GameManager* gm, const Faction& playerFaction, const Array<CityData>& cities)
	: m_currentScene(nullptr)
	, m_gameManager(gm)
	, m_playerFaction(playerFaction)
	, m_cities(cities)
{
	// 初期シーンはワールドマップ
	m_currentScene = new WorldMapScene(m_gameManager, m_playerFaction, &m_cities);
}

GameSceneManager::~GameSceneManager()
{
	if (m_currentScene) delete m_currentScene;
}

void GameSceneManager::update()
{
	if (m_currentScene)
	{
		m_currentScene->update();

		if (m_currentScene->isSceneEnd())
		{
			String next = m_currentScene->getNextScene();

			// マップから都市へ行くとき、選ばれた都市番号を保存
			if (next == U"City")
			{
				auto* mapScene = dynamic_cast<WorldMapScene*>(m_currentScene);
				if (mapScene)
				{
					m_selectedCityIndex = mapScene->getSelectedCityIndex();
				}
			}
			changeScene(next);
		}
	}
}

void GameSceneManager::draw() const
{
	if (m_currentScene) m_currentScene->draw();
}

void GameSceneManager::changeScene(String nextScene, bool stackClear)
{
	(void)stackClear;

	if (m_currentScene)
	{
		delete m_currentScene;
		m_currentScene = nullptr;
	}

	if (nextScene == U"WorldMap")
	{
		// プレイヤーの都市が残っているか確認
		bool isAlive = false;
		for (const auto& city : m_cities)
		{
			if (city.owner == m_playerFaction.name)
			{
				isAlive = true;
				break;
			}
		}

		if (!isAlive)
		{
			System::MessageBoxOK(U"あなたの勢力は滅亡しました。", MessageBoxStyle::Error);
			System::Exit();
			return;
		}

		m_currentScene = new WorldMapScene(m_gameManager, m_playerFaction, &m_cities);
	}
	else if (nextScene == U"City")
	{
		if (m_selectedCityIndex >= 0 && m_selectedCityIndex < m_cities.size())
		{
			m_currentScene = new CityScene(m_gameManager, m_cities[m_selectedCityIndex]);
		}
		else
		{
			// エラー回避
			m_currentScene = new WorldMapScene(m_gameManager, m_playerFaction, &m_cities);
		}
	}
	else if (nextScene == U"Battle")
	{
		if (m_selectedCityIndex < 0 || m_selectedCityIndex >= m_cities.size())
		{
			Print << U"[ERROR] 無効な都市インデックス: " << m_selectedCityIndex;
			m_currentScene = new WorldMapScene(m_gameManager, m_playerFaction, &m_cities);
			return;
		}

		// 攻撃側データ
		CityData* attacker = &m_cities[m_selectedCityIndex];

		// 防御側データの検索（最初の敵都市）
		CityData* defender = nullptr;
		for (auto& city : m_cities)
		{
			if (city.owner != m_playerFaction.name)
			{
				defender = &city;
				break;
			}
		}

		// 敵がいない場合の保険
		static CityData dummyEnemy(U"賊軍", Point(0, 0), U"賊");
		if (defender == nullptr)
		{
			defender = &dummyEnemy;
		}

		// 武将選択（とりあえず最初の武将）
		Officer selectedOfficer;
		if (!attacker->officers.isEmpty())
		{
			selectedOfficer = attacker->officers[0];
		}

		// シーン遷移
		m_currentScene = new BattleScene(m_gameManager, *attacker, *defender, true);
	}
	else if (nextScene == U"Diplomacy")
	{
		// ★ 外交シーンへ遷移
		m_currentScene = new DiplomacyScene(&m_gameManager->diplomacy, m_playerFaction.name, &m_cities, m_gameManager);
	}
}
