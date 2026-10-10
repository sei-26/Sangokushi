#include "CampaignScene.hpp"
#include "CampaignUI.hpp"
using namespace frontline;
using namespace campaignui;
namespace
{
	int Rows()
	{
		return Max(1, (Scene::Height() - 440) / 32);
	}
	Rect PageButton(bool next)
	{
		return Rect(next ? 490 : 330, 218 + Rows() * 32 + 8, 150, 30);
	}
} // namespace
void CampaignScene::updateAssignments()
{
	if (button(19).leftClicked() || KeyEscape.down())
	{
		m_councilMode = 0;
		return;
	}
	if (Rect(330, 114, 200, 30).leftClicked())
	{
		m_councilMode = 1;
		return;
	}
	updateRosterPage(m_game.player);
	int row = 0;
	for (int g = 0; g < static_cast<int>(m_game.generals.size()); ++g)
		if (m_game.generals[g].faction == m_game.player)
		{
			if (row < m_rosterPage * 8 || row >= (m_rosterPage + 1) * 8)
			{
				++row;
				continue;
			}
			if (button(31 + row % 8).leftClicked())
				m_inspect = g;
			++row;
		}
	const auto& officer = m_game.generals[m_inspect];
	const int right = Scene::Width() - 460;
	if (m_assignmentTarget < 0 || m_assignmentTarget == officer.home ||
	    m_game.cities[m_assignmentTarget].owner != m_game.player)
	{
		m_assignmentTarget = -1;
		for (int c = 0; c < static_cast<int>(m_game.cities.size()); ++c)
			if (m_game.cities[c].owner == m_game.player && c != officer.home)
			{
				m_assignmentTarget = c;
				break;
			}
	}
	std::vector<int> owned;
	for (int c = 0; c < static_cast<int>(m_game.cities.size()); ++c)
		if (m_game.cities[c].owner == m_game.player)
			owned.push_back(c);
	const int lastPage = Max(0, (static_cast<int>(owned.size()) - 1) / Rows());
	m_assignmentPage = Clamp(m_assignmentPage, 0, lastPage);
	if (PageButton(false).leftClicked())
		m_assignmentPage = Max(0, m_assignmentPage - 1);
	if (PageButton(true).leftClicked())
		m_assignmentPage = Min(lastPage, m_assignmentPage + 1);
	row = 0;
	for (int c = 0; c < static_cast<int>(m_game.cities.size()); ++c)
		if (m_game.cities[c].owner == m_game.player)
		{
			if (row >= m_assignmentPage * Rows() && row < (m_assignmentPage + 1) * Rows() &&
			    Rect(330, 218 + (row % Rows()) * 32, right - 350, 28).leftClicked())
				m_assignmentTarget = c;
			++row;
		}
	if (Rect(right, 390, 370, 42).leftClicked())
		m_message = m_game.AssignOfficer(m_inspect, m_assignmentTarget)
		                ? U"異動を開始。金100・命令1。到着まで武将は任用できません。"
		                : U"空いた武将・金100・命令1・味方領地の連絡路が必要です。";
}
void CampaignScene::drawAssignments() const
{
	Scene::Rect().draw(ColorF(.02, .04, .06, .85));
	Rect(60, 102, Scene::Width() - 120, Scene::Height() - 176)
	    .rounded(8)
	    .draw(ColorF(.09, .14, .17))
	    .drawFrame(2, ColorF(.49, .60, .55));
	DrawButton(button(19), U"閉じる");
	DrawButton(Rect(330, 114, 200, 30), U"武将評定へ");
	FontAsset(U"campaignBody")(U"武将配置").draw(80, 115, ColorF(.95, .85, .60));
	int row = 0;
	for (int g = 0; g < static_cast<int>(m_game.generals.size()); ++g)
		if (m_game.generals[g].faction == m_game.player)
		{
			if (row < m_rosterPage * 8 || row >= (m_rosterPage + 1) * 8)
			{
				++row;
				continue;
			}
			DrawButton(button(31 + row % 8),
			           text(m_game.generals[g].name) + (m_game.Busy(g) ? U" / 任務中" : U""));
			if (g == m_inspect)
				button(31 + row % 8).drawFrame(2, ColorF(.98, .8, .4));
			++row;
		}
	drawRosterPages(m_game.player);
	const int right = Scene::Width() - 460;
	const auto& officer = m_game.generals[m_inspect];
	FontAsset(U"campaignBody")(U"送り先の都市を選択").draw(330, 175, ColorF(.9, .87, .72));
	row = 0;
	for (int c = 0; c < static_cast<int>(m_game.cities.size()); ++c)
		if (m_game.cities[c].owner == m_game.player)
		{
			if (row < m_assignmentPage * Rows() || row >= (m_assignmentPage + 1) * Rows())
			{
				++row;
				continue;
			}
			const int staff = static_cast<int>(m_game.Available(c).size());
			const Rect card(330, 218 + (row % Rows()) * 32, right - 350, 28);
			DrawButton(card, U"{} / 空き武将 {} / 兵站 {}"_fmt(text(m_game.cities[c].name), staff,
			                                                   m_game.cities[c].logistics));
			if (c == m_assignmentTarget)
				card.drawFrame(2, ColorF(.98, .8, .4));
			++row;
		}
	DrawButton(PageButton(false), U"前の都市", m_assignmentPage > 0);
	DrawButton(PageButton(true), U"次の都市", (m_assignmentPage + 1) * Rows() < row);
	FontAsset(U"campaignBody")(text(officer.name) + U" / " + text(m_game.cities[officer.home].name))
	    .draw(right, 175, ColorF(.9, .87, .72));
	const int days = m_game.AssignmentDays(officer.home, m_assignmentTarget);
	const bool free = !m_game.Busy(m_inspect) && officer.readyDay <= m_game.day;
	const String target =
	    m_assignmentTarget >= 0 ? text(m_game.cities[m_assignmentTarget].name) : U"送り先なし";
	FontAsset(U"campaignBody")(U"送り先：{}\n{}\n政治 {} / 得意：{}"_fmt(
	                               target,
	                               days < 0 ? U"味方領地でつながる都市を選択" : U"到着まで {}日"_fmt(days),
	                               officer.politics, text(Campaign::DutyName(officer.specialty))))
	    .draw(right, 229, ColorF(.81, .87, .78));
	DrawButton(Rect(right, 390, 370, 42), U"異動を命じる / 金100・命令1",
	           free && days >= 0 && m_game.cities[officer.home].gold >= 100 && m_game.commands > 0 &&
	               m_game.result == 0);
	String status = U"異動中は内政・出陣・使者に任用不可。\n到着後は送り先の資源を使って活動。\n元の都市には"
	                U"金100を支払います。\n都市が陥落すると中止・10日休養。";
	for (const auto& move : m_game.assignments)
		if (move.general == m_inspect)
			status = U"{}へ異動中 / 残り{}日\n到着後に内政・軍務へ任用できます。"_fmt(
			    text(m_game.cities[move.to].name), move.left);
	(void)FontAsset(U"campaignBody")(status).draw(RectF(right, 462, 370, 180), ColorF(.79, .85, .75));
	(void)FontAsset(U"campaignSmall")(m_message).draw(
	    RectF(80, Scene::Height() - 135, Scene::Width() - 160, 44), ColorF(.94, .83, .55));
}
