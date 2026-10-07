#include "CampaignScene.hpp"
#include "CampaignSave.hpp"

using namespace frontline;
namespace
{
	ColorF FactionColor(int faction)
	{
		return faction == 0 ? ColorF(0.25,0.72,0.53) : faction == 1 ? ColorF(0.31,0.53,0.91) : faction == 2 ? ColorF(0.88,0.37,0.30) : ColorF(0.40,0.43,0.42);
	}
	void DrawButton(const Rect& rect, const String& label, bool enabled = true)
	{
		rect.rounded(5).draw(enabled && rect.mouseOver() ? ColorF(0.30,0.39,0.42) : ColorF(0.16,0.21,0.23));
		rect.rounded(5).drawFrame(1,enabled ? ColorF(0.49,0.60,0.58) : ColorF(0.25));
		FontAsset(U"campaignBody")(label).drawAt(rect.center(), enabled ? ColorF(0.94,0.93,0.86) : ColorF(0.45));
	}
}

CampaignScene::CampaignScene() { m_game.Reset(0); }
RectF CampaignScene::mapRect() const
{
	const double size = Max(8.0,Min((Scene::Width() - 390.0) / Width,(Scene::Height() - 190.0) / Height));
	return RectF(24,92,Width * size,Height * size);
}
Vec2 CampaignScene::tileCenter(int tile) const
{
	const auto rect = mapRect(); const double size = rect.w / Width;
	return rect.pos + Vec2((tile % Width + 0.5) * size,(tile / Width + 0.5) * size);
}
int CampaignScene::mouseTile() const
{
	const auto rect = mapRect(); if (!rect.mouseOver()) return -1;
	const Vec2 position = Cursor::PosF() - rect.pos;
	return Campaign::At(static_cast<int>(position.x / (rect.w / Width)),static_cast<int>(position.y / (rect.w / Width)));
}
Rect CampaignScene::button(int index) const
{
	const int x = Scene::Width() - 330;
	const int right = Scene::Width() - 460;
	if (index >= 31 && index < 39) return Rect(80,200 + (index - 31) * 42,218,36);
	switch (index)
	{
	case 0: return Rect(x,22,306,48);
	case 1: return Rect(x + 12,282,282,34);
	case 2: return Rect(x + 12,322,282,34);
	case 3: return Rect(x + 12,370,134,34);
	case 4: return Rect(x + 158,370,136,34);
	case 5: return Rect(x + 12,412,134,34);
	case 6: return Rect(x + 158,412,136,34);
	case 7: return Rect(x + 12,458,282,34);
	case 8: return Rect(x + 12,260,282,34);
	case 9: return Rect(x + 12,302,282,34);
	case 10: return Rect(x + 12,Scene::Height() - 182,134,34);
	case 11: return Rect(x + 158,Scene::Height() - 182,136,34);
	case 13: return Rect(x + 12,242,134,30);
	case 14: return Rect(x + 158,242,136,30);
	case 15: return Rect(225,65,170,24);
	case 16: return Rect(405,65,170,24);
	case 17: return Rect(585,65,170,24);
	case 18: return Rect(765,65,160,24);
	case 19: return Rect(Scene::Width() - 170,114,90,30);
	case 20: return Rect(80,156,218,34);
	case 21: return Rect(right,216,370,34);
	case 22: return Rect(right,258,370,34);
	case 23: return Rect(right,308,370,34);
	case 24: return Rect(right,402,370,34);
	case 25: return Rect(right,520,176,34);
	case 26: return Rect(right + 192,520,178,34);
	case 27: return Rect(right,562,176,34);
	case 28: return Rect(right + 192,562,178,34);
	case 29: return Rect(80,Scene::Height() - 120,134,30);
	case 30: return Rect(228,Scene::Height() - 120,134,30);
	case 39: return Rect(x + 12,344,282,34);
	default: return Rect(x + 12,Scene::Height() - 138,282,34);
	}
}
void CampaignScene::deploy(Arm arm)
{
	const auto available = m_game.Available(m_city);
	if (available.empty()) { m_message = U"出陣できる武将がいません。帰還・負傷回復を待ってください。"; return; }
	m_generalChoice %= static_cast<int>(available.size());
	const int army = m_game.Deploy(m_city,available[m_generalChoice],m_soldiers,arm);
	if (army < 0) { m_message = U"守備兵1000を残す兵力・金・糧・命令が必要です。出陣上限は6隊。"; return; }
	m_army = army; m_city = -1; m_message = U"右クリックで進路を指定。赤い都市へ進むと包囲・攻城を開始します。";
}
void CampaignScene::preview(int mode)
{
	m_started = true; m_game.Reset(0); m_city = 2;
	const int a = m_game.Deploy(2,4,3000,Arm::Spear); m_game.Order(a,m_game.cities[3].tile);
	const int b = m_game.Deploy(2,5,3000,Arm::Siege); m_game.Order(b,m_game.cities[3].tile);
	m_game.BeginTurn(); for (int i = 0; i < 6; ++i) m_game.AdvanceDay(); m_army = a; m_city = -1;
	m_supplyView = true; m_city = 0; m_army = -1; m_tab = 1; m_generalChoice = 1; m_game.Develop(0,0,Duty::Commerce);
	for (int i = 0; i < 4; ++i) m_game.AdvanceDay();
	m_councilMode = 1; m_inspect = 19; m_partner = 1; m_rosterFaction = 0;
	if (mode == 2)
	{
		m_game.randomState = 1; m_game.SendMission(0,19,3,MissionKind::Diplomacy,1);
		for (int i = 0; i < 20; ++i) m_game.AdvanceDay(); m_councilMode = 2;
	}
	else if (mode == 3) { m_army = a; m_city = -1; m_councilMode = 0; m_game.ActivateTactic(a); m_game.AdvanceDay(); }
	for (const auto& army : m_game.armies) m_positions.push_back(tileCenter(army.tile));
}
void CampaignScene::update()
{
	if (!m_started)
	{
		const int start = Scene::Center().x - 465;
		for (int f = 0; f < 3; ++f)
			if (Rect(start + f * 320,Scene::Center().y - 20,290,155).leftClicked())
			{ m_game.Reset(f); m_started = true; m_city = f * 3; m_army = -1; m_generalChoice = 0; m_positions.clear(); m_hits.clear(); return; }
		if (Rect(Scene::Center().x - 310,Scene::Center().y + 165,300,38).leftClicked())
		{
			Campaign loaded;
			if (LoadJSON(JSON::Load(U"frontline-save.json"),loaded))
			{ m_game = std::move(loaded); m_started = true; m_city = -1; m_army = -1; m_positions.clear(); m_hits.clear(); m_mapTexture = RenderTexture{}; m_message = U"戦況を復元しました。"; }
			else m_message = U"新作のセーブが見つかりません。勢力を選んで始めてください。";
		}
		if (Rect(Scene::Center().x + 10,Scene::Center().y + 165,300,38).leftClicked()) m_legacy = true;
		return;
	}
	if (KeyTab.down()) { m_supplyView = !m_supplyView; m_mapDay = -1; }
	if (m_councilMode != 0) { updateCouncil(); return; }
	if (button(17).leftClicked() || button(18).leftClicked())
	{
		m_councilMode = button(17).leftClicked() ? 1 : 2; m_paused = m_daysLeft > 0;
		m_rosterFaction = m_game.player; m_inspect = m_game.player * 6; m_partner = -1; m_historyPage = 0;
		if (m_army >= 0 && m_game.armies[m_army].faction == m_game.player) m_inspect = m_game.armies[m_army].general;
		else if (m_city >= 0 && m_game.cities[m_city].owner == m_game.player)
			for (int g = 0; g < static_cast<int>(m_game.generals.size()); ++g) if (m_game.generals[g].home == m_city && m_game.generals[g].faction == m_game.player) { m_inspect = g; break; }
		return;
	}
	if (button(15).leftClicked()) m_speed = (m_speed + 1) % 3;
	if (m_daysLeft > 0 && (KeySpace.down() || button(0).leftClicked())) m_paused = !m_paused;
	if (m_city >= 0 && button(13).leftClicked()) m_tab = 0;
	if (m_city >= 0 && button(14).leftClicked()) m_tab = 1;
	if (m_daysLeft > 0 && !m_paused)
	{
		m_timer += Scene::DeltaTime();
		const double interval = m_speed == 0 ? 1.0 : m_speed == 1 ? 0.5 : 0.2;
		if (m_timer >= interval)
		{
			m_timer -= interval;
			std::vector<int> troops; for (const auto& army : m_game.armies) troops.push_back(army.troops);
			m_game.AdvanceDay(); --m_daysLeft;
			for (int i = 0; i < static_cast<int>(m_game.armies.size()); ++i)
			{
				const auto& army = m_game.armies[i];
				if (troops[i] > army.troops && (army.troops > 0 || m_game.generals[army.general].readyDay == m_game.day + 20))
					m_hits.push_back({tileCenter(army.tile),troops[i] - army.troops,0.75});
			}
			if (m_game.result != 0) m_daysLeft = 0;
			if (m_daysLeft == 0) m_message = U"命令の時間です。新たな進路・出陣・補給を検討してください。";
		}
	}
	else
	{
		if (m_daysLeft == 0 && button(12).leftClicked()) { m_started = false; m_army = -1; m_city = -1; return; }
		if (m_daysLeft == 0 && button(10).leftClicked())
		{
			try { m_message = SaveJSON(m_game).save(U"frontline-save.json") ? U"新作の戦況を保存しました。" : U"保存できませんでした。"; }
			catch (...) { m_message = U"保存できませんでした。"; }
		}
		if (m_daysLeft == 0 && button(11).leftClicked())
		{
			Campaign loaded;
			if (LoadJSON(JSON::Load(U"frontline-save.json"),loaded))
			{ m_game = std::move(loaded); m_city = -1; m_army = -1; m_positions.clear(); m_hits.clear(); m_mapDay = -1; m_mapTexture = RenderTexture{}; m_message = U"戦況を復元しました。"; }
			else m_message = U"有効な新作のセーブが見つかりません。";
		}
		if (m_game.result == 0)
		{
			const int tile = mouseTile();
			if (MouseL.down() && Campaign::Valid(tile))
			{
				const int city = m_game.CityAt(tile);
				std::vector<int> units;
				for (int i = 0; i < static_cast<int>(m_game.armies.size()); ++i) if (m_game.armies[i].troops > 0 && m_game.armies[i].tile == tile) units.push_back(i);
				if (city >= 0 && (KeyShift.pressed() || units.empty())) { m_city = city; m_army = -1; m_generalChoice = 0; }
				else if (!units.empty())
				{
					auto it = std::find(units.begin(),units.end(),m_army);
					m_army = it != units.end() && ++it != units.end() ? *it : units.front(); m_city = -1;
				}
				else { m_city = -1; m_army = -1; }
			}
			if (m_army >= 0 && m_army < static_cast<int>(m_game.armies.size()) && m_game.armies[m_army].troops > 0 && m_game.armies[m_army].faction == m_game.player)
			{
				if (MouseR.down() && Campaign::Valid(tile)) m_message = m_game.Order(m_army,tile) ? U"進路を更新。進行開始で全勢力が同時に動きます。" : U"その場所へは到達できません。";
				if (button(8).leftClicked())
				{
					const int home = m_game.NearestCity(m_game.armies[m_army].tile,m_game.player,false);
					m_message = home >= 0 && m_game.Order(m_army,m_game.cities[home].tile,true) ? U"撤退命令。味方都市へ帰還します。" : U"帰還できる都市がありません。";
				}
				if (button(9).leftClicked()) { m_game.Order(m_army,m_game.armies[m_army].tile); m_message = U"現在地を守備します。近くの敵には自動で応戦。"; }
				if (button(39).leftClicked()) m_message = m_game.ActivateTactic(m_army) ? U"戦法を予約。次の1日に発動します。" : U"士気30以上・再使用待ちの終了・対象や適正兵科が必要です。";
			}
			else if (m_city >= 0 && m_game.cities[m_city].owner == m_game.player)
			{
				if (button(1).leftClicked()) ++m_generalChoice;
				if (m_tab == 1)
				{
					const auto available = m_game.Available(m_city);
					for (int d = 0; d < 4; ++d) if (button(3 + d).leftClicked())
					{
						m_message = !available.empty() && m_game.Develop(m_city,available[m_generalChoice % available.size()],static_cast<Duty>(d))
							? U"内政開始。金500・命令1を使用。30日間は担当武将が出陣できません。"
							: U"空いた武将・金500・命令1が必要。都市の事業は同時に1つ、能力上限100。";
					}
					if (button(7).leftClicked()) m_message = m_game.CancelWork(m_city) ? U"事業を中止。支払った金は戻りません。" : U"進行中の事業がありません。";
				}
				else
				{
				if (button(2).leftClicked()) m_soldiers = m_soldiers == 6000 ? 1500 : m_soldiers + 1500;
				if (button(3).leftClicked()) deploy(Arm::Spear);
				else if (button(4).leftClicked()) deploy(Arm::Bow);
				else if (button(5).leftClicked()) deploy(Arm::Siege);
				else if (button(6).leftClicked()) deploy(Arm::Cavalry);
				else if (button(7).leftClicked()) m_message = m_game.Recruit(m_city) ? U"兵2000を募集。金300・糧1000・治安10を使用。" : U"金300・糧1000・命令1・治安35以上が必要。兵力上限18000。";
				}
			}
			if (m_daysLeft == 0 && (button(0).leftClicked() || KeyEnter.down() || button(16).leftClicked())) { m_game.BeginTurn(); m_daysLeft = button(16).leftClicked() ? 1 : 10 - m_game.day % 10; m_timer = 0; m_paused = false; m_message = U"進行中。Spaceで一時停止し、作戦を見直せます。"; }
		}
	}
	while (m_positions.size() < m_game.armies.size()) m_positions.push_back(tileCenter(m_game.armies[m_positions.size()].tile));
	for (size_t i = 0; i < m_positions.size(); ++i) m_positions[i] = m_positions[i].lerp(tileCenter(m_game.armies[i].tile),Min(1.0,Scene::DeltaTime() * 18));
	for (auto& hit : m_hits) hit.time -= Scene::DeltaTime();
	m_hits.remove_if([](const Hit& hit) { return hit.time <= 0; });
}

