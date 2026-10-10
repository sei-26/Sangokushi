#include "CampaignScene.hpp"
#include "CampaignInformation.hpp"
#include "CampaignInfoLayout.hpp"
#include "CampaignUI.hpp"
#include "CampaignVisuals.hpp"
using namespace frontline;
using namespace campaignui;
namespace iu = informationui;
void CampaignScene::drawInformation() const
{
	Scene::Rect().draw(ColorF(.015, .03, .04, .82));
	Rect(40, 104, Scene::Width() - 80, Scene::Height() - 144)
	    .rounded(7)
	    .draw(ColorF(.07, .115, .13))
	    .drawFrame(2, ColorF(.56, .63, .54));
	FontAsset(U"campaignTitle")(U"情報メニュー").draw(64, 115, ColorF(.95, .85, .58));
	FontAsset(U"campaignSmall")(U"閲覧中は日数停止 / 閉じると元の進行に戻ります")
	    .draw(300, 126, ColorF(.74, .82, .76));
	DrawButton(iu::Close(), U"閉じる");
	const String tabs[]{U"城", U"武将", U"勢力", U"部隊", U"記録"};
	for (int i = 0; i < 5; ++i)
	{
		DrawButton(iu::Tab(i), tabs[i]);
		if (m_infoTab == i)
			iu::Tab(i).drawFrame(2, ColorF(.97, .78, .39));
	}
	DrawButton(iu::Filter(),
	           m_infoTab == 4      ? U"全記録"
	           : m_infoFaction < 0 ? U"全勢力 / 切替"
	                               : text(Campaign::FactionName(m_infoFaction)) + U"軍 / 切替",
	           m_infoTab != 4);
	const String sorts[]{U"標準", U"兵力順", U"金順", U"兵糧順"};
	const String officerSorts[]{U"標準", U"統率順", U"知力順", U"政治順"};
	const String armySorts[]{U"標準", U"兵力順", U"士気順", U"携行糧順"};
	const String sort = m_infoTab == 4   ? U"新しい記録順"
	                    : m_infoTab == 1 ? officerSorts[m_infoSort]
	                    : m_infoTab == 3 ? armySorts[m_infoSort]
	                                     : sorts[m_infoSort];
	DrawButton(iu::Sort(), U"並び：" + sort, m_infoTab != 4);
	Line(iu::DetailX() - 12, 208, iu::DetailX() - 12, Scene::Height() - 100).draw(1, ColorF(.34, .44, .42));
	String heading = m_infoTab == 0   ? U"城名"
	                 : m_infoTab == 1 ? U"武将"
	                 : m_infoTab == 2 ? U"勢力"
	                 : m_infoTab == 3 ? U"部隊"
	                                  : U"日付・出来事";
	FontAsset(U"campaignSmall")(heading).draw(76, 250, ColorF(.77, .80, .68));
	String metric = m_infoTab == 1   ? (m_infoSort == 2   ? U"知力"
	                                    : m_infoSort == 3 ? U"政治"
	                                                      : U"統率")
	                : m_infoTab == 3 ? (m_infoSort == 2   ? U"士気"
	                                    : m_infoSort == 3 ? U"携行糧"
	                                                      : U"兵力")
	                                 : (m_infoSort == 2   ? U"金"
	                                    : m_infoSort == 3 ? U"兵糧"
	                                                      : U"兵力");
	if (m_infoTab != 4)
		FontAsset(U"campaignSmall")(U"所属 / " + metric)
		    .draw(Arg::topRight = Vec2(iu::DetailX() - 38, 250), ColorF(.77, .80, .68));
	const auto rows = informationRows();
	for (int row = 0; row < iu::Rows(); ++row)
	{
		const int n = m_infoPage * iu::Rows() + row;
		if (n >= static_cast<int>(rows.size()))
			break;
		const int id = rows[n];
		const auto rect = iu::Row(row);
		rect.rounded(3).draw(id == m_infoSelection ? ColorF(.20, .29, .25)
		                     : rect.mouseOver()    ? ColorF(.16, .22, .21)
		                                           : ColorF(.10, .16, .17));
		if (id == m_infoSelection)
			rect.drawFrame(1.5, ColorF(.97, .78, .39));
		String name;
		int faction = -1;
		int64 value = 0;
		int nameX = rect.x + 10;
		if (m_infoTab == 0)
		{
			const auto& c = m_game.cities[id];
			name = text(c.name);
			faction = c.owner;
			value = m_infoSort == 2 ? c.gold : m_infoSort == 3 ? c.food : c.troops;
		}
		if (m_infoTab == 1)
		{
			const auto& g = m_game.generals[id];
			name = text(g.name);
			faction = g.faction;
			value = m_infoSort == 2 ? g.intelligence : m_infoSort == 3 ? g.politics : g.leadership;
			campaignvisual::OfficerCard(RectF(rect.x + 5, rect.y + 2, 28, 30), g, m_campaignFaces);
			nameX += 30;
		}
		if (m_infoTab == 2)
		{
			name = text(Campaign::FactionName(id)) + U"軍";
			faction = id;
			const auto total = information::Faction(m_game, id);
			value = m_infoSort == 2 ? total.gold : m_infoSort == 3 ? total.food : total.troops;
		}
		if (m_infoTab == 3)
		{
			const auto& a = m_game.armies[id];
			name = text(m_game.generals[a.general].name) + U"隊";
			faction = a.faction;
			value = m_infoSort == 2 ? a.morale : m_infoSort == 3 ? a.food : a.troops;
		}
		if (m_infoTab == 4)
		{
			const auto& c = m_game.chronicle[id];
			name = U"{}日 {}"_fmt(c.day, text(c.text));
			(void)FontAsset(U"campaignSmall")(name).draw(RectF(nameX, rect.y + 7, rect.w - 20, 24),
			                                             ColorF(.90, .92, .83));
		}
		else
		{
			FontAsset(U"campaignBody")(name).draw(nameX, rect.y + 5, ColorF(.94, .92, .80));
			FontAsset(U"campaignSmall")(text(Campaign::FactionName(faction)))
			    .draw(rect.rightX() - 155, rect.y + 8, FactionColor(faction));
			FontAsset(U"campaignSmall")(Format(value))
			    .draw(Arg::topRight = Vec2(rect.rightX() - 12, rect.y + 8), ColorF(.90, .92, .83));
		}
	}
	if (rows.empty())
		FontAsset(U"campaignBody")(U"該当する情報はありません").draw(76, 286, ColorF(.76, .82, .76));
	const int pages = Max(1, (static_cast<int>(rows.size()) + iu::Rows() - 1) / iu::Rows());
	DrawButton(iu::Prev(), U"前の頁", m_infoPage > 0);
	DrawButton(iu::Next(), U"次の頁", m_infoPage + 1 < pages);
	FontAsset(U"campaignSmall")(U"{}件 / {} / {}頁"_fmt(rows.size(), m_infoPage + 1, pages))
	    .draw(308, Scene::Height() - 80, ColorF(.76, .83, .74));
	drawInformationDetail();
}
