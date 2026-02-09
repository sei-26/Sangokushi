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
		if (m_selectedCityIndex < 0 || m_selectedCityIndex >= m_cities.size())
		{
			// ログを出力（デバッグ用）
			Console << U"Error: Invalid City Index: " << m_selectedCityIndex;
			m_currentScene = new WorldMapScene(&m_gameManager, m_playerFaction, &m_cities);
			return;
		}

		// 2. 攻撃側データの取得（安全確認済み）
		CityData* attacker = &m_cities[m_selectedCityIndex];

		// 3. 防御側データの検索
		CityData* defender = nullptr;
		for (auto& city : m_cities)
		{
			if (city.owner != m_playerFaction.name)
			{
				defender = &city;
				break;
			}
		}

		// 敵がいない場合の保険（ここも重要）
		static CityData dummyEnemy(U"賊軍", Point(0, 0), U"賊");
		if (defender == nullptr) defender = &dummyEnemy;

		// 4. シーン遷移
		m_currentScene = new BattleScene(&m_gameManager, *attacker, *defender);
	}

}