void CampaignScene::drawMap() const
{
	const RectF area = mapRect(); const double cell = area.w / Width;
	if (!m_mapTexture || m_mapRevision != m_game.revision || m_mapSupply != m_supplyView || (m_supplyView && m_mapDay != m_game.day))
	{
		m_mapTexture = RenderTexture(Width * 32,Height * 32,ColorF(0.08,0.12,0.13));
		const auto supply = m_game.Supply(m_game.player);
		{
			const ScopedRenderTarget2D target(m_mapTexture);
			for (int i = 0; i < TileCount; ++i)
			{
				const auto& tile = m_game.tiles[i]; const RectF rect((i % Width) * 32,(i / Width) * 32,32,32);
				ColorF color(0.32,0.37,0.29);
				if (tile.terrain == Terrain::Forest) color = ColorF(0.17,0.29,0.23);
				else if (tile.terrain == Terrain::Mountain) color = ColorF(0.36,0.34,0.31);
				else if (tile.terrain == Terrain::River || tile.terrain == Terrain::Sea) color = ColorF(0.12,0.24,0.32);
				rect.draw(color);
				if (tile.owner >= 0) rect.draw(ColorF(FactionColor(tile.owner),0.28));
				if (m_supplyView && supply[i] >= 0) rect.stretched(-3).draw(ColorF(0.4,0.95,0.72,0.22));
				if (tile.terrain == Terrain::Mountain) Triangle(rect.center().movedBy(0,-8),rect.center().movedBy(-9,7),rect.center().movedBy(9,7)).draw(ColorF(0.6,0.57,0.51,0.4));
				else if (tile.terrain == Terrain::Forest) { Circle(rect.center().movedBy(-4,0),6).draw(ColorF(0.1,0.2,0.15)); Circle(rect.center().movedBy(5,-3),5).draw(ColorF(0.12,0.24,0.16)); }
				else if (tile.terrain == Terrain::Bridge) RectF(rect.x + 4,rect.y + 13,24,6).draw(ColorF(0.72,0.63,0.43));
				rect.drawFrame(0.5,ColorF(0.05,0.10,0.12,0.35));
				if (i % Width + 1 < Width && tile.owner != m_game.tiles[i+1].owner) Line(rect.tr(),rect.br()).draw(1.5,ColorF(0.82,0.80,0.63,0.6));
				if (i / Width + 1 < Height && tile.owner != m_game.tiles[i+Width].owner) Line(rect.bl(),rect.br()).draw(1.5,ColorF(0.82,0.80,0.63,0.6));
			}
		}
		m_mapRevision = m_game.revision; m_mapDay = m_game.day; m_mapSupply = m_supplyView;
	}
	m_mapTexture.scaled(cell / 32.0).draw(area.pos); area.drawFrame(2,ColorF(0.48,0.55,0.48));
	if (m_army >= 0 && m_army < static_cast<int>(m_game.armies.size()) && m_game.armies[m_army].troops > 0)
	{
		const auto& army = m_game.armies[m_army]; Vec2 last = tileCenter(army.tile);
		for (int p : army.path) { const Vec2 next = tileCenter(p); Line(last,next).draw(2,ColorF(0.93,0.79,0.4,0.9)); last = next; }
		RectF(Arg::center(tileCenter(army.target)),cell,cell).drawFrame(2,ColorF(0.97,0.79,0.36));
	}
	for (int i = 0; i < static_cast<int>(m_game.cities.size()); ++i)
	{
		const auto& city = m_game.cities[i]; const Vec2 center = tileCenter(city.tile);
		RectF(Arg::center(center),cell * 0.75,cell * 0.75).draw(ColorF(0.07,0.09,0.10)).drawFrame(2,FactionColor(city.owner));
		RectF(Arg::center(center.movedBy(0,-3)),cell * 0.45,cell * 0.35).draw(FactionColor(city.owner));
		RectF(center.x - cell * 0.4,center.y + cell * 0.5,cell * 0.8,3).draw(ColorF(0.04,0.07,0.08));
		RectF(center.x - cell * 0.4,center.y + cell * 0.5,cell * 0.8 * Min(1.0,city.troops / 10000.0),3).draw(FactionColor(city.owner));
		if (i == m_city) Circle(center,cell * 0.7).drawFrame(2,ColorF(0.98,0.82,0.4));
		FontAsset(U"campaignSmall")(text(city.name)).drawAt(center.movedBy(0,-cell * 0.85),ColorF(0.05,0.06,0.07));
		FontAsset(U"campaignSmall")(text(city.name)).drawAt(center.movedBy(0,-cell * 0.85 - 1),ColorF(0.96,0.94,0.83));
	}
	for (int i = 0; i < static_cast<int>(m_game.armies.size()); ++i)
	{
		const auto& army = m_game.armies[i]; if (army.troops <= 0) continue;
		Vec2 center = i < static_cast<int>(m_positions.size()) ? m_positions[i] : tileCenter(army.tile);
		int count = 0, order = 0;
		for (int j = 0; j < static_cast<int>(m_game.armies.size()); ++j)
			if (m_game.armies[j].troops > 0 && m_game.armies[j].tile == army.tile) { if (j < i) ++order; ++count; }
		center.x += (order - (count - 1) * 0.5) * cell * 0.48;
		center.y += cell * 0.2;
		Circle(center,cell * 0.33).draw(FactionColor(army.faction)).drawFrame(1.5,ColorF(0.96,0.93,0.8));
		if (i == m_army) Circle(center,cell * 0.47).drawFrame(2,ColorF(0.98,0.82,0.4));
		const String icon = army.arm == Arm::Siege ? U"城" : army.arm == Arm::Bow ? U"弓" : army.arm == Arm::Cavalry ? U"騎" : U"槍";
		FontAsset(U"campaignSmall")(icon).drawAt(center,ColorF(0.04,0.08,0.08));
		bool nearby = false;
		for (int j = 0; j < static_cast<int>(m_game.armies.size()); ++j)
			if (j != i && m_game.armies[j].troops > 0 && Campaign::Distance(army.tile,m_game.armies[j].tile) <= 1) nearby = true;
		if (i == m_army || (count == 1 && !nearby)) FontAsset(U"campaignSmall")(text(m_game.generals[army.general].name)).drawAt(center.movedBy(0,cell * 0.7),ColorF(0.99,0.96,0.86));
		if (!army.supplied) Circle(center.movedBy(cell * 0.35,-cell * 0.35),3).draw(ColorF(1.0,0.55,0.25));
		if (army.tacticLeft > 0) Line(center.movedBy(-cell * 0.3,cell * 0.4),center.movedBy(cell * 0.3,cell * 0.4)).draw(3,ColorF(0.98,0.81,0.36));
	}
	for (const auto& hit : m_hits)
		FontAsset(U"campaignSmall")(U"-{}"_fmt(hit.amount)).drawAt(hit.position.movedBy(0,-12 - (0.75-hit.time) * 30),ColorF(1.0,0.7,0.4,Min(1.0,hit.time * 2)));
	for (int i = 0; i < static_cast<int>(m_game.armies.size()); ++i)
		for (int j = i + 1; j < static_cast<int>(m_game.armies.size()); ++j)
		{
			const auto& a = m_game.armies[i]; const auto& b = m_game.armies[j];
			const int rangeA = a.arm == Arm::Bow ? (a.tacticLeft > 0 && m_game.generals[a.general].tactic == Tactic::Volley ? 3 : 2) : 1;
			const int rangeB = b.arm == Arm::Bow ? (b.tacticLeft > 0 && m_game.generals[b.general].tactic == Tactic::Volley ? 3 : 2) : 1;
			if (a.troops <= 0 || b.troops <= 0 || !m_game.Hostile(a.faction,b.faction) || Campaign::Distance(a.tile,b.tile) > Max(rangeA,rangeB)) continue;
			const Vec2 center = (tileCenter(a.tile) + tileCenter(b.tile)) / 2;
			Line(center.movedBy(-4,-5),center.movedBy(4,5)).draw(2,ColorF(1.0,0.72,0.35));
			Line(center.movedBy(4,-5),center.movedBy(-4,5)).draw(2,ColorF(1.0,0.72,0.35));
		}
	const int hover = mouseTile();
	if (Campaign::Valid(hover)) RectF(Arg::center(tileCenter(hover)),cell,cell).drawFrame(1,ColorF(0.96,0.95,0.8,0.8));
}

