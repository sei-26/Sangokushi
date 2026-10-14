#include "StrategistJourneyScene.hpp"

using namespace frontline;

void StrategistJourneyScene::Open()
{
	if (!m_initialized)
	{
		m_journey.Reset();
		m_initialized = true;
		m_fx.LoadAudio();
	}
	m_back = false;
}
RectF StrategistJourneyScene::MapRect() const
{
	return RectF(240, 128, Scene::Width() - 558, Scene::Height() - 244);
}
double StrategistJourneyScene::Cell() const
{
	const auto r = MapRect();
	return m_zoom * Min(r.w / (StrategistJourney::Columns + 1.),
	                    r.h / ((StrategistJourney::Rows + 1.) * hexgrid::Row));
}
Vec2 StrategistJourneyScene::Position(int tile) const
{
	const auto p = hexgrid::Center(tile % Width, tile / Width);
	return MapRect().center() + (Vec2(p.first, p.second) - m_camera) * Cell();
}
int StrategistJourneyScene::Pick(Vec2 point) const
{
	if (!MapRect().contains(point))
		return -1;
	const auto p = (point - MapRect().center()) / Cell() + m_camera;
	const auto at = hexgrid::Pick(p.x, p.y);
	if (at.first < 0 || at.first >= StrategistJourney::Columns || at.second < 0 ||
	    at.second >= StrategistJourney::Rows)
		return -1;
	const int tile = Campaign::At(at.first, at.second);
	return m_journey.world.tiles[tile].terrain == Terrain::Sea ? -1 : tile;
}
int StrategistJourneyScene::EnemyAt(int tile) const
{
	for (const auto& contact : m_journey.intelligence[0].contacts)
		if (contact.current && contact.observed.tile == tile)
			return contact.id;
	return -1;
}
Rect StrategistJourneyScene::Control(int id) const
{
	const int x = Scene::Width() - 298;
	if (id == 0)
		return Rect(Scene::Width() - 144, 22, 126, 34);
	if (id == 1)
		return Rect(x, 228, 278, 34);
	if (id == 2)
		return Rect(x, 268, 134, 32);
	if (id == 3)
		return Rect(x + 142, 268, 136, 32);
	if (id == 4)
		return Rect(x, Scene::Height() - 140, 134, 34);
	if (id == 5)
		return Rect(x + 142, Scene::Height() - 140, 136, 34);
	if (id == 6)
		return Rect(x, Scene::Height() - 98, 278, 36);
	if (id == 7)
		return Rect(x, Scene::Height() - 54, 278, 34);
	return Rect(-1000, -1000, 1, 1);
}
void StrategistJourneyScene::Sample()
{
	const auto& world = m_journey.world;
	const int target = m_enemy >= 0 ? m_enemy : 3;
	const auto* contact = m_journey.intelligence[0].Find(target);
	if (!contact || !contact->current)
	{
		m_journey.message = U"見本に使う、確認中の敵を選ぼう。";
		return;
	}
	int retreat = -1;
	for (int tile : world.MapNeighbors(world.armies[0].tile))
		if (StrategistJourney::OnMap(tile) && TacticChain::Free(world, tile) &&
		    world.MapDistance(tile, world.armies[target].tile) > 1)
		{
			retreat = tile;
			break;
		}
	m_orders = {{0, target, ChainTactic::Lure, retreat},
	            {1, target, ChainTactic::Ambush, -1},
	            {2, target, ChainTactic::Breakthrough, -1}};
	m_pick = 0;
	m_journey.message = U"見本を予約した。線と成立の見込みを確認し、必要なら消して組み直そう。";
}
void StrategistJourneyScene::StartReplay()
{
	const auto& events = m_journey.lastChain.events;
	if (m_replay < 0 || m_replay >= static_cast<int>(events.size()))
	{
		m_replay = -1;
		m_fx.Skip();
		return;
	}
	const auto& event = events[m_replay];
	const String title = event.success
	                         ? U"{}連鎖  {}"_fmt(m_replay + 1, String(TacticChain::Name(event.tactic)))
	                         : U"連鎖中断";
	m_fx.Moment(event.army, title, Text(event.reason));
	m_replayTime = 0;
}
void StrategistJourneyScene::Update()
{
	m_music.Input(Vec2(-1000, -1000), 26);
	m_music.Update(m_journey.stage == JourneyStage::Battle                     ? musiccues::Cue::Battle
	               : m_journey.stage == JourneyStage::Debrief && m_journey.won ? musiccues::Cue::Victory
	                                                                           : musiccues::Cue::Journey,
	               Scene::DeltaTime(), m_replay >= 0);
	m_fx.Tick(m_journey.stage == JourneyStage::Battle, Scene::DeltaTime());
	if (KeyM.down())
		m_fx.ToggleSound();
	if (Control(0).leftClicked() || (KeyEscape.down() && m_pick == 0))
	{
		m_back = true;
		m_music.Stop();
		return;
	}
	if (KeyEscape.down() && m_pick != 0)
	{
		m_pick = 0;
		return;
	}
	if (m_replay >= 0)
	{
		m_replayTime += Scene::DeltaTime();
		if (m_replayTime >= 2.1 || KeySpace.down() || KeyEnter.down())
		{
			++m_replay;
			StartReplay();
		}
		return;
	}
	if (m_journey.stage == JourneyStage::Encounter)
	{
		if (KeySpace.down() || KeyEnter.down() ||
		    Rect(Scene::Center().x - 160, Scene::Height() - 72, 320, 40).leftClicked())
		{
			if (++m_beat >= 3)
				m_journey.Meet();
		}
		return;
	}
	if (m_journey.stage == JourneyStage::Preparation)
	{
		const int x = Scene::Center().x - 440, y = Scene::Center().y - 250;
		for (int i = 0; i < 2; ++i)
			if (Rect(x + 20 + i * 430, y + 145, 410, 145).leftClicked())
				m_choice = i;
		for (int i = 0; i < 3; ++i)
			if (Rect(x + 20 + i * 286, y + 325, 276, 42).leftClicked())
				m_policy = i;
		if (Rect(x + 20, y + 403, 270, 40).leftClicked())
			m_journey.Reinforce();
		if (Rect(x + 560, y + 403, 300, 40).leftClicked())
		{
			m_checkpoint = std::make_unique<StrategistJourney>(m_journey);
			if (m_journey.Prepare(m_choice, m_policy))
			{
				m_orders.clear();
				m_selected = 0;
				m_enemy = 3;
				m_pick = 0;
			}
		}
		return;
	}
	if (m_journey.stage == JourneyStage::Debrief)
	{
		if (Rect(Scene::Center().x - 310, Scene::Center().y + 178, 300, 40).leftClicked() && m_checkpoint)
		{
			m_journey = *m_checkpoint;
			m_orders.clear();
			m_pick = 0;
		}
		if (m_journey.won && Rect(Scene::Center().x + 10, Scene::Center().y + 178, 300, 40).leftClicked())
		{
			m_journey.Continue();
			m_orders.clear();
			m_pick = 0;
			m_policy = m_journey.enemyPolicy;
		}
		return;
	}
	if (m_journey.stage == JourneyStage::Complete)
	{
		if (Rect(Scene::Center().x - 160, Scene::Center().y + 158, 320, 40).leftClicked())
		{
			m_journey.Reset();
			m_beat = 0;
			m_orders.clear();
			m_pick = 0;
			m_choice = m_policy = 0;
		}
		return;
	}
	if (MapRect().contains(Cursor::PosF()))
	{
		const double wheel = Mouse::Wheel();
		if (wheel != 0)
		{
			const auto before = (Cursor::PosF() - MapRect().center()) / Cell() + m_camera;
			m_zoom = Clamp(m_zoom * std::pow(1.15, -wheel), .8, 2.6);
			m_camera = before - (Cursor::PosF() - MapRect().center()) / Cell();
		}
		if (MouseM.pressed())
			m_camera -= Cursor::DeltaF() / Cell();
	}
	if (KeyHome.down())
	{
		m_zoom = 1;
		m_camera = Vec2(8.5, 7.5 * hexgrid::Row);
	}
	m_camera.x = Clamp(m_camera.x, 1., 15.);
	m_camera.y = Clamp(m_camera.y, hexgrid::Row, 13 * hexgrid::Row);
	for (int i = 0; i < 3; ++i)
		if (Rect(18, 152 + i * 96, 206, 88).leftClicked())
		{
			m_selected = i;
			m_pick = 0;
		}
	const int tile = Pick(Cursor::PosF());
	if (Control(4).leftClicked())
	{
		Sample();
		return;
	}
	if (Control(5).leftClicked())
	{
		m_orders.clear();
		m_pick = 0;
		m_journey.message = U"予約を消した。未使用の隊で組み直せる。";
		return;
	}
	if (Control(2).leftClicked())
	{
		m_journey.Ambush(m_selected);
		m_orders.clear();
		return;
	}
	if (Control(3).leftClicked())
	{
		m_pick = 3;
		m_journey.message = U"通常攻撃の対象を、地図の確認中の敵から選ぶ。";
		return;
	}
	if (Control(1).leftClicked())
	{
		m_pick = 1;
		m_journey.message = U"連鎖に使う戦法の対象を、地図の確認中の敵から選ぶ。";
		return;
	}
	if (Control(6).leftClicked() || KeyC.down())
	{
		if (!m_orders.empty())
		{
			m_journey.Execute(m_orders);
			m_orders.clear();
			m_pick = 0;
			m_replay = 0;
			StartReplay();
		}
		return;
	}
	if (Control(7).leftClicked() || KeySpace.down())
	{
		m_orders.clear();
		m_pick = 0;
		m_journey.EndTurn();
		return;
	}
	if (MouseR.down())
	{
		if (m_pick != 0)
		{
			m_pick = 0;
			m_journey.message = U"対象の指定を取り消した。";
		}
		else if (tile >= 0)
		{
			m_journey.Move(m_selected, tile);
			m_orders.clear();
		}
		return;
	}
	if (MouseL.down() && tile >= 0)
	{
		if (m_pick == 2)
		{
			if (m_orders.size() < 3)
				m_orders.push_back({m_selected, m_enemy, ChainTactic::Lure, tile});
			m_pick = 0;
			return;
		}
		const int enemy = EnemyAt(tile);
		if (m_pick == 3)
		{
			m_journey.Strike(m_selected, enemy);
			m_pick = 0;
			m_orders.clear();
			return;
		}
		if (m_pick == 1 && enemy >= 0)
		{
			if (m_orders.size() >= 3)
			{
				m_journey.message = U"最大３手。予約を消して組み直そう。";
				m_pick = 0;
				return;
			}
			for (const auto& order : m_orders)
				if (order.army == m_selected)
				{
					m_journey.message = U"同じ隊は１回の連鎖に１手だけ。";
					m_pick = 0;
					return;
				}
			m_enemy = enemy;
			if (m_selected == 0)
			{
				m_pick = 2;
				m_journey.message = U"誘導役の後退先をクリック。敵から離れる隣接マスを指定する。";
			}
			else
			{
				m_orders.push_back({m_selected, enemy,
				                    m_selected == 1 ? ChainTactic::Ambush : ChainTactic::Breakthrough, -1});
				m_pick = 0;
			}
			return;
		}
		if (m_pick == 0)
		{
			if (enemy >= 0)
				m_enemy = enemy;
			for (int i = 0; i < 3; ++i)
				if (m_journey.world.armies[i].troops > 0 && m_journey.world.armies[i].tile == tile)
					m_selected = i;
		}
	}
}
void StrategistJourneyScene::Preview(int mode)
{
	m_journey.Reset();
	m_initialized = false;
	Open();
	if (m_music.Enabled())
		m_music.Toggle();
	if (m_fx.Sound())
		m_fx.ToggleSound();
	if (mode == 77)
		return;
	m_journey.Meet();
	if (mode == 78)
		return;
	m_checkpoint = std::make_unique<StrategistJourney>(m_journey);
	m_journey.Prepare(0, mode == 82 ? 1 : 0);
	m_enemy = 3;
	Sample();
	if (mode == 80)
	{
		m_journey.Execute(m_orders);
		m_orders.clear();
		m_replay = 2;
		StartReplay();
		m_fx.Tick(false, .5);
		m_replayTime = .5;
	}
	if (mode == 81)
	{
		m_journey.Execute(m_orders);
		for (int i = 0; i < 8 && m_journey.stage == JourneyStage::Battle; ++i)
		{
			m_journey.EndTurn();
			if (m_journey.stage == JourneyStage::Battle)
				m_journey.Strike(2, 3);
		}
		m_orders.clear();
	}
}
