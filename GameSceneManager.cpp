#include "GameSceneManager.hpp"
#include "FactionSelectScene.hpp"
#include "WorldMapScene.hpp"
#include "CityScene.hpp"
#include "AttackSelectScene.hpp"
#include "ArmyConfigScene.hpp"
#include "BattleMapScene.hpp"
#include "TurnEndScene.hpp"
#include "GameClearScene.hpp"
#include "OfficerDatabase.hpp"

// =================================================
// コンストラクタ：初期都市セットアップ
// =================================================
GameSceneManager::GameSceneManager()
{
	m_currentName = U"FactionSelectScene";
	m_currentScene = std::make_unique<FactionSelectScene>();

	// 全都市作成
	m_allCities = {
		CityData(U"洛陽", Vec2(200,180), Palette::Red,    800,600,200,80, U"曹操"),
		CityData(U"許昌", Vec2(400,240), Palette::Orange, 600,500,150,70, U"曹操"),
		CityData(U"新野", Vec2(300,380), Palette::Green,  500,400,100,75, U"劉備"),
		CityData(U"寿春", Vec2(500,400), Palette::Blue,   700,500,180,85, U"孫堅"),
		CityData(U"長安", Vec2(100,250), Palette::Purple, 900,800,250,60, U"董卓"),
		CityData(U"北海", Vec2(600,150), Palette::Cyan,   650,450,160,70, U"公孫瓚"),
		CityData(U"業",   Vec2(750,300), Palette::Yellow, 720,480,170,65, U"袁紹"),
	};

	// ========= 武将読み込み =========
	Array<Officer> all = OfficerDatabase::LoadAll();

	Array<Officer> liubeiArmy = { all[0], all[1], all[2] };
	Array<Officer> caos = { all[3], all[4], all[5] };
	Array<Officer> sons = { all[6] };
	Array<Officer> dongzhuo = { all[7] };
	Array<Officer> gongsons = { all[8] };
	Array<Officer> yuans = { all[9] };

	for (auto& city : m_allCities)
	{
		if (city.owner == U"劉備")   city.officers = liubeiArmy;
		if (city.owner == U"曹操")   city.officers = caos;
		if (city.owner == U"孫堅")   city.officers = sons;
		if (city.owner == U"董卓")   city.officers = dongzhuo;
		if (city.owner == U"公孫瓚") city.officers = gongsons;
		if (city.owner == U"袁紹")   city.officers = yuans;
	}
}

// =================================================
// 全都市制覇チェック
// =================================================
bool GameSceneManager::checkAllCitiesOwned() const
{
	for (const auto& c : m_allCities)
		if (c.owner != m_playerFaction.name)
			return false;
	return true;
}

// =================================================
// update()
// =================================================
void GameSceneManager::update()
{
	if (!m_currentScene) return;

	m_currentScene->update();
	if (!m_currentScene->isSceneEnd()) return;

	const String next = m_currentScene->nextSceneName();

	// ① 勢力選択 → ワールドマップ
	if (m_currentName == U"FactionSelectScene")
	{
		auto* fs = dynamic_cast<FactionSelectScene*>(m_currentScene.get());
		m_playerFaction = fs->getSelectedFaction();

		// ▼ 初期都市＝その勢力が最初に所有している都市
		String startCityName = U"";
		for (auto& c : m_allCities)
		{
			if (c.owner.trim() == m_playerFaction.name.trim())
			{
				startCityName = c.name;
				break;
			}
		}
		m_currentScene = std::make_unique<WorldMapScene>(
			m_playerFaction,
			&m_allCities,
			startCityName
		);
		m_currentName = U"WorldMapScene";
		return;
	}

	// ② ワールド → 都市
	if (next == U"City")
	{
		auto* wm = dynamic_cast<WorldMapScene*>(m_currentScene.get());
		int index = wm->getSelectedCityIndex();

		m_currentScene = std::make_unique<CityScene>(
			index, &m_allCities, m_advisor
		);
		m_currentName = U"CityScene";
		return;
	}

	// ③ 都市 → 攻撃先選択
	if (next == U"AttackSelectScene")
	{
		auto* cs = dynamic_cast<CityScene*>(m_currentScene.get());
		int from = cs->getCityIndex();

		m_currentScene = std::make_unique<AttackSelectScene>(from, &m_allCities);
		m_currentName = U"AttackSelectScene";
		return;
	}

	// ④ 攻撃先選択 → 部隊編成
	if (next == U"ArmyConfigScene")
	{
		auto* atk = dynamic_cast<AttackSelectScene*>(m_currentScene.get());
		int from = atk->getFromIndex();
		int target = atk->getTargetIndex();

		m_currentScene = std::make_unique<ArmyConfigScene>(
			from, target, &m_allCities
		);
		m_currentName = U"ArmyConfigScene";
		return;
	}

	// ⑤ 部隊編成 → 戦闘
	if (next == U"BattleMapScene")
	{
		auto* ac = dynamic_cast<ArmyConfigScene*>(m_currentScene.get());

		int from = ac->getFromIndex();
		int target = ac->getTargetIndex();
		Officer leader = ac->getSelectedOfficer();
		int soldiers = ac->getSoldierAllocation();

		m_currentScene = std::make_unique<BattleMapScene>(
			from, target, &m_allCities, leader, soldiers
		);
		m_currentName = U"BattleMapScene";
		return;
	}

	// ⑥ 内政 → ターン終了
	if (next == U"TurnEnd")
	{
		applyTurnGrowth();
		m_currentScene = std::make_unique<TurnEndScene>(m_advisor, m_turn);
		m_currentName = U"TurnEndScene";
		return;
	}

	// ⑦ 戦闘 → ワールドに戻る（初期都市も保持）
	if (next == U"WorldMapScene")
	{
		// 初期都市は変わらないので再計算
		String startCityName = U"";
		for (auto& c : m_allCities)
		{
			if (c.owner == m_playerFaction.name)
			{
				startCityName = c.name;
				break;
			}
		}

		m_currentScene = std::make_unique<WorldMapScene>(
			m_playerFaction,
			&m_allCities,
			startCityName
		);
		m_currentName = U"WorldMapScene";
		return;
	}

	// ⑧ 全都市制覇 → エンディング
	if (checkAllCitiesOwned())
	{
		m_currentScene = std::make_unique<GameClearScene>();
		m_currentName = U"GameClearScene";
		return;
	}
}

// =================================================
// ターン処理
// =================================================
void GameSceneManager::applyTurnGrowth()
{
	for (auto& city : m_allCities)
	{
		city.food += city.agriculture * 200;
		city.gold += city.commerce * 300;
		city.troops += city.barracks * 300;
		city.order = Min(city.order + 4, 100);
	}
	m_turn++;
}

// =================================================
// draw()
// =================================================
void GameSceneManager::draw() const
{
	if (m_currentScene)
		m_currentScene->draw();
}