void CampaignScene::drawPanel() const
{
	const int x = Scene::Width() - 330;
	Rect(x,86,306,Scene::Height() - 106).rounded(7).draw(ColorF(0.10,0.14,0.16)).drawFrame(1,ColorF(0.30,0.40,0.40));
	const auto line = [&](const String& label, int y, ColorF color = ColorF(0.87,0.90,0.86)) { FontAsset(U"campaignBody")(label).draw(x + 14,y,color); };
	const bool orders = (m_daysLeft == 0 || m_paused) && m_game.result == 0;
	if (m_army >= 0 && m_army < static_cast<int>(m_game.armies.size()) && m_game.armies[m_army].troops > 0)
	{
		const auto& army = m_game.armies[m_army];
		line(text(m_game.generals[army.general].name) + U"隊 / " + text(Campaign::ArmName(army.arm)),105,FactionColor(army.faction));
		line(U"兵力 {}　士気 {}"_fmt(army.troops,army.morale),143);
		line(U"携行兵糧 {}"_fmt(army.food),178);
		line(army.supplied ? U"補給：都市と接続" : U"補給：途絶（携行糧を消費）",211,army.supplied ? ColorF(0.5,0.88,0.65) : ColorF(1.0,0.62,0.36));
		const bool own = army.faction == m_game.player;
		DrawButton(button(8),U"味方都市へ撤退",own && orders); DrawButton(button(9),U"現在地を守備",own && orders);
		const auto& g = m_game.generals[army.general];
		DrawButton(button(39),army.tacticQueued ? U"戦法予約済み" : text(TacticName(g.tactic)) + U" / 待ち{}日"_fmt(Max(0,army.tacticReadyDay - m_game.day)),own && orders && !army.tacticQueued && army.tacticReadyDay <= m_game.day && army.morale >= 30);
		(void)FontAsset(U"campaignSmall")(text(TacticDescription(g.tactic)) + U"\n\n個性：" + text(TraitName(g.trait)) + U"\n" + text(TraitDescription(g.trait)) + U"\n近隣との親密度：{}"_fmt(m_game.SupportBond(m_army)) + U"\n戦法は士気15を使い、30日後に再使用。\n右クリックで進路を指定。").draw(RectF(x + 14,392,280,210),ColorF(0.74,0.81,0.76));
	}
	else if (m_city >= 0)
	{
		const auto& city = m_game.cities[m_city]; const bool own = city.owner == m_game.player;
		line(text(city.name) + U" / " + text(Campaign::FactionName(city.owner)),105,FactionColor(city.owner));
		line(U"守備兵 {}"_fmt(city.troops),145); line(U"金 {}"_fmt(city.gold),180); line(U"兵糧 {}"_fmt(city.food),215);
		DrawButton(button(13),m_tab == 0 ? U"● 軍務" : U"軍務"); DrawButton(button(14),m_tab == 1 ? U"● 内政・武将" : U"内政・武将");
		const auto available = m_game.Available(m_city);
		const String name = available.empty() ? U"出陣可能な武将なし" : text(m_game.generals[available[m_generalChoice % available.size()]].name);
		DrawButton(button(1),name + U"　切替",own && orders && !available.empty());
		if (m_tab == 1)
		{
			FontAsset(U"campaignSmall")(U"農政 {} / 商業 {} / 治安 {} / 兵站 {}"_fmt(city.farming,city.commerce,city.order,city.logistics)).draw(x + 14,324,ColorF(0.87,0.90,0.86));
			FontAsset(U"campaignSmall")(U"月収：金 {} / 糧 {}"_fmt((100 + city.commerce * 15) * city.order / 100,(800 + city.farming * 80) * city.order / 100)).draw(x + 14,346,ColorF(0.93,0.83,0.57));
			for (int d = 0; d < 4; ++d)
			{
				const auto duty = static_cast<Duty>(d);
				const int gain = available.empty() ? 0 : m_game.WorkGain(available[m_generalChoice % available.size()],duty);
				DrawButton(button(3 + d),text(Campaign::DutyName(duty)) + U" +{}"_fmt(gain),own && orders && city.worker < 0 && !available.empty());
			}
			DrawButton(button(7),city.worker >= 0 ? U"事業を中止（返金なし）" : U"金500 / 30日 / 命令1",own && orders && city.worker >= 0);
			String detail;
			if (!available.empty())
			{
				const int g = available[m_generalChoice % available.size()]; const auto& officer = m_game.generals[g];
				detail = U"統率 {} / 政治 {} / 得意：{}\n得意な仕事は開発量 +8。兵站型は\n出陣時にも兵糧の消費を抑えます。"_fmt(officer.leadership,officer.politics,text(Campaign::DutyName(officer.specialty)));
			}
			if (city.worker >= 0) detail += U"\n{}：{} / 残り{}日"_fmt(text(m_game.generals[city.worker].name),text(Campaign::DutyName(static_cast<Duty>(city.work))),city.workLeft);
			(void)FontAsset(U"campaignSmall")(detail).draw(RectF(x + 14,503,280,100),ColorF(0.76,0.85,0.80));
		}
		else
		{
		DrawButton(button(2),U"出陣兵数 {}　切替"_fmt(m_soldiers),own && orders);
		DrawButton(button(3),U"槍兵で出陣",own && orders); DrawButton(button(4),U"弓兵で出陣",own && orders);
		DrawButton(button(5),U"攻城隊で出陣",own && orders); DrawButton(button(6),U"騎兵で出陣",own && orders);
		DrawButton(button(7),U"募兵 +2000",own && orders);
		FontAsset(U"campaignSmall")(U"出陣：金=兵数/10・糧=兵数/2\n募兵は治安を10消費。自動増兵なし。\n序盤60日は敵も内政を優先します。").draw(x + 14,503,ColorF(0.68,0.77,0.74));
		}
	}
	else
	{
		line(U"戦況を見て、作戦を立てる",107,ColorF(0.93,0.83,0.57));
		(void)FontAsset(U"campaignBody")(U"都市を選択 → 出陣\n部隊選択 → 右クリックで進路\nEnter → 全勢力の10日間を進行\n\nShift+左：都市を選択\n同じマスはクリックで部隊切替\nTab：補給可能な土地を表示\n\n槍兵：野戦の基本\n弓兵：2マス先へ射撃\n騎兵：平地で速く強い\n攻城隊：城に強い、野戦に弱い").draw(RectF(x + 14,154,280,Max(330,Scene::Height() - 350)),ColorF(0.75,0.82,0.77));
	}
	DrawButton(button(10),U"保存",m_daysLeft == 0); DrawButton(button(11),U"読込",m_daysLeft == 0); DrawButton(button(12),U"勢力選択へ",m_daysLeft == 0);
	(void)FontAsset(U"campaignSmall")(m_message).draw(RectF(x + 14,Scene::Height() - 84,280,60),ColorF(0.94,0.83,0.55));
}
void CampaignScene::drawMenu() const
{
	Scene::Rect().draw(ColorF(0.02,0.06,0.08,0.88));
	FontAsset(U"campaignTitle")(U"群 雄 戦 線").drawAt(Scene::Center().x,Scene::Center().y - 160,ColorF(0.95,0.86,0.61));
	FontAsset(U"campaignBody")(U"領土を奪い、兵站を断ち、天下を争う。").drawAt(Scene::Center().x,Scene::Center().y - 105,ColorF(0.76,0.85,0.81));
	const Array<String> notes{U"西方から進む。山と隘路を味方に。", U"中央を押さえる。多方面への備えを。", U"江南を守る。渡河点をめぐる戦い。"};
	const int start = Scene::Center().x - 465;
	for (int f = 0; f < 3; ++f)
	{
		const Rect card(start + f * 320,Scene::Center().y - 20,290,155);
		card.rounded(8).draw(ColorF(FactionColor(f),card.mouseOver() ? 0.4 : 0.18)).drawFrame(2,FactionColor(f));
		FontAsset(U"campaignTitle")(text(Campaign::FactionName(f))).drawAt(card.center().movedBy(0,-30),FactionColor(f));
		FontAsset(U"campaignSmall")(notes[f]).drawAt(card.center().movedBy(0,28),ColorF(0.91,0.92,0.84));
		FontAsset(U"campaignBody")(U"この勢力で始める").drawAt(card.center().movedBy(0,58),ColorF(0.95,0.86,0.61));
	}
	DrawButton(Rect(Scene::Center().x - 310,Scene::Center().y + 165,300,38),U"続きから");
	DrawButton(Rect(Scene::Center().x + 10,Scene::Center().y + 165,300,38),U"以前の作品を開く");
	FontAsset(U"campaignSmall")(U"新作の戦略試作版 / 史実の領土・年代を再現するシナリオではありません").drawAt(Scene::Center().x,Scene::Center().y + 232,ColorF(0.64,0.73,0.71));
	if (m_message.starts_with(U"新作のセーブ")) FontAsset(U"campaignSmall")(m_message).drawAt(Scene::Center().x,Scene::Center().y + 260,ColorF(0.98,0.78,0.48));
}
void CampaignScene::draw() const
{
	Scene::SetBackground(ColorF(0.055,0.085,0.10));
	FontAsset(U"campaignTitle")(U"群雄戦線").draw(24,22,ColorF(0.95,0.87,0.63));
	const String status = m_game.result == 1 ? U"天下統一" : m_game.result == 2 ? U"勢力滅亡" : m_daysLeft > 0 ? (m_paused ? U"一時停止" : U"進行中") : U"命令期間";
	FontAsset(U"campaignBody")(U"{}軍　{}日経過　{}　命令 {} / 3"_fmt(text(Campaign::FactionName(m_game.player)),m_game.day,status,m_game.commands)).draw(225,37,ColorF(0.81,0.87,0.82));
	DrawButton(button(0),m_daysLeft > 0 ? (m_paused ? U"再開 / Space" : U"一時停止 / Space") : U"次の命令期へ / Enter",m_game.result == 0);
	DrawButton(button(15),m_speed == 0 ? U"速度：じっくり ×1" : m_speed == 1 ? U"速度：×2" : U"速度：×5");
	DrawButton(button(16),U"1日だけ進める",m_daysLeft == 0 && m_game.result == 0);
	DrawButton(button(17),U"武将・評定"); DrawButton(button(18),U"群雄の記録");
	drawMap(); drawPanel();
	FontAsset(U"campaignSmall")(m_supplyView ? U"補給表示ON / Tabで切替　橙の印＝補給途絶" : U"左クリック：選択　右クリック：進路　Shift+左：都市　Tab：補給表示").draw(24,mapRect().bottomY() + 12,ColorF(0.73,0.81,0.77));
	const double logY = Scene::Height() - 68;
	RectF(24,logY,Scene::Width() - 390,54).rounded(5).draw(ColorF(0.08,0.13,0.15));
	const int first = Max(0,static_cast<int>(m_game.log.size()) - 2);
	for (int i = first; i < static_cast<int>(m_game.log.size()); ++i) FontAsset(U"campaignSmall")(text(m_game.log[i])).draw(36,logY + 5 + (i-first) * 22,ColorF(0.84,0.87,0.78));
	if (m_game.result != 0 && m_started) FontAsset(U"campaignTitle")(m_game.result == 1 ? U"天下統一 — 群雄を制す" : U"敗北 — 再び旗を掲げよ").drawAt(mapRect().center(),ColorF(0.99,0.85,0.44));
	if (!m_started) drawMenu();
	else if (m_councilMode != 0) drawCouncil();
}

