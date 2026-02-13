#include "GameSceneManager.hpp"
#include "WorldMapScene.hpp"
#include "CityScene.hpp"
#include "BattleScene.hpp"

GameSceneManager::GameSceneManager()
	: m_currentScene(nullptr)
{
	m_playerFaction.name = U"劉備";
	m_playerFaction.color = Palette::Green;

	m_cities.clear();
	// 都市データを作成
	m_cities.push_back(CityData(U"襄平", Point(1400, 300), U"公孫度"));
	m_cities.push_back(CityData(U"北平", Point(1100, 350), U"公孫瓚"));
	m_cities.push_back(CityData(U"代県", Point(900, 400), U"劉虞"));

	if (!m_cities.isEmpty())
	{
		m_cities[0].owner = m_playerFaction.name;
	}

	m_currentScene = new WorldMapScene(&m_gameManager, m_playerFaction, &m_cities);
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
		bool isAlive = false;
		for (const auto& city : m_cities)
		{
			{
				if (city.owner == m_playerFaction.name)
				{
					isAlive = true;
					break;
				}
			}
		}
		if (!isAlive)
		{
			System::MessageBoxOK(U"あなたの勢力は滅亡しました。ゲームオーバーです。", MessageBoxStyle::Error);
			System::Exit();
			return;
		}
		m_currentScene = new WorldMapScene(&m_gameManager, m_playerFaction, &m_cities);

	}
	else if (nextScene == U"City")
	{
		// ★ ここで「2つの引数」を渡して CityScene を作っています
		if (m_cities.size() > m_selectedCityIndex)
		{
			m_currentScene = new CityScene(&m_gameManager, m_cities[m_selectedCityIndex]);
		}
		else
		{
			// エラー回避：もしインデックスがおかしければマップに戻す
			m_currentScene = new WorldMapScene(&m_gameManager, m_playerFaction, &m_cities);
		}

	}
	else if (nextScene == U"Battle")
	{
		CityData* pPlayerCity = nullptr;
		CityData* pEnemyCity = nullptr;
		bool isPlayerAttacker = true;

		// ========================================================
		// パターンA：防衛戦イベント（敵から攻められた）
		// ========================================================
		if (m_gameManager.pendingBattle.isOccurring)
		{
			int atkIdx = m_gameManager.pendingBattle.atkCityIndex;
			int defIdx = m_gameManager.pendingBattle.defCityIndex;

			// 安全性チェック
			if (atkIdx < 0 || atkIdx >= static_cast<int>(m_cities.size()) ||
				defIdx < 0 || defIdx >= static_cast<int>(m_cities.size()))
			{
				Print << U"[ERROR] 無効な都市インデックス: atk=" << atkIdx
					<< U", def=" << defIdx << U", size=" << m_cities.size();
				m_gameManager.pendingBattle.isOccurring = false;
				m_currentScene = new WorldMapScene(&m_gameManager, m_playerFaction, &m_cities);
				return;
			}

			// 防衛戦なので、プレイヤーは防御側
			pEnemyCity = &m_cities[atkIdx];  // 敵（攻め）
			pPlayerCity = &m_cities[defIdx]; // 自分（守り）
			isPlayerAttacker = false;

			Print << U"[防衛戦] " << pEnemyCity->name << U"(" << pEnemyCity->troops
				<< U"兵) → " << pPlayerCity->name << U"(" << pPlayerCity->troops << U"兵)";

			// イベントフラグをリセット
			m_gameManager.pendingBattle.isOccurring = false;
		}
		// ========================================================
		// パターンB：通常攻撃（プレイヤーが攻める）
		// ========================================================
		else
		{
			// 安全性チェック
			if (m_selectedCityIndex < 0 || m_selectedCityIndex >= static_cast<int>(m_cities.size()))
			{
				Print << U"[ERROR] 無効な選択都市: " << m_selectedCityIndex;
				m_currentScene = new WorldMapScene(&m_gameManager, m_playerFaction, &m_cities);
				return;
			}

			pPlayerCity = &m_cities[m_selectedCityIndex];

			// 兵数チェック
			if (pPlayerCity->troops <= 0)
			{
				Print << U"[ERROR] 兵が不足しています";
				m_currentScene = new WorldMapScene(&m_gameManager, m_playerFaction, &m_cities);
				return;
			}

			isPlayerAttacker = true;

			// 敵都市を探す
			for (auto& city : m_cities)
			{
				if (city.owner != m_playerFaction.name && city.troops > 0)
				{
					pEnemyCity = &city;
					break;
				}
			}

			if (pEnemyCity == nullptr)
			{
				Print << U"[INFO] 攻撃可能な敵都市がありません";
				m_currentScene = new WorldMapScene(&m_gameManager, m_playerFaction, &m_cities);
				return;
			}

			Console << U"[出陣] " << pPlayerCity->name << U"(" << pPlayerCity->troops
				<< U"兵) → " << pEnemyCity->name << U"(" << pEnemyCity->troops << U"兵)";
		}

		// ========================================================
		// 最終安全チェック
		// ========================================================
		if (pPlayerCity == nullptr || pEnemyCity == nullptr)
		{
			Print << U"[ERROR] 都市データが無効です";
			m_currentScene = new WorldMapScene(&m_gameManager, m_playerFaction, &m_cities);
			return;
		}

		// ========================================================
		// BattleScene を作成（4引数バージョン）
		// ========================================================
		m_currentScene = new BattleScene(&m_gameManager, *pPlayerCity, *pEnemyCity, isPlayerAttacker);
	}

}
