#pragma once
#include <Siv3D.hpp>
#include "GameSceneManager.hpp"
#include "FactionSelectScene.hpp"
#include "WorldMapScene.hpp"
#include "CityScene.hpp"
#include "BattleScene.hpp"
#include "TurnEndScene.hpp"
#include "AttackSelectScene.hpp"
#include "ArmyConfigScene.hpp"
#include "CityData.hpp"
#include "OfficerDatabase.hpp"
#include "Faction.hpp"
#include "BattleMapScene.hpp"
#include "GameClearScene.hpp"

// ==============================================
GameSceneManager::GameSceneManager()
{
	m_currentName = U"FactionSelectScene";
	m_currentScene = std::make_unique<FactionSelectScene>();

	// ★ 全都市を初期化（あなたの元コード）
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

	// ============================
	// ★ 勢力ごとに武将を割り当てる
	// ============================
	Array<Officer> all = OfficerDatabase::LoadAll();

	// 劉備軍（勢力ID 1）
	Array<Officer> liubeiArmy = { all[0], all[1], all[2] };

	// 曹操軍（勢力ID 2）
	Array<Officer> caos = { all[3], all[4], all[5] };

	// 孫堅軍（勢力ID 3）
	Array<Officer> sons = { all[6] };

	// 董卓軍（勢力ID 4）
	Array<Officer> dongzhuo = { all[7] };

	// 公孫瓚軍（勢力ID 5）
	Array<Officer> gongsons = { all[8] };

	// 袁紹軍（勢力ID 6）
	Array<Officer> yuans = { all[9] };

	// ============================
	// ★ 都市 owner に応じて武将配置
	// ============================
	for (auto& city : m_allCities)
	{
		if (city.owner == U"劉備")      city.officers = liubeiArmy;
		if (city.owner == U"曹操")      city.officers = caos;
		if (city.owner == U"孫堅")      city.officers = sons;
		if (city.owner == U"董卓")      city.officers = dongzhuo;
		if (city.owner == U"公孫瓚")    city.officers = gongsons;
		if (city.owner == U"袁紹")      city.officers = yuans;
	}
}
bool GameSceneManager::checkAllCitiesOwned() const
{
	for (const auto& c : m_allCities)
	{
		if (c.owner != m_playerFaction.name)
			return false;
	}
	return true;
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
	// 都市 → 侵攻（AttackSelectScene）
	if (next == U"AttackSelectScene")
	{
		auto* city = dynamic_cast<CityScene*>(m_currentScene.get());
		if (city)
		{
			CityData fromCity = city->getCity();

			// ★ 引数は 2 つ必要
			m_currentScene = std::make_unique<AttackSelectScene>(
				fromCity,
				m_allCities
			);

			m_currentName = U"AttackSelectScene";
		}
		return;
	}
	// 侵攻先 → 部隊編成
	if (next == U"ArmyConfigScene")
	{
		auto* atk = dynamic_cast<AttackSelectScene*>(m_currentScene.get());
		if (atk)
		{
			CityData fromCity = atk->getFromCity();
			CityData targetCity = atk->getTargetCity();

			m_currentScene = std::make_unique<ArmyConfigScene>(fromCity, targetCity);
			m_currentName = U"ArmyConfigScene";
		}
		return;
	}

	// 都市 → 戦闘
	if (next == U"BattleMapScene")
	{
		auto* ac = dynamic_cast<ArmyConfigScene*>(m_currentScene.get());
		if (ac)
		{
			CityData fromCity = ac->getFromCity();
			CityData targetCity = ac->getTargetCity();
			Officer leader = ac->getSelectedOfficer();
			int soldiers = ac->getSoldierAllocation();

			m_currentScene = std::make_unique<BattleMapScene>(
				fromCity, targetCity, leader, soldiers
			);
			m_currentName = U"BattleMapScene";
		}
		return;
	}




	// 都市 → ターン終了 → マップ
	if (next == U"TurnEnd")
	{
		applyTurnGrowth();

		m_currentScene = std::make_unique<TurnEndScene>(m_advisor, m_turn);
		m_currentName = U"TurnEndScene";
		return;
	}


	// 戦闘 → マップ
	if (next == U"WorldMapScene")
	{
		m_currentScene = std::make_unique<WorldMapScene>(m_playerFaction, m_allCities);
		m_currentName = U"WorldMapScene";
		return;
	}
	// ★全都市制覇チェック
	if (checkAllCitiesOwned())
	{
		m_currentScene = std::make_unique<GameClearScene>();
		m_currentName = U"GameClearScene";
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