void CampaignScene::updateCouncil()
{
	if (button(19).leftClicked() || KeyEscape.down()) { m_councilMode = 0; return; }
	const int rows = Max(1,(Scene::Height() - 320) / 54);
	if (m_councilMode == 2)
	{
		if (button(29).leftClicked()) m_historyPage = Max(0,m_historyPage - 1);
		if (button(30).leftClicked()) m_historyPage = Min(Max(0,(static_cast<int>(m_game.chronicle.size()) - 1) / rows),m_historyPage + 1);
		return;
	}
	if (button(20).leftClicked()) { m_rosterFaction = (m_rosterFaction + 1) % 3; m_inspect = m_rosterFaction * 6; m_partner = -1; }
	int row = 0;
	for (int g = 0; g < static_cast<int>(m_game.generals.size()); ++g)
		if (m_game.generals[g].faction == m_rosterFaction) { if (button(31 + row).leftClicked()) { m_inspect = g; m_partner = -1; } ++row; }
	const auto& g = m_game.generals[m_inspect]; const int city = g.home;
	if (m_game.cities[m_targetCity].owner == g.faction)
		for (int c = 0; c < static_cast<int>(m_game.cities.size()); ++c) if (m_game.cities[c].owner != g.faction) { m_targetCity = c; break; }
	const auto staff = m_game.Available(city);
	const bool available = g.faction == m_game.player && std::find(staff.begin(),staff.end(),m_inspect) != staff.end() && m_game.result == 0;
	if (available && button(21).leftClicked())
	{
		std::vector<int> options{-1}; for (int member : staff) if (member != m_inspect) options.push_back(member);
		auto it = std::find(options.begin(),options.end(),m_partner);
		m_partner = it != options.end() && ++it != options.end() ? *it : options.front();
	}
	if (available && button(22).leftClicked())
		for (int offset = 1; offset <= static_cast<int>(m_game.cities.size()); ++offset)
		{ const int c = (m_targetCity + offset) % static_cast<int>(m_game.cities.size()); if (m_game.cities[c].owner != g.faction) { m_targetCity = c; break; } }
	if (!available) return;
	for (int kind = 0; kind < 2; ++kind) if (button(kind == 0 ? 23 : 24).leftClicked())
		m_message = m_game.SendMission(city,m_inspect,m_targetCity,static_cast<MissionKind>(kind),m_partner)
			? U"任務を開始。金300・命令1を使用。担当者は完了まで出陣できません。"
			: U"金300・命令1・空いた担当者が必要です。停戦中の勢力へ工作はできません。";
	for (int duty = 0; duty < 4; ++duty) if (button(25 + duty).leftClicked())
		m_message = m_game.Develop(city,m_inspect,static_cast<Duty>(duty),false,m_partner)
			? U"内政を開始。副担当との親密度は完成時に8上がります。"
			: U"金500・命令1・空いた担当者・都市の事業枠が必要です。";
}

