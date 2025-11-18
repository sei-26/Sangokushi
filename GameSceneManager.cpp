#include "GameSceneManager.hpp"
#include "FactionSelectScene.hpp"
#include "WorldMapScene.hpp"
#include "CityScene.hpp"
#include "BattleScene.hpp"

GameSceneManager::GameSceneManager()
{
	m_currentName = U"FactionSelectScene";
	m_currentScene = std::make_unique<FactionSelectScene>();

	// ★ 軍師初期化
	m_advisor.name = U"諸葛亮";
	m_advisor.agricultureBonus = 20;
	m_advisor.commerceBonus = 15;
	m_advisor.orderBonus = 10;

	// ★ 全都市を初期化（絶対必要）
	m_allCities = {
		CityData(U"洛陽", Vec2(200,180), Palette::Red,
				 800,600,200,80, U"曹操"),
		CityData(U"許昌", Vec2(400,240), Palette::Orange,
				 600,500,150,70, U"夏侯惇"),
		CityData(U"新野", Vec2(300,380), Palette::Green,
				 500,400,100,75, U"劉備"),
		CityData(U"寿春", Vec2(500,400), Palette::Blue,
				 700,500,180,85, U"孫堅"),
		CityData(U"長安", Vec2(100,250), Palette::Purple,
				 900,800,250,60, U"董卓"),
		CityData(U"北海", Vec2(600,150), Palette::Cyan,
				 650,450,160,70, U"公孫瓚"),
		CityData(U"業", Vec2(750,300), Palette::Yellow,
				 720,480,170,65, U"袁紹"),
	};
}

void GameSceneManager::update()
{
	if (!m_currentScene) return;

	m_currentScene->update();
	if (!m_currentScene->isSceneEnd()) return;

	const String next = m_currentScene->nextSceneName();

	// 勢力選択 → ワールドマップ
	if (m_currentName == U"FactionSelectScene")
	{
		auto* fs = dynamic_cast<FactionSelectScene*>(m_currentScene.get());
		m_playerFaction = fs->getSelectedFaction();

		m_currentScene = std::make_unique<WorldMapScene>(m_playerFaction, m_allCities);
		m_currentName = U"WorldMapScene";
		return;
	}

	// ワールド → 都市
	if (next == U"City")
	{
		auto* wm = dynamic_cast<WorldMapScene*>(m_currentScene.get());
		if (wm && wm->getSelectedCity().has_value())
		{
			CityData city = wm->getSelectedCity().value();
			m_currentScene = std::make_unique<CityScene>(city, m_advisor);
			m_currentName = U"CityScene";
		}
		return;
	}

	// 都市 → 戦闘
	if (next == U"BattleScene")
	{
		m_currentScene = std::make_unique<BattleScene>();
		m_currentName = U"BattleScene";
		return;
	}

	// 都市 → ターン終了 → マップ
	if (next == U"TurnEnd")
	{
		applyTurnGrowth();  // ★ 全都市成長
		m_currentScene = std::make_unique<WorldMapScene>(m_playerFaction, m_allCities);
		m_currentName = U"WorldMapScene";
		return;
	}

	// 戦闘 → マップ
	if (next == U"WorldMapScene")
	{
		m_currentScene = std::make_unique<WorldMapScene>(m_playerFaction, m_allCities);
		m_currentName = U"WorldMapScene";
		return;
	}
}

void GameSceneManager::applyTurnGrowth()
{
	for (auto& city : m_allCities)
	{
		city.food += city.agriculture * 200;
		city.gold += city.commerce * 300;
		city.troops += city.barracks * 300;
		city.order = Max(city.order + 4, 0);
	}

	m_turn++;
}

void GameSceneManager::draw() const
{
	if (m_currentScene)
	{
		m_currentScene->draw();
	}
}
