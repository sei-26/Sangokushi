#include "StrategistWarScene.hpp"
#include "CampaignUI.hpp"

using namespace frontline;
using namespace campaignui;

void StrategistWarScene::Open()
{
	m_battle.Reset();
	m_back = false;
	m_council = true;
	m_paused = false;
	m_selected = m_daysLeft = 0;
	m_timer = 0;
	m_operationOpen = false;
	m_pickPurpose = 0;
	m_message = U"軍議で方針を決めよう。";
}
RectF StrategistWarScene::MapRect() const
{
	return RectF(300, 118, Scene::Width() - 584, Scene::Height() - 230);
}
double StrategistWarScene::Cell() const
{
	const auto rect = MapRect();
	return Min(rect.w / (StrategistBattle::MapColumns + 1.),
	           rect.h / ((StrategistBattle::MapRows + 1.) * hexgrid::Row));
}
Vec2 StrategistWarScene::Position(int tile) const
{
	const auto center = hexgrid::Center(tile % Width, tile / Width);
	return MapRect().center() + Vec2(center.first - 12.25, center.second - 9 * hexgrid::Row) * Cell();
}
int StrategistWarScene::Pick(Vec2 position) const
{
	if (!MapRect().contains(position))
		return -1;
	const auto offset = (position - MapRect().center()) / Cell() + Vec2(12.25, 9 * hexgrid::Row);
	const auto picked = hexgrid::Pick(offset.x, offset.y);
	const int tile = Campaign::At(picked.first, picked.second);
	return StrategistBattle::OnMap(tile) && m_battle.world.tiles[tile].terrain != Terrain::Sea ? tile : -1;
}
Rect StrategistWarScene::Control(int id) const
{
	const int right = Scene::Width() - 264;
	if (id == 0)
		return Rect(right, Scene::Height() - 94, 244, 36);
	if (id == 1)
		return Rect(right, 426, 244, 34);
	if (id == 2)
		return Rect(right, 468, 118, 32);
	if (id == 3)
		return Rect(right + 126, 468, 118, 32);
	if (id == 4)
		return Rect(Scene::Width() - 130, 22, 110, 32);
	if (id == 5)
		return Rect(right, 511, 244, 32);
	if (id == 6)
		return Rect(right, 552, 244, 32);
	return Rect(-1000, -1000, 1, 1);
}
void StrategistWarScene::Update()
{
	m_music.Input(Vec2(-1000, -1000), 26);
	m_music.Update(m_battle.outcome == 1   ? musiccues::Cue::Victory
	               : m_battle.outcome == 2 ? musiccues::Cue::Storm
	               : m_council             ? musiccues::Cue::Journey
	                                       : musiccues::Cue::Battle,
	               Scene::DeltaTime(), false);
	if (KeyEscape.down() && (m_operationOpen || m_pickPurpose != 0))
	{
		m_operationOpen = false;
		m_pickPurpose = 0;
		return;
	}
	if (Control(4).leftClicked() || KeyEscape.down())
	{
		m_music.Stop();
		m_back = true;
		return;
	}
	if (m_battle.outcome)
	{
		if (Rect(Scene::Center().x - 160, Scene::Center().y + 170, 320, 38).leftClicked())
		{
			const int enemyPolicy = m_battle.enemyPolicy, plan = m_battle.plan;
			const bool intelligence = m_battle.intelligenceEnabled;
			Open();
			m_battle.enemyPolicy = enemyPolicy;
			m_battle.plan = plan;
			m_battle.intelligenceEnabled = intelligence;
			m_battle.RefreshIntelligence();
			m_message = U"同じ敵方針・観測ルールで再挑戦。軍議で別の方針も選べる。";
		}
		return;
	}
	if (m_council)
	{
		for (int i = 0; i < 3; ++i)
			if (Rect(Scene::Center().x - 435 + i * 295, Scene::Center().y - 45, 280, 155).leftClicked())
				m_battle.plan = i;
		if (Rect(Scene::Center().x - 310, Scene::Center().y + 123, 620, 34).leftClicked())
			m_battle.enemyPolicy = (m_battle.enemyPolicy + 1) % 3;
		if (Rect(Scene::Center().x - 310, Scene::Center().y + 164, 620, 28).leftClicked())
		{
			m_battle.intelligenceEnabled = !m_battle.intelligenceEnabled;
			for (auto& known : m_battle.intelligence)
				known.Reset(m_battle.world);
			m_battle.RefreshIntelligence();
		}
		if (Rect(Scene::Center().x - 160, Scene::Center().y + 201, 320, 42).leftClicked())
		{
			m_battle.SetPlan(m_battle.plan);
			m_council = false;
			m_message = U"各隊の配置を確認。輸送隊の出発時期も自分で決めよう。";
		}
		return;
	}
	for (int i = 0; i < 5; ++i)
		if (!m_operationOpen && m_pickPurpose == 0 && Rect(20, 159 + i * 72, 262, 66).leftClicked())
			m_selected = i;
	if (m_operationOpen)
	{
		UpdateOperation();
		return;
	}
	if (m_pickPurpose != 0)
	{
		const int tile = Pick(Cursor::PosF());
		if (MouseR.down())
		{
			m_pickPurpose = 0;
			m_operationOpen = true;
		}
		else if (MouseL.down() && tile >= 0)
		{
			if (m_pickPurpose == 1)
				m_draft.target = tile;
			if (m_pickPurpose == 2)
				m_draft.fallback = tile;
			if (m_pickPurpose == 3)
				m_draft.watchTile = tile;
			if (m_pickPurpose == 4)
				m_draft.signalTile = tile;
			m_pickPurpose = 0;
			m_operationOpen = true;
		}
		return;
	}
	if (m_daysLeft > 0)
	{
		if (Control(0).leftClicked() || KeySpace.down())
			m_paused = !m_paused;
		if (!m_paused)
		{
			m_timer += Scene::DeltaTime();
			if (m_timer >= 1.)
			{
				m_timer -= 1.;
				m_battle.AdvanceDay();
				--m_daysLeft;
				if (m_daysLeft == 0)
					m_message = U"再軍議。戦況と損耗を確認して、次の10日の命令を決めよう。";
			}
		}
		return;
	}
	const int tile = Pick(Cursor::PosF());
	if ((Control(6).leftClicked() || KeyP.down()) && m_battle.world.armies[m_selected].troops > 0)
	{
		OpenOperation();
		return;
	}
	if (MouseL.down() && tile >= 0)
		for (int i = 0; i < 5; ++i)
			if (m_battle.world.armies[i].tile == tile && m_battle.world.armies[i].troops > 0)
				m_selected = i;
	if (MouseR.down() && tile >= 0)
		m_message = m_battle.Order(m_selected, tile)
		                ? U"直接命令に変更。予約を解除し、次の進行から動く。"
		                : U"命令できません。輸送隊は味方拠点へ配送し、停止は現在地で指示します。";
	if (Control(1).leftClicked())
	{
		const auto& world = m_battle.world;
		m_message = m_battle.Order(StrategistBattle::Convoy, world.cities[StrategistBattle::Destination].tile)
		                ? U"輸送隊へ出発命令。積荷を守り、前線拠点まで護送しよう。"
		                : U"輸送経路が遮断されています。味方の支配と敵の接近を確認してください。";
	}
	if (Control(2).leftClicked())
		m_message = m_battle.Order(m_selected, m_battle.world.armies[m_selected].tile)
		                ? U"現在地で待機。接近した敵には応戦する。"
		                : U"この隊は待機命令を受けられません。";
	if (Control(3).leftClicked())
		m_message = m_battle.Order(m_selected, m_battle.world.cities[0].tile, true)
		                ? U"後方へ撤退。帰還した戦闘兵も温存兵に含まれる。"
		                : U"後方への経路を確保してください。";
	if (Control(0).leftClicked() || KeySpace.down())
	{
		m_daysLeft = Min(10, StrategistBattle::Deadline - m_battle.world.day);
		m_paused = false;
		m_timer = 0;
		m_message = U"全隊が進行中。命令の変更は次の再軍議で。Spaceで停止して戦況を確認できる。";
	}
	if (m_daysLeft == 0 && Control(5).leftClicked())
	{
		const auto stance =
		    static_cast<battle::Stance>((static_cast<int>(m_battle.world.armies[m_selected].stance) + 1) % 3);
		if (m_battle.SetStance(m_selected, stance))
			m_message = U"構えを変更。固守は被害を抑えるが、攻撃と行軍が遅くなる。";
	}
}
void StrategistWarScene::Preview(int mode)
{
	Open();
	if (m_music.Enabled())
		m_music.Toggle();
	if (mode >= 70)
	{
		m_battle.SetPlan(0);
		m_council = false;
		m_battle.Order(StrategistBattle::Convoy, m_battle.world.cities[StrategistBattle::Destination].tile);
		m_message = U"右クリックで進路 / 左の名簿で担当選択 / 10日進行で作戦を実行";
	}
	if (mode == 72)
	{
		for (int day = 0; day < 45 && !m_battle.outcome; ++day)
			m_battle.AdvanceDay();
	}
	if (mode == 74 || mode == 76)
	{
		m_selected = 2;
		OpenOperation();
		m_draft.trigger = OperationTrigger::AllyArrival;
		m_draft.signalArmy = 0;
		m_draft.signalTile = m_battle.world.armies[0].target;
		m_message = U"関羽の到着を合図に予備隊を動かす。損耗したら後方へ戻す作戦。";
	}
	if (mode == 75)
	{
		BattleOperation scout;
		scout.target = Campaign::At(14, 8);
		scout.scout = true;
		m_battle.QueueOperation(1, scout);
		m_selected = 1;
		for (int day = 0; day < 10 && !m_battle.outcome; ++day)
			m_battle.AdvanceDay();
	}
}
