#include "CampaignScene.hpp"
#include "CampaignInformation.hpp"
#include "CampaignInfoLayout.hpp"
using namespace frontline;
namespace iu = informationui;
std::vector<int> CampaignScene::informationRows() const
{
	return information::Rows(m_game, static_cast<information::Tab>(m_infoTab), m_infoFaction, m_infoSort);
}
void CampaignScene::openInformation(int tab, int selected)
{
	m_infoOpen = true;
	m_infoTab = Clamp(tab, 0, 4);
	m_infoFaction = -1;
	m_infoSort = 0;
	m_infoPage = 0;
	m_infoDetailPage = 0;
	m_infoRelations = false;
	const auto rows = informationRows();
	const auto it = std::find(rows.begin(), rows.end(), selected);
	m_infoSelection = it != rows.end() ? selected : rows.empty() ? -1 : rows.front();
	if (it != rows.end())
		m_infoPage = static_cast<int>(it - rows.begin()) / iu::Rows();
}
String CampaignScene::informationPosting(int officer) const
{
	using information::Post;
	const auto p = information::PostOf(m_game, officer);
	if (p.kind == Post::Governor)
		return text(m_game.cities[p.index].name) + U"の太守";
	if (p.kind == Post::Army)
		return U"出陣中：" + text(Campaign::ArmName(m_game.armies[p.index].arm));
	if (p.kind == Post::Transfer)
		return U"異動 → {} / あと{}日"_fmt(text(m_game.cities[m_game.assignments[p.index].to].name), p.days);
	if (p.kind == Post::Mission)
		return U"{}任務 / あと{}日"_fmt(
		    m_game.missions[p.index].kind == MissionKind::Diplomacy ? U"外交" : U"謀略", p.days);
	if (p.kind == Post::Development)
		return U"{}の{} / あと{}日"_fmt(
		    text(m_game.cities[p.index].name),
		    text(Campaign::DutyName(static_cast<Duty>(m_game.cities[p.index].work))), p.days);
	if (p.kind == Post::Rest)
		return U"休養中 / あと{}日"_fmt(p.days);
	if (p.kind == Post::Displaced)
		return U"本拠を喪失 / 任用不可";
	return U"待機中 / 任用可能";
}
void CampaignScene::updateInformation()
{
	if (iu::Close().leftClicked() || KeyEscape.down() || KeyI.down())
	{
		m_infoOpen = false;
		return;
	}
	for (int tab = 0; tab < 5; ++tab)
		if (iu::Tab(tab).leftClicked())
		{
			openInformation(tab);
			return;
		}
	bool reset = false;
	if (m_infoTab != 4 && iu::Filter().leftClicked())
	{
		m_infoFaction = m_infoFaction == 2 ? -1 : m_infoFaction + 1;
		reset = true;
	}
	if (m_infoTab != 4 && iu::Sort().leftClicked())
	{
		m_infoSort = (m_infoSort + 1) % 4;
		reset = true;
	}
	auto rows = informationRows();
	if (reset)
	{
		m_infoPage = 0;
		m_infoSelection = rows.empty() ? -1 : rows.front();
		m_infoDetailPage = 0;
	}
	const int last = Max(0, (static_cast<int>(rows.size()) - 1) / iu::Rows());
	m_infoPage = Clamp(m_infoPage, 0, last);
	const int previousPage = m_infoPage;
	if (iu::Prev().leftClicked())
		m_infoPage = Max(0, m_infoPage - 1);
	if (iu::Next().leftClicked())
		m_infoPage = Min(last, m_infoPage + 1);
	if (Rect(64, 246, iu::DetailX() - 88, iu::Rows() * 38 + 28).mouseOver() && Mouse::Wheel() != 0)
		m_infoPage = Clamp(m_infoPage + (Mouse::Wheel() > 0 ? 1 : -1), 0, last);
	if (m_infoPage != previousPage && !rows.empty())
	{
		m_infoSelection = rows[m_infoPage * iu::Rows()];
		m_infoDetailPage = 0;
	}
	for (int row = 0; row < iu::Rows(); ++row)
	{
		const int index = m_infoPage * iu::Rows() + row;
		if (index >= static_cast<int>(rows.size()))
			break;
		if (iu::Row(row).leftClicked())
		{
			m_infoSelection = rows[index];
			m_infoDetailPage = 0;
		}
	}
	if (m_infoSelection < 0)
		return;
	if (m_infoTab == 1)
	{
		if (iu::Profile().leftClicked())
		{
			m_infoRelations = false;
			m_infoDetailPage = 0;
		}
		if (iu::Relations().leftClicked())
		{
			m_infoRelations = true;
			m_infoDetailPage = 0;
		}
	}
	std::vector<int> links;
	if (m_infoTab == 0)
		links = information::CityMembers(m_game, m_infoSelection);
	if (m_infoTab == 1 && m_infoRelations)
		links = information::Relations(m_game, m_infoSelection);
	if (!links.empty())
	{
		const int lastDetail = Max(0, (static_cast<int>(links.size()) - 1) / iu::DetailRows(m_infoTab));
		m_infoDetailPage = Clamp(m_infoDetailPage, 0, lastDetail);
		if (iu::DetailPrev().leftClicked())
			m_infoDetailPage = Max(0, m_infoDetailPage - 1);
		if (iu::DetailNext().leftClicked())
			m_infoDetailPage = Min(lastDetail, m_infoDetailPage + 1);
		for (int row = 0; row < iu::DetailRows(m_infoTab); ++row)
		{
			const int index = m_infoDetailPage * iu::DetailRows(m_infoTab) + row;
			if (index >= static_cast<int>(links.size()))
				break;
			if (iu::DetailRow(row, m_infoTab).leftClicked())
			{
				openInformation(1, links[index]);
				return;
			}
		}
	}
	if (m_infoTab == 0 && Rect(iu::DetailX() + 12, Scene::Height() - 124, 220, 30).leftClicked() &&
	    m_game.cities[m_infoSelection].owner == m_game.player && m_daysLeft == 0 && m_game.result == 0)
	{
		m_governCity = m_infoSelection;
		m_governChoice = -1;
		m_governPage = 0;
		m_infoOpen = false;
		m_councilMode = 4;
		return;
	}
	if (m_infoTab != 4 && iu::Map().leftClicked())
		focusInformation();
}
void CampaignScene::focusInformation()
{
	int tile = -1, city = -1, army = -1;
	if (m_infoTab == 0)
		city = m_infoSelection;
	if (m_infoTab == 1)
	{
		const auto p = information::PostOf(m_game, m_infoSelection);
		if (p.kind == information::Post::Army)
			army = p.index;
		else
			city = m_game.generals[m_infoSelection].home;
	}
	if (m_infoTab == 2)
	{
		for (int i = 0; i < static_cast<int>(m_game.cities.size()); ++i)
			if (m_game.cities[i].owner == m_infoSelection)
			{
				city = i;
				break;
			}
	}
	if (m_infoTab == 3)
		army = m_infoSelection;
	if (army >= 0)
		tile = m_game.armies[army].tile;
	else if (city >= 0 && city < static_cast<int>(m_game.cities.size()))
		tile = m_game.cities[city].tile;
	if (!Campaign::Valid(tile))
		return;
	const auto p = m_game.hexMap ? hexgrid::Center(tile % Width, tile / Width)
	                             : std::pair<double, double>{tile % Width + .5, tile / Width + .5};
	m_camera.x = p.first;
	m_camera.y = p.second;
	m_camera.zoom = Max(3.2, m_camera.zoom);
	m_camera.Clamp(Scene::Width(), Scene::Height());
	m_army = army;
	m_city = army >= 0 ? -1 : city;
	m_region = -1;
	m_panelHidden = false;
	m_infoOpen = false;
	m_positions.clear();
	m_hits.clear();
}