void CampaignScene::drawCouncil() const
{
	Scene::Rect().draw(ColorF(0.02,0.04,0.06,0.82));
	Rect(60,102,Scene::Width() - 120,Scene::Height() - 176).rounded(8).draw(ColorF(0.09,0.14,0.17)).drawFrame(2,ColorF(0.49,0.60,0.55));
	DrawButton(button(19),U"閉じる");
	const auto label = [&](const String& value,int x,int y) { FontAsset(U"campaignBody")(value).draw(x,y,ColorF(0.87,0.90,0.84)); };
	if (m_councilMode == 2)
	{
		label(U"群雄の記録 — 今回のプレイで起きた出来事",80,115);
		const int rows = Max(1,(Scene::Height() - 320) / 54);
		for (int row = 0; row < rows; ++row)
		{
			const int index = static_cast<int>(m_game.chronicle.size()) - 1 - m_historyPage * rows - row; if (index < 0) break;
			const auto& event = m_game.chronicle[index];
			Rect(80,174 + row * 54,Scene::Width() - 160,50).rounded(4).draw(ColorF(0.13,0.19,0.21));
			(void)FontAsset(U"campaignBody")(U"{}日　{}"_fmt(event.day,text(event.text))).draw(RectF(90,180 + row * 54,Scene::Width() - 180,46),ColorF(0.88,0.88,0.78));
		}
		DrawButton(button(29),U"新しい記録",m_historyPage > 0); DrawButton(button(30),U"古い記録",(m_historyPage + 1) * rows < static_cast<int>(m_game.chronicle.size()));
		label(U"{} / {}頁"_fmt(m_historyPage + 1,Max(1,(static_cast<int>(m_game.chronicle.size()) + rows - 1) / rows)),390,Scene::Height() - 118);
		return;
	}
	label(U"武将・評定 — 個性と人のつながりを活かす",80,115);
	DrawButton(button(20),text(Campaign::FactionName(m_rosterFaction)) + U"軍の武将 / 切替");
	int row = 0;
	for (int member = 0; member < static_cast<int>(m_game.generals.size()); ++member)
		if (m_game.generals[member].faction == m_rosterFaction)
		{
			DrawButton(button(31 + row),text(m_game.generals[member].name) + (m_game.Busy(member) ? U" / 任務中" : m_game.generals[member].readyDay > m_game.day ? U" / 休養中" : U""));
			if (member == m_inspect) button(31 + row).drawFrame(2,ColorF(0.96,0.78,0.40)); ++row;
		}
	const int middle = 330, right = Scene::Width() - 460;
	Line(314,156,314,Scene::Height() - 144).draw(1,ColorF(0.35,0.45,0.42));
	Line(right - 16,156,right - 16,Scene::Height() - 144).draw(1,ColorF(0.35,0.45,0.42));
	const auto& g = m_game.generals[m_inspect]; const auto& home = m_game.cities[g.home];
	label(text(g.name) + U" / " + text(home.name),middle,163);
	label(U"統率 {}　政治 {}"_fmt(g.leadership,g.politics),middle,204);
	label(U"知力 {}　魅力 {}"_fmt(g.intelligence,g.charm),middle,235);
	label(U"個性：" + text(TraitName(g.trait)),middle,275);
	(void)FontAsset(U"campaignBody")(text(TraitDescription(g.trait))).draw(RectF(middle,309,right - middle - 32,68),ColorF(0.76,0.85,0.80));
	label(U"戦法：" + text(TacticName(g.tactic)),middle,387);
	(void)FontAsset(U"campaignBody")(text(TacticDescription(g.tactic))).draw(RectF(middle,420,right - middle - 32,76),ColorF(0.76,0.85,0.80));
	label(m_partner >= 0 ? U"{}との親密度：{}"_fmt(text(m_game.generals[m_partner].name),m_game.Affinity(m_inspect,m_partner)) : U"副担当を選ぶと共同任務ができます",middle,511);
	std::vector<int> friends;
	for (int member = 0; member < static_cast<int>(m_game.generals.size()); ++member) if (member != m_inspect && m_game.Affinity(m_inspect,member) > 20) friends.push_back(member);
	std::sort(friends.begin(),friends.end(),[&](int a,int b) { return m_game.Affinity(m_inspect,a) > m_game.Affinity(m_inspect,b); });
	String connections = U"深いつながり：";
	for (int i = 0; i < Min(3,static_cast<int>(friends.size())); ++i) connections += U"\n{} / 親密度 {}"_fmt(text(m_game.generals[friends[i]].name),m_game.Affinity(m_inspect,friends[i]));
	(void)FontAsset(U"campaignSmall")(connections).draw(RectF(middle,550,right - middle - 32,90),ColorF(0.87,0.80,0.58));
	const auto staff = m_game.Available(g.home);
	const bool free = g.faction == m_game.player && std::find(staff.begin(),staff.end(),m_inspect) != staff.end() && m_game.result == 0;
	label(free ? U"任命元：{} / 金 {}"_fmt(text(home.name),home.gold) : U"任命不可：任務・休養・他勢力",right,164);
	DrawButton(button(21),m_partner < 0 ? U"副担当：なし / 切替" : U"副担当：{} / 切替"_fmt(text(m_game.generals[m_partner].name)),free);
	DrawButton(button(22),U"対象：{}（{}） / 切替"_fmt(text(m_game.cities[m_targetCity].name),text(Campaign::FactionName(m_game.cities[m_targetCity].owner))),free);
	const int targetFaction = m_game.cities[m_targetCity].owner;
	DrawButton(button(23),U"停戦交渉 / 成功率 {}%"_fmt(m_game.MissionChance(m_inspect,m_targetCity,MissionKind::Diplomacy,m_partner)),free && home.gold >= 300 && m_game.commands > 0 && targetFaction != g.faction);
	FontAsset(U"campaignSmall")(U"金300・命令1・20日。成立すると60日停戦。\n信頼 {} / 君主親密 {} / 停戦 {}日"_fmt(m_game.regard[g.faction][targetFaction],m_game.Affinity(m_inspect,m_game.Leader(targetFaction)),Max(0,m_game.truceUntil[g.faction][targetFaction] - m_game.day))).draw(right,350,ColorF(0.76,0.85,0.80));
	DrawButton(button(24),U"兵糧攪乱 / 成功率 {}%"_fmt(m_game.MissionChance(m_inspect,m_targetCity,MissionKind::Sabotage,m_partner)),free && home.gold >= 300 && m_game.commands > 0 && m_game.Hostile(g.faction,targetFaction));
	FontAsset(U"campaignSmall")(U"金300・命令1・30日。糧と治安に損害。\n任務失敗：再任用まで10日。副担当も拘束。").draw(right,444,ColorF(0.76,0.85,0.80));
	label(U"内政：30日 / 金500 / 命令1",right,488);
	for (int d = 0; d < 4; ++d) DrawButton(button(25 + d),text(Campaign::DutyName(static_cast<Duty>(d))) + U" +{}"_fmt(m_game.WorkGain(m_inspect,static_cast<Duty>(d),m_partner)),free && home.worker < 0 && home.gold >= 500 && m_game.commands > 0);
	String status;
	for (const auto& c : m_game.cities) if (c.worker == m_inspect || (c.worker >= 0 && c.helper == m_inspect)) status = U"{}：{} / 残り{}日"_fmt(text(c.name),text(Campaign::DutyName(static_cast<Duty>(c.work))),c.workLeft);
	for (const auto& m : m_game.missions) if (m.general == m_inspect || m.helper == m_inspect) status = U"{}：{} / 残り{}日"_fmt(text(m_game.cities[m.target].name),m.kind == MissionKind::Diplomacy ? U"停戦交渉" : U"兵糧攪乱",m.left);
	if (status.isEmpty()) status = U"共同内政：完成時に親密度 +8。\n近隣の親しい部隊は攻撃力が上がります。";
	(void)FontAsset(U"campaignSmall")(status).draw(RectF(right,610,370,48),ColorF(0.94,0.83,0.55));
	(void)FontAsset(U"campaignSmall")(m_message).draw(RectF(80,Scene::Height() - 135,Scene::Width() - 160,44),ColorF(0.94,0.83,0.55));
}
