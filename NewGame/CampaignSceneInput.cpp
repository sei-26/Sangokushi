#include "CampaignScene.hpp"
#include "CampaignSave.hpp"
#include "CampaignUI.hpp"

using namespace frontline;
using namespace campaignui;

void CampaignScene::deploy(Arm arm)
{
	const auto available = m_game.Available(m_city);
	if (available.empty())
	{
		m_message = U"出陣できる武将がいません。帰還・負傷回復を待ってください。";
		return;
	}
	m_generalChoice %= static_cast<int>(available.size());
	const int army = m_game.Deploy(m_city, available[m_generalChoice], m_soldiers, arm);
	if (army < 0)
	{
		m_message = U"守備兵1000を残す兵力・金・糧・命令が必要です。出陣上限は6隊。";
		return;
	}
	m_army = army;
	m_city = -1;
	m_message = U"右クリックで進路を指定。赤い都市へ進むと包囲・攻城を開始します。";
}

void CampaignScene::update()
{
	if (m_storyActive)
	{
		m_story.update();
		if (m_story.Back())
			m_storyActive = false;
		return;
	}
	if (!m_started)
	{
		const int start = Scene::Center().x - 465;
		for (int f = 0; f < 3; ++f)
			if (Rect(start + f * 320, Scene::Center().y - 20, 290, 155).leftClicked())
			{
				m_game.Reset(f);
				m_region = -1;
				fitMap();
				m_mapTexture = RenderTexture{};
				m_started = true;
				m_city = f * 3;
				m_army = -1;
				m_generalChoice = 0;
				m_positions.clear();
				m_hits.clear();
				return;
			}
		if (Rect(Scene::Center().x - 225, Scene::Center().y + 165, 450, 38).leftClicked())
		{
			Campaign loaded;
			if (LoadJSON(JSON::Load(U"frontline-save.json"), loaded))
			{
				m_game = std::move(loaded);
				m_region = -1;
				m_transportEstimateDay = -1;
				fitMap();
				m_started = true;
				m_city = -1;
				m_army = -1;
				m_positions.clear();
				m_hits.clear();
				m_mapTexture = RenderTexture{};
				m_message = U"戦況を復元しました。";
			}
			else
				m_message = U"新作のセーブが見つかりません。勢力を選んで始めてください。";
		}
		return;
	}
	if (KeyTab.down())
	{
		m_supplyView = !m_supplyView;
		m_mapDay = -1;
	}
	if (m_councilMode != 0)
	{
		updateCouncil();
		return;
	}
	if (KeyV.down() || button(49).leftClicked())
		m_panelHidden = !m_panelHidden;
	updateCamera();
	if ((button(17).leftClicked() && (!m_game.hexMap || m_daysLeft == 0)) || button(18).leftClicked())
	{
		m_councilMode = button(17).leftClicked() ? 1 : 2;
		m_paused = m_daysLeft > 0;
		m_rosterFaction = m_game.player;
		m_inspect = m_game.player * 6;
		m_partner = -1;
		m_historyPage = 0;
		if (m_army >= 0 && m_game.armies[m_army].faction == m_game.player)
			m_inspect = m_game.armies[m_army].general;
		else if ((!m_game.hexMap || m_daysLeft == 0) && m_city >= 0 &&
		         m_game.cities[m_city].owner == m_game.player)
			for (int g = 0; g < static_cast<int>(m_game.generals.size()); ++g)
				if (m_game.generals[g].home == m_city && m_game.generals[g].faction == m_game.player)
				{
					m_inspect = g;
					break;
				}
		return;
	}
	if (button(15).leftClicked())
		m_speed = (m_speed + 1) % 3;
	if (m_daysLeft > 0 && (KeySpace.down() || button(0).leftClicked()))
		m_paused = !m_paused;
	if (m_city >= 0 && button(13).leftClicked())
		m_tab = 0;
	if (m_city >= 0 && button(14).leftClicked())
		m_tab = 1;
	if (m_city >= 0 && button(40).leftClicked())
		m_tab = 2;
	if (m_daysLeft > 0 && !m_paused)
	{
		m_timer += Scene::DeltaTime();
		const double interval = m_speed == 0 ? 1.0 : m_speed == 1 ? 0.5 : 0.2;
		if (m_timer >= interval)
		{
			m_timer -= interval;
			std::vector<int> troops;
			for (const auto& army : m_game.armies)
				troops.push_back(army.troops);
			m_game.AdvanceDay();
			--m_daysLeft;
			for (int i = 0; i < static_cast<int>(m_game.armies.size()); ++i)
			{
				const auto& army = m_game.armies[i];
				if (troops[i] > army.troops &&
				    (army.troops > 0 || m_game.generals[army.general].readyDay == m_game.day + 20))
					m_hits.push_back({tileCenter(army.tile), troops[i] - army.troops, 0.75});
			}
			if (m_game.result != 0)
				m_daysLeft = 0;
			if (m_daysLeft == 0)
				m_message = U"命令の時間です。新たな進路・出陣・補給を検討してください。";
		}
	}
	else
	{
		if (m_daysLeft == 0 && button(12).leftClicked())
		{
			m_started = false;
			m_army = -1;
			m_city = -1;
			return;
		}
		if (m_daysLeft == 0 && button(10).leftClicked())
		{
			try
			{
				m_message = SaveJSON(m_game).save(U"frontline-save.json") ? U"新作の戦況を保存しました。"
				                                                          : U"保存できませんでした。";
			}
			catch (...)
			{
				m_message = U"保存できませんでした。";
			}
		}
		if (m_daysLeft == 0 && button(11).leftClicked())
		{
			Campaign loaded;
			if (LoadJSON(JSON::Load(U"frontline-save.json"), loaded))
			{
				m_game = std::move(loaded);
				m_region = -1;
				m_transportEstimateDay = -1;
				fitMap();
				m_city = -1;
				m_army = -1;
				m_positions.clear();
				m_hits.clear();
				m_mapDay = -1;
				m_mapTexture = RenderTexture{};
				m_message = U"戦況を復元しました。";
			}
			else
				m_message = U"有効な新作のセーブが見つかりません。";
		}
		if (m_game.result == 0)
		{
			const int tile = mouseTile();
			if (MouseL.down() && Campaign::Valid(tile))
			{
				m_panelHidden = false;
				m_region = -1;
				const int city = m_game.CityAt(tile);
				std::vector<int> units;
				for (int i = 0; i < static_cast<int>(m_game.armies.size()); ++i)
					if (m_game.armies[i].troops > 0 && m_game.armies[i].tile == tile)
						units.push_back(i);
				if (city >= 0 && (KeyShift.pressed() || units.empty()))
				{
					m_city = city;
					m_army = -1;
					m_generalChoice = 0;
				}
				else if (!units.empty())
				{
					auto it = std::find(units.begin(), units.end(), m_army);
					m_army = it != units.end() && ++it != units.end() ? *it : units.front();
					m_city = -1;
				}
				else
				{
					m_region = m_game.RegionAt(tile);
					m_city = -1;
					m_army = -1;
				}
			}
			if ((!m_game.hexMap || m_daysLeft == 0) && m_army >= 0 &&
			    m_army < static_cast<int>(m_game.armies.size()) && m_game.armies[m_army].troops > 0 &&
			    m_game.armies[m_army].faction == m_game.player)
			{
				if (MouseR.down() && Campaign::Valid(tile))
					m_message = m_game.Order(m_army, tile) ? U"進路を更新。進行開始で全勢力が同時に動きます。"
					                                       : U"その場所へは到達できません。";
				updateReturnOrders();
				if (button(9).leftClicked())
				{
					m_game.Order(m_army, m_game.armies[m_army].tile);
					m_message = m_game.armies[m_army].arm == Arm::Transport
					                ? U"輸送を停止。右クリックで味方都市へ配送先を変更できます。"
					                : U"現在地を守備します。近くの敵には自動で応戦。";
				}
				if (button(43).leftClicked() && m_game.armies[m_army].arm != Arm::Transport)
					m_message = m_game.SetStance(
					                m_army, static_cast<battle::Stance>(
					                            (static_cast<int>(m_game.armies[m_army].stance) + 1) % 3))
					                ? U"構えを変更。命令1を使用。攻勢は高火力・高損耗、固守は低火力・低損耗・"
					                  U"低速です。"
					                : U"構えの変更には命令1が必要です。";
				if (button(39).leftClicked())
					m_message = m_game.ActivateTactic(m_army)
					                ? U"戦法を予約。次の1日に発動します。"
					                : U"士気30以上・再使用待ちの終了・対象や適正兵科が必要です。";
			}
			else if ((!m_game.hexMap || m_daysLeft == 0) && m_city >= 0 &&
			         m_game.cities[m_city].owner == m_game.player)
			{
				if (button(1).leftClicked())
					++m_generalChoice;
				if (m_tab == 2)
					updateTransport();
				else if (m_tab == 1)
				{
					const auto available = m_game.Available(m_city);
					for (int d = 0; d < 4; ++d)
						if (button(3 + d).leftClicked())
						{
							m_message =
							    !available.empty() &&
							            m_game.Develop(m_city, available[m_generalChoice % available.size()],
							                           static_cast<Duty>(d))
							        ? U"内政開始。金500・命令1を使用。30日間は担当武将が出陣できません。"
							        : U"空いた武将・金500・命令1が必要。都市の事業は同時に1つ、能力上限100。";
						}
					if (button(7).leftClicked())
						m_message = m_game.CancelWork(m_city) ? U"事業を中止。支払った金は戻りません。"
						                                      : U"進行中の事業がありません。";
				}
				else
				{
					if (button(2).leftClicked())
						m_soldiers = m_soldiers == 6000 ? 1500 : m_soldiers + 1500;
					if (button(3).leftClicked())
						deploy(Arm::Spear);
					else if (button(4).leftClicked())
						deploy(Arm::Bow);
					else if (button(5).leftClicked())
						deploy(Arm::Siege);
					else if (button(6).leftClicked())
						deploy(Arm::Cavalry);
					else if (button(7).leftClicked())
						m_message = m_game.Recruit(m_city)
						                ? U"兵2000を募集。金300・糧1000・治安10を使用。"
						                : U"金300・糧1000・命令1・治安35以上が必要。兵力上限18000。";
				}
			}
			if (m_daysLeft == 0 && (button(0).leftClicked() || KeyEnter.down() || button(16).leftClicked()))
			{
				m_game.BeginTurn();
				m_daysLeft = (!m_game.hexMap && button(16).leftClicked()) ? 1 : 10 - m_game.day % 10;
				m_timer = 0;
				m_paused = false;
				m_message = U"進行中。Spaceで一時停止し、作戦を見直せます。";
			}
		}
	}
	while (m_positions.size() < m_game.armies.size())
		m_positions.push_back(tileCenter(m_game.armies[m_positions.size()].tile));
	for (size_t i = 0; i < m_positions.size(); ++i)
		m_positions[i] =
		    m_positions[i].lerp(tileCenter(m_game.armies[i].tile), Min(1.0, Scene::DeltaTime() * 18));
	for (auto& hit : m_hits)
		hit.time -= Scene::DeltaTime();
	m_hits.remove_if([](const Hit& hit) { return hit.time <= 0; });
}
