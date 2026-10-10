#include "CampaignScene.hpp"
#include "CampaignSave.hpp"
#include "CampaignUI.hpp"

using namespace frontline;
using namespace campaignui;

void CampaignScene::updateRosterPage(int faction)
{
	int count = 0;
	for (const auto& g : m_game.generals)
		if (g.faction == faction)
			++count;
	const int last = Max(0, (count - 1) / 8);
	m_rosterPage = Clamp(m_rosterPage, 0, last);
	if (Rect(80, 550, 104, 30).leftClicked())
		m_rosterPage = Max(0, m_rosterPage - 1);
	if (Rect(194, 550, 104, 30).leftClicked())
		m_rosterPage = Min(last, m_rosterPage + 1);
}
void CampaignScene::drawRosterPages(int faction) const
{
	int count = 0;
	for (const auto& g : m_game.generals)
		if (g.faction == faction)
			++count;
	const int pages = Max(1, (count + 7) / 8);
	DrawButton(Rect(80, 550, 104, 30), U"前の武将", m_rosterPage > 0);
	DrawButton(Rect(194, 550, 104, 30), U"次の武将", m_rosterPage + 1 < pages);
	FontAsset(U"campaignSmall")(U"{}人 / {} / {}頁"_fmt(count, m_rosterPage + 1, pages))
	    .draw(80, 588, ColorF(.8, .85, .75));
}
void CampaignScene::updateCouncil()
{
	if (m_councilMode == 4)
	{
		updateGovernance();
		return;
	}
	if (m_councilMode == 3)
	{
		updateAssignments();
		return;
	}
	if (Rect(330, 114, 200, 30).leftClicked() && m_councilMode == 1)
	{
		m_councilMode = 3;
		m_rosterFaction = m_game.player;
		m_rosterPage = 0;
		m_inspect = m_game.player * 6;
		return;
	}
	if (button(19).leftClicked() || KeyEscape.down())
	{
		m_councilMode = 0;
		return;
	}
	const int rows = Max(1, (Scene::Height() - 320) / 54);
	if (m_councilMode == 2)
	{
		if (button(29).leftClicked())
			m_historyPage = Max(0, m_historyPage - 1);
		if (button(30).leftClicked())
			m_historyPage =
			    Min(Max(0, (static_cast<int>(m_game.chronicle.size()) - 1) / rows), m_historyPage + 1);
		return;
	}
	if (button(20).leftClicked())
	{
		m_rosterFaction = (m_rosterFaction + 1) % 3;
		m_rosterPage = 0;
		m_inspect = m_rosterFaction * 6;
		m_partner = -1;
	}
	updateRosterPage(m_rosterFaction);
	int row = 0;
	for (int g = 0; g < static_cast<int>(m_game.generals.size()); ++g)
		if (m_game.generals[g].faction == m_rosterFaction)
		{
			if (row < m_rosterPage * 8 || row >= (m_rosterPage + 1) * 8)
			{
				++row;
				continue;
			}
			if (button(31 + row % 8).leftClicked())
			{
				m_inspect = g;
				m_partner = -1;
			}
			++row;
		}
	const auto& g = m_game.generals[m_inspect];
	const int city = g.home;
	if (m_game.cities[m_targetCity].owner == g.faction)
		for (int c = 0; c < static_cast<int>(m_game.cities.size()); ++c)
			if (m_game.cities[c].owner != g.faction)
			{
				m_targetCity = c;
				break;
			}
	const auto staff = m_game.Available(city);
	const bool available = g.faction == m_game.player &&
	                       std::find(staff.begin(), staff.end(), m_inspect) != staff.end() &&
	                       m_game.result == 0;
	if (available && button(21).leftClicked())
	{
		std::vector<int> options{-1};
		for (int member : staff)
			if (member != m_inspect)
				options.push_back(member);
		auto it = std::find(options.begin(), options.end(), m_partner);
		m_partner = it != options.end() && ++it != options.end() ? *it : options.front();
	}
	if (available && button(22).leftClicked())
		for (int offset = 1; offset <= static_cast<int>(m_game.cities.size()); ++offset)
		{
			const int c = (m_targetCity + offset) % static_cast<int>(m_game.cities.size());
			if (m_game.cities[c].owner != g.faction)
			{
				m_targetCity = c;
				break;
			}
		}
	if (!available)
		return;
	for (int kind = 0; kind < 2; ++kind)
		if (button(kind == 0 ? 23 : 24).leftClicked())
			m_message =
			    m_game.SendMission(city, m_inspect, m_targetCity, static_cast<MissionKind>(kind), m_partner)
			        ? U"任務を開始。金300・命令1を使用。担当者は完了まで出陣できません。"
			        : U"金300・命令1・空いた担当者が必要です。停戦中の勢力へ工作はできません。";
	for (int duty = 0; duty < 4; ++duty)
		if (button(25 + duty).leftClicked())
			m_message = m_game.Develop(city, m_inspect, static_cast<Duty>(duty), false, m_partner)
			                ? U"内政を開始。副担当との親密度は完成時に8上がります。"
			                : U"金500・命令1・空いた担当者・都市の事業枠が必要です。";
}

