#include "CampaignScene.hpp"
#include "CampaignUI.hpp"
#include "CampaignVisuals.hpp"
#include "FormationStatus.hpp"
namespace
{
	Rect FormationClose()
	{
		return Rect(Scene::Width() - 206, 116, 126, 32);
	}
	Rect FormationRow(int row)
	{
		return Rect(Scene::Width() / 2, 238 + row * 78, Scene::Width() / 2 - 84, 70);
	}
	String FormationText(const officer::Link& e)
	{
		String s;
		if (e.attack)
			s += U"攻撃 +{}% "_fmt(e.attack);
		if (e.defense)
			s += U"被害 -{}% "_fmt(e.defense);
		if (e.morale)
			s += U"士気 +{} "_fmt(e.morale / 2);
		if (e.supply)
			s += U"糧消費 -{}%"_fmt(e.supply);
		return s.isEmpty() ? U"補正なし" : s;
	}
} // namespace
void CampaignScene::updateFormation()
{
	if (FormationClose().leftClicked() || KeyEscape.down() || KeyL.down())
	{
		m_councilMode = 0;
		return;
	}
	const auto members = formationstatus::Members(m_game, m_army);
	for (int row = 0; row < static_cast<int>(members.size()); ++row)
		if (FormationRow(row).leftClicked())
		{
			m_army = members[row].army;
			return;
		}
	if (Rect(80, Scene::Height() - 138, 340, 36).leftClicked() && m_army >= 0)
	{
		const auto& a = m_game.armies[m_army];
		const auto p = m_game.hexMap ? hexgrid::Center(a.tile % frontline::Width, a.tile / frontline::Width)
		                             : std::pair<double, double>{a.tile % frontline::Width + .5,
		                                                         a.tile / frontline::Width + .5};
		m_camera.x = p.first;
		m_camera.y = p.second;
		m_camera.zoom = Max(3.5, m_camera.zoom);
		m_camera.Clamp(Scene::Width(), Scene::Height());
		m_positions.clear();
		m_city = m_region = -1;
		m_councilMode = 0;
	}
}
void CampaignScene::drawFormation() const
{
	using namespace campaignui;
	if (m_army < 0 || m_army >= static_cast<int>(m_game.armies.size()))
		return;
	const auto& a = m_game.armies[m_army];
	const auto& g = m_game.generals[a.general];
	const auto total = m_game.Formation(m_army);
	Scene::Rect().draw(ColorF(.015, .035, .04, .87));
	campaignui::Panel(RectF(60, 102, Scene::Width()-120, Scene::Height()-174),true);
	FontAsset(U"campaignTitle")(U"部隊連携 / 仲間の力をつなぐ").draw(80, 116, ColorF(.97, .86, .61));
	DrawButton(FormationClose(), U"閉じる / L");
	campaignvisual::OfficerCard(RectF(80, 190, 118, 144), g, m_campaignFaces);
	FontAsset(U"campaignTitle")(text(g.name) + U"隊").draw(220, 196, ColorF(.98, .88, .68));
	FontAsset(U"campaignBody")(text(officer::RoleName(officer::RoleOf(g.name))) + U" / 士気 {}"_fmt(a.morale))
	    .draw(220, 242, ColorF(.82, .89, .80));
	(void)FontAsset(U"campaignSmall")(text(officer::RoleHelp(officer::RoleOf(g.name))))
	    .draw(RectF(220, 280, Scene::Width() / 2 - 270, 70), ColorF(.77, .84, .76));
	FontAsset(U"campaignBody")(U"現在、この部隊が受ける連携").draw(80, 366, ColorF(.97, .83, .55));
	(void)FontAsset(U"campaignBody")(FormationText(total))
	    .draw(RectF(80, 406, Scene::Width() / 2 - 120, 56), ColorF(.78, .94, .83));
	(void)FontAsset(U"campaignSmall")(
	    U"2マス以内・親密40以上で、仲間の役割が力になる。\n帰還中・士気30未満・糧が尽きた部隊は連携不可。\n同"
	    U"じ役割同士の補正は半分。各効果は最大値だけ適用。\n共に交戦すると10日ごとに親密度 +2。")
	    .draw(RectF(80, 478, Scene::Width() / 2 - 120, 98), ColorF(.78, .84, .76));
	DrawButton(Rect(80, Scene::Height() - 138, 340, 36), U"この部隊を地図で見る");
	FontAsset(U"campaignBody")(U"同じ勢力の出陣部隊 / 名前をクリックして比較")
	    .draw(Scene::Width() / 2, 192, ColorF(.90, .87, .68));
	const auto members = formationstatus::Members(m_game, m_army);
	if (members.empty())
		FontAsset(U"campaignBody")(U"仲間の部隊がまだ出陣していません。")
		    .draw(Scene::Width() / 2, 250, ColorF(.79, .85, .77));
	for (int row = 0; row < static_cast<int>(members.size()); ++row)
	{
		const auto& m = members[row];
		const auto& b = m_game.armies[m.army];
		const auto& ally = m_game.generals[b.general];
		const auto r = FormationRow(row);
		const bool active = formationstatus::Any(m.actual);
		r.rounded(4)
		    .draw(r.mouseOver() ? ColorF(.20, .29, .23) : ColorF(.11, .19, .18))
		    .drawFrame(1, active ? ColorF(.81, .75, .36) : ColorF(.36, .43, .38));
		campaignvisual::OfficerCard(RectF(r.x + 8, r.y + 6, 46, 58), ally, m_campaignFaces);
		FontAsset(U"campaignBody")(text(ally.name) + U" / 親密 {} / {}マス"_fmt(m.affinity, m.distance))
		    .draw(r.x + 66, r.y + 5, ColorF(.96, .90, .72));
		(void)FontAsset(U"campaignSmall")(String(formationstatus::Reason(m.state)) +
		                                  (active ? U" / " + FormationText(m.actual)
		                                   : formationstatus::Any(m.potential)
		                                       ? U" / 見込：" + FormationText(m.potential)
		                                       : U""))
		    .draw(RectF(r.x + 66, r.y + 37, r.w - 80, 27),
		          active ? ColorF(.69, .95, .78) : ColorF(.86, .76, .57));
	}
}
void CampaignScene::drawFormationLinks(double cell) const
{
	if (m_army < 0 || m_army >= static_cast<int>(m_game.armies.size()) || cell < 18)
		return;
	const Vec2 p = tileCenter(m_game.armies[m_army].tile);
	for (const auto& member : formationstatus::Members(m_game, m_army))
	{
		if (!formationstatus::Any(member.actual))
			continue;
		const Vec2 q = tileCenter(m_game.armies[member.army].tile);
		const ColorF color = member.actual.defense  ? ColorF(.40, .84, 1.)
		                     : member.actual.supply ? ColorF(.48, .93, .62)
		                     : member.actual.morale ? ColorF(.97, .82, .38)
		                                            : ColorF(1., .57, .29);
		Line(p, q).draw(2.5, ColorF(color, .7));
		Circle(q, cell * .58).drawFrame(2, ColorF(color, .65));
		if (p.distanceFrom(q) > 16)
		{
			const double t = std::fmod(Scene::Time() * .4 + member.army * .17, 1.);
			Circle(p.lerp(q, t), 3.5).draw(color);
		}
	}
}
