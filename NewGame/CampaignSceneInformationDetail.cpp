#include "CampaignScene.hpp"
#include "CampaignInformation.hpp"
#include "CampaignInfoLayout.hpp"
#include "CampaignUI.hpp"
#include "CampaignVisuals.hpp"
using namespace frontline;
using namespace campaignui;
namespace iu = informationui;
void CampaignScene::drawInformationDetail() const
{
	const int id = m_infoSelection, d = iu::DetailX() + 12, w = iu::DetailWidth() - 24;
	const auto label = [&](const String& value, int y, ColorF color = ColorF(.89, .91, .82)) {
		FontAsset(U"campaignBody")(value).draw(d, y, color);
	};
	const auto paragraph = [&](const String& value, int y, int height = 64) {
		(void)FontAsset(U"campaignSmall")(value).draw(RectF(d, y, w, height), ColorF(.77, .84, .79));
	};
	if (id < 0)
	{
		label(U"一覧から情報を選択してください", 232);
		return;
	}
	std::vector<int> links;
	if (m_infoTab == 0)
	{
		const auto& c = m_game.cities[id];
		FontAsset(U"campaignTitle")(text(c.name) + U" / 城情報").draw(d, 218, ColorF(.96, .86, .61));
		label(text(Campaign::FactionName(c.owner)) +
		          U"軍 / {} / 座標 ({}, {})"_fmt(String(cityidentity::Name(m_game.CityKind(id))),
		                                         c.tile % Width, c.tile / Width),
		      258, FactionColor(c.owner));
		label(U"守備兵 {}　金 {}　兵糧 {}"_fmt(c.troops, c.gold, c.food), 292);
		const String names[]{U"農政", U"商業", U"治安", U"兵站"};
		const int levels[]{c.farming, c.commerce, c.order, c.logistics};
		const int cw = (w - 24) / 4;
		for (int i = 0; i < 4; ++i)
		{
			Rect(d + i * (cw + 8), 330, cw, 54).rounded(3).draw(ColorF(.13, .20, .19));
			FontAsset(U"campaignSmall")(names[i]).draw(d + i * (cw + 8) + 10, 335, ColorF(.75, .81, .73));
			FontAsset(U"campaignBody")(Format(levels[i]))
			    .draw(d + i * (cw + 8) + 10, 355, ColorF(.98, .87, .60));
		}
		int hubs = 0;
		for (const auto& r : m_game.regions)
			if (r.city == id && r.owner == c.owner && m_game.CityAt(r.tile) < 0)
				++hubs;
		paragraph(U"都市月収：金 {} / 糧 {}（府の収入は別） / 所管府 {}"_fmt(
		              m_game.CityIncome(id, false), m_game.CityIncome(id, true), hubs),
		          392, 28);
		paragraph(c.worker >= 0
		              ? U"内政：{} / {} / あと{}日"_fmt(
		                    text(Campaign::DutyName(static_cast<Duty>(c.work))),
		                    text(m_game.generals[c.worker].name) +
		                        (c.helper >= 0 ? U"・" + text(m_game.generals[c.helper].name) : U""),
		                    c.workLeft)
		              : U"内政：実施中の事業なし",
		          422, 28);
		links = information::CityMembers(m_game, id);
		label(
		    U"本拠武将 {}人 / 任用可能 {}人（名前から詳細へ）"_fmt(links.size(), m_game.Available(id).size()),
		    456);
	}
	else if (m_infoTab == 1)
	{
		const auto& g = m_game.generals[id];
		campaignvisual::OfficerCard(RectF(d, 224, 112, 128), g, m_campaignFaces);
		FontAsset(U"campaignTitle")(text(g.name)).draw(d + 130, 222, ColorF(.96, .86, .61));
		FontAsset(U"campaignBody")(text(Campaign::FactionName(g.faction)) + U"軍 / 本拠：" +
		                           text(m_game.cities[g.home].name))
		    .draw(d + 130, 262, FactionColor(g.faction));
		FontAsset(U"campaignBody")(
		    U"統率 {}　政治 {}\n知力 {}　魅力 {}"_fmt(g.leadership, g.politics, g.intelligence, g.charm))
		    .draw(d + 130, 294, ColorF(.92, .93, .83));
		paragraph(informationPosting(id) + U" / 得意：" + text(Campaign::DutyName(g.specialty)), 364, 30);
		DrawButton(iu::Profile(), U"能力・個性・戦法");
		DrawButton(iu::Relations(), U"親密度・連携");
		(m_infoRelations ? iu::Relations() : iu::Profile()).drawFrame(2, ColorF(.97, .78, .39));
		if (m_infoRelations)
		{
			links = information::Relations(m_game, id);
			paragraph(U"親密度が高い順 / 40・60・80で連携が強化 / 名前から詳細へ", 430, 22);
		}
		else
		{
			label(U"個性：" + text(TraitName(g.trait)) + U" / " +
			          text(officer::RoleName(officer::RoleOf(g.name))),
			      446, ColorF(.95, .82, .54));
			paragraph(text(TraitDescription(g.trait)), 478, 50);
			label(U"戦法：" + text(TacticName(g.tactic)), 534, ColorF(.95, .82, .54));
			paragraph(text(TacticDescription(g.tactic)) + U"\n発動：士気15 / 再使用30日", 566, 60);
		}
	}
	else if (m_infoTab == 2)
	{
		const auto totals = information::Faction(m_game, id);
		const int leader = m_game.Leader(id);
		if (leader >= 0)
			campaignvisual::OfficerCard(RectF(d, 224, 88, 100), m_game.generals[leader], m_campaignFaces);
		FontAsset(U"campaignTitle")(text(Campaign::FactionName(id)) + U"軍 / 勢力情報")
		    .draw(d + 108, 224, ColorF(.96, .86, .61));
		label(U"城 {} / 府 {} / 武将 {} / 出陣部隊 {}"_fmt(totals.cities, totals.regions, totals.officers,
		                                                   totals.armies),
		      344);
		label(U"総兵力 {}　保有金 {}"_fmt(totals.troops, totals.gold), 384);
		label(U"総兵糧 {}（城・携行糧・輸送の積荷）"_fmt(totals.food), 422);
		paragraph(U"総兵力は城の守備兵と、生存する全部隊の合計。", 454, 34);
		label(U"外交状況", 508, ColorF(.95, .82, .54));
		int row = 0;
		for (int other = 0; other < 3; ++other)
			if (other != id)
			{
				const int days = Max(0, m_game.truceUntil[id][other] - m_game.day);
				label(text(Campaign::FactionName(other)) + U"軍 / 信頼 {} / "_fmt(m_game.regard[id][other]) +
				          (days > 0 ? U"停戦 あと{}日"_fmt(days) : U"交戦可能"),
				      546 + row++ * 38);
			}
	}
	else if (m_infoTab == 3)
	{
		const auto& a = m_game.armies[id];
		const auto& g = m_game.generals[a.general];
		campaignvisual::OfficerCard(RectF(d, 224, 112, 128), g, m_campaignFaces);
		FontAsset(U"campaignTitle")(text(g.name) + U"隊").draw(d + 130, 224, ColorF(.96, .86, .61));
		FontAsset(U"campaignBody")(text(Campaign::FactionName(a.faction)) + U"軍 / " +
		                           text(Campaign::ArmName(a.arm)))
		    .draw(d + 130, 266, FactionColor(a.faction));
		FontAsset(U"campaignBody")(
		    U"兵力 {} / 士気 {}\n携行糧 {} / 輸送積荷 {}"_fmt(a.troops, a.morale, a.food, a.cargoFood))
		    .draw(d + 130, 302, ColorF(.92, .93, .83));
		label(U"位置 ({}, {}) / 補給：{}"_fmt(a.tile % Width, a.tile / Width, a.supplied ? U"接続" : U"途絶"),
		      382);
		const int target = m_game.CityAt(a.target);
		label(U"行先：" +
		          (target >= 0 ? text(m_game.cities[target].name)
		                       : U"({}, {})"_fmt(a.target % Width, a.target / Width)) +
		          U" / 残り{}マス"_fmt(a.path.size()),
		      422);
		label(a.retreat              ? U"行動：帰還中"
		      : a.aiAssemblyDays > 0 ? U"行動：集結待ち {}日"_fmt(a.aiAssemblyDays)
		                             : U"行動：進軍・守備",
		      460);
		paragraph(U"構え：{} / 射程 {}\n武将補正：野戦 {}% / 攻城 {}% / 被害 {}%"_fmt(
		              String(battle::Name(a.stance)), m_game.AttackRange(id), m_game.OfficerAttackPercent(id),
		              m_game.OfficerAttackPercent(id, true), m_game.OfficerDamagePercent(id)),
		          500, 66);
		label(U"戦法：" + text(TacticName(g.tactic)), 562, ColorF(.95, .82, .54));
		paragraph(a.tacticQueued ? U"発動予約済み"
		                         : U"効果 残り{}日 / 再使用まで{}日"_fmt(
		                               a.tacticLeft, Max(0, a.tacticReadyDay - m_game.day)),
		          598, 30);
	}
	else
	{
		const auto& event = m_game.chronicle[id];
		label(U"{}日 / 群雄の記録"_fmt(event.day), 228, ColorF(.95, .82, .54));
		(void)FontAsset(U"campaignBody")(text(event.text))
		    .draw(RectF(d, 282, w, Scene::Height() - 414), ColorF(.90, .92, .84));
	}
	if (!links.empty())
	{
		for (int row = 0; row < iu::DetailRows(m_infoTab); ++row)
		{
			const int n = m_infoDetailPage * iu::DetailRows(m_infoTab) + row;
			if (n >= static_cast<int>(links.size()))
				break;
			const int member = links[n];
			const auto rect = iu::DetailRow(row, m_infoTab);
			rect.rounded(3).draw(rect.mouseOver() ? ColorF(.19, .27, .23) : ColorF(.12, .18, .18));
			const auto& g = m_game.generals[member];
			campaignvisual::OfficerCard(RectF(rect.x + 4, rect.y + 2, 24, 28), g, m_campaignFaces);
			FontAsset(U"campaignBody")(text(g.name)).draw(rect.x + 38, rect.y + 3, ColorF(.93, .91, .81));
			const String detail =
			    m_infoTab == 0
			        ? informationPosting(member)
			        : text(Campaign::FactionName(g.faction)) +
			              U"軍 / 親密 {}/100 / 連携 Lv{}"_fmt(m_game.Affinity(id, member),
			                                                  officer::BondTier(m_game.Affinity(id, member)));
			(void)FontAsset(U"campaignSmall")(detail).draw(RectF(rect.x + 134, rect.y + 6, rect.w - 146, 26),
			                                               ColorF(.79, .86, .79));
		}
		const int pages = Max(1, (static_cast<int>(links.size()) + iu::DetailRows(m_infoTab) - 1) /
		                             iu::DetailRows(m_infoTab));
		DrawButton(iu::DetailPrev(), U"前の武将", m_infoDetailPage > 0);
		DrawButton(iu::DetailNext(), U"次の武将", m_infoDetailPage + 1 < pages);
	}
	if (m_infoTab == 0 && m_game.cities[id].owner == m_game.player)
		DrawButton(Rect(iu::DetailX() + 12, Scene::Height() - 124, 220, 30), U"都市運営・太守の任命",
		           m_daysLeft == 0 && m_game.result == 0);
	if (m_infoTab != 4)
		DrawButton(iu::Map(), U"地図で見る", m_infoTab != 2 || information::Faction(m_game, id).cities > 0);
}