void CampaignScene::drawCouncil() const
{
	if (m_councilMode == 4)
	{
		drawGovernance();
		return;
	}
	if (m_councilMode == 3)
	{
		drawAssignments();
		return;
	}
	Scene::Rect().draw(ColorF(0.02, 0.04, 0.06, 0.82));
	Rect(60, 102, Scene::Width() - 120, Scene::Height() - 176)
	    .rounded(8)
	    .draw(ColorF(0.09, 0.14, 0.17))
	    .drawFrame(2, ColorF(0.49, 0.60, 0.55));
	DrawButton(button(19), U"閉じる");
	if (m_councilMode == 1)
		DrawButton(Rect(330, 114, 200, 30), U"武将配置へ");
	const auto label = [&](const String& value, int x, int y) {
		FontAsset(U"campaignBody")(value).draw(x, y, ColorF(0.87, 0.90, 0.84));
	};
	if (m_councilMode == 2)
	{
		label(U"群雄の記録 — 今回のプレイで起きた出来事", 80, 115);
		const int rows = Max(1, (Scene::Height() - 320) / 54);
		for (int row = 0; row < rows; ++row)
		{
			const int index = static_cast<int>(m_game.chronicle.size()) - 1 - m_historyPage * rows - row;
			if (index < 0)
				break;
			const auto& event = m_game.chronicle[index];
			Rect(80, 174 + row * 54, Scene::Width() - 160, 50).rounded(4).draw(ColorF(0.13, 0.19, 0.21));
			(void)FontAsset(U"campaignBody")(U"{}日　{}"_fmt(event.day, text(event.text)))
			    .draw(RectF(90, 180 + row * 54, Scene::Width() - 180, 46), ColorF(0.88, 0.88, 0.78));
		}
		DrawButton(button(29), U"新しい記録", m_historyPage > 0);
		DrawButton(button(30), U"古い記録",
		           (m_historyPage + 1) * rows < static_cast<int>(m_game.chronicle.size()));
		label(U"{} / {}頁"_fmt(m_historyPage + 1,
		                       Max(1, (static_cast<int>(m_game.chronicle.size()) + rows - 1) / rows)),
		      390, Scene::Height() - 118);
		return;
	}
	label(U"武将・評定", 80, 115);
	DrawButton(button(20), text(Campaign::FactionName(m_rosterFaction)) + U"軍の武将 / 切替");
	int row = 0;
	for (int member = 0; member < static_cast<int>(m_game.generals.size()); ++member)
		if (m_game.generals[member].faction == m_rosterFaction)
		{
			if (row < m_rosterPage * 8 || row >= (m_rosterPage + 1) * 8)
			{
				++row;
				continue;
			}
			DrawButton(button(31 + row % 8),
			           text(m_game.generals[member].name) + (m_game.Busy(member) ? U" / 任務中"
			                                                 : m_game.generals[member].readyDay > m_game.day
			                                                     ? U" / 休養中"
			                                                     : U""));
			if (member == m_inspect)
				button(31 + row % 8).drawFrame(2, ColorF(0.96, 0.78, 0.40));
			++row;
		}
	drawRosterPages(m_rosterFaction);
	const int middle = 330, right = Scene::Width() - 460;
	Line(314, 156, 314, Scene::Height() - 144).draw(1, ColorF(0.35, 0.45, 0.42));
	Line(right - 16, 156, right - 16, Scene::Height() - 144).draw(1, ColorF(0.35, 0.45, 0.42));
	const auto& g = m_game.generals[m_inspect];
	const auto& home = m_game.cities[g.home];
	label(text(g.name) + U" / " + text(home.name), middle, 163);
	label(U"統率 {}　政治 {}"_fmt(g.leadership, g.politics), middle, 204);
	label(U"知力 {}　魅力 {}"_fmt(g.intelligence, g.charm), middle, 235);
	label(U"個性：" + text(TraitName(g.trait)) + U" / " + text(officer::RoleName(officer::RoleOf(g.name))),
	      middle, 275);
	(void)FontAsset(U"campaignBody")(text(TraitDescription(g.trait)))
	    .draw(RectF(middle, 309, right - middle - 32, 68), ColorF(0.76, 0.85, 0.80));
	label(U"戦法：" + text(TacticName(g.tactic)), middle, 387);
	(void)FontAsset(U"campaignBody")(text(TacticDescription(g.tactic)))
	    .draw(RectF(middle, 420, right - middle - 32, 76), ColorF(0.76, 0.85, 0.80));
	label(m_partner >= 0 ? U"{}との親密度：{}"_fmt(text(m_game.generals[m_partner].name),
	                                               m_game.Affinity(m_inspect, m_partner))
	                     : U"副担当を選ぶと共同任務ができます",
	      middle, 511);
	std::vector<int> friends;
	for (int member = 0; member < static_cast<int>(m_game.generals.size()); ++member)
		if (member != m_inspect && m_game.Affinity(m_inspect, member) > 20)
			friends.push_back(member);
	std::sort(friends.begin(), friends.end(),
	          [&](int a, int b) { return m_game.Affinity(m_inspect, a) > m_game.Affinity(m_inspect, b); });
	String connections = U"深いつながり：";
	for (int i = 0; i < Min(3, static_cast<int>(friends.size())); ++i)
		connections += U"\n{} / 親密度 {}"_fmt(text(m_game.generals[friends[i]].name),
		                                       m_game.Affinity(m_inspect, friends[i]));
	(void)FontAsset(U"campaignSmall")(connections)
	    .draw(RectF(middle, 550, right - middle - 32, 90), ColorF(0.87, 0.80, 0.58));
	const auto staff = m_game.Available(g.home);
	const bool free = g.faction == m_game.player &&
	                  std::find(staff.begin(), staff.end(), m_inspect) != staff.end() && m_game.result == 0;
	label(free ? U"任命元：{} / 金 {}"_fmt(text(home.name), home.gold) : U"任命不可：任務・休養・他勢力",
	      right, 164);
	DrawButton(button(21),
	           m_partner < 0 ? U"副担当：なし / 切替"
	                         : U"副担当：{} / 切替"_fmt(text(m_game.generals[m_partner].name)),
	           free);
	DrawButton(button(22),
	           U"対象：{}（{}） / 切替"_fmt(text(m_game.cities[m_targetCity].name),
	                                        text(Campaign::FactionName(m_game.cities[m_targetCity].owner))),
	           free);
	const int targetFaction = m_game.cities[m_targetCity].owner;
	DrawButton(button(23),
	           U"停戦交渉 / 成功率 {}%"_fmt(
	               m_game.MissionChance(m_inspect, m_targetCity, MissionKind::Diplomacy, m_partner)),
	           free && home.gold >= 300 && m_game.commands > 0 && targetFaction != g.faction);
	FontAsset(U"campaignSmall")(
	    U"金300・命令1・20日。成立すると60日停戦。\n信頼 {} / 君主親密 {} / 停戦 {}日"_fmt(
	        m_game.regard[g.faction][targetFaction], m_game.Affinity(m_inspect, m_game.Leader(targetFaction)),
	        Max(0, m_game.truceUntil[g.faction][targetFaction] - m_game.day)))
	    .draw(right, 350, ColorF(0.76, 0.85, 0.80));
	DrawButton(button(24),
	           U"兵糧攪乱 / 成功率 {}%"_fmt(
	               m_game.MissionChance(m_inspect, m_targetCity, MissionKind::Sabotage, m_partner)),
	           free && home.gold >= 300 && m_game.commands > 0 && m_game.Hostile(g.faction, targetFaction));
	FontAsset(U"campaignSmall")(
	    U"金300・命令1・30日。糧と治安に損害。\n任務失敗：再任用まで10日。副担当も拘束。")
	    .draw(right, 444, ColorF(0.76, 0.85, 0.80));
	label(U"内政：30日 / 金500 / 命令1", right, 488);
	for (int d = 0; d < 4; ++d)
		DrawButton(button(25 + d),
		           text(Campaign::DutyName(static_cast<Duty>(d))) +
		               U" +{}"_fmt(m_game.WorkGain(m_inspect, static_cast<Duty>(d), m_partner)),
		           free && home.worker < 0 && home.gold >= 500 && m_game.commands > 0);
	String status;
	for (const auto& c : m_game.cities)
		if (c.worker == m_inspect || (c.worker >= 0 && c.helper == m_inspect))
			status = U"{}：{} / 残り{}日"_fmt(
			    text(c.name), text(Campaign::DutyName(static_cast<Duty>(c.work))), c.workLeft);
	for (const auto& m : m_game.missions)
		if (m.general == m_inspect || m.helper == m_inspect)
			status = U"{}：{} / 残り{}日"_fmt(text(m_game.cities[m.target].name),
			                                  m.kind == MissionKind::Diplomacy ? U"停戦交渉" : U"兵糧攪乱",
			                                  m.left);
	if (status.isEmpty())
		status = U"共同内政：完成時に親密度 +8。\n近隣の親しい部隊は攻撃力が上がります。";
	(void)FontAsset(U"campaignSmall")(status).draw(RectF(right, 610, 370, 48), ColorF(0.94, 0.83, 0.55));
	(void)FontAsset(U"campaignSmall")(m_message).draw(
	    RectF(80, Scene::Height() - 135, Scene::Width() - 160, 44), ColorF(0.94, 0.83, 0.55));
}
