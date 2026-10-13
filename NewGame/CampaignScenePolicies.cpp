#include "CampaignScene.hpp"
#include "CampaignUI.hpp"
#include "CampaignVisuals.hpp"
using namespace frontline;
using namespace campaignui;
namespace
{
	int X()
	{
		return Scene::Width() / 2 + 24;
	}
	Rect Close()
	{
		return Rect(Scene::Width() - 240, 116, 160, 32);
	}
	Rect Slot(int n)
	{
		const int w = (Scene::Width() - 160) / 3;
		return Rect(80 + n * w, 180, w - 12, 116);
	}
	int Rows()
	{
		return Max(1, Min(6, (Scene::Height() - 448) / 38));
	}
	Rect Row(int n)
	{
		return Rect(80, 354 + n * 38, X() - 112, 34);
	}
	Rect Page(bool next)
	{
		return Rect(next ? 246 : 80, 354 + Rows() * 38 + 8, 154, 30);
	}
	Rect Choice(int p)
	{
		const int w = (Scene::Width() - X() - 92) / 2;
		return Rect(X() + (p % 2) * (w + 12), 448 + (p / 2) * 48, w, 38);
	}
	Rect Adopt()
	{
		return Rect(X(), Scene::Height() - 118, 306, 38);
	}
	Rect Withdraw()
	{
		return Rect(X() + 318, Scene::Height() - 118, Scene::Width() - X() - 398, 38);
	}
	String PolicyEffectSummary(int p, int power)
	{
		if (p == 0)
			return U"基礎兵糧収入 +{}% / 農政開発 +{}"_fmt(power, power / 5);
		if (p == 1)
			return U"基礎金収入 +{}% / 商業開発 +{}"_fmt(power, power / 5);
		if (p == 2)
			return U"月末治安補正 +{} / 治安開発 +{}"_fmt(1 + power / 10, power / 5);
		return U"実効兵站 +{} / 募兵 +{}%"_fmt(power, power);
	}
} // namespace
std::vector<int> CampaignScene::policyCandidates() const
{
	std::vector<int> members;
	for (int g = 0; g < static_cast<int>(m_game.generals.size()); ++g)
		if (m_game.generals[g].faction == m_game.player && m_game.PolicyEligible(g))
			members.push_back(g);
	std::stable_sort(members.begin(), members.end(),
	                 [&](int a, int b) { return m_game.generals[a].politics > m_game.generals[b].politics; });
	return members;
}
void CampaignScene::openPolicies()
{
	m_councilMode = 8;
	m_policySlot = m_policyPage = 0;
	const auto members = policyCandidates();
	m_policyOfficer = members.empty() ? -1 : members.front();
	m_policyChoice = m_policyOfficer >= 0
	                     ? (m_game.generals[m_policyOfficer].policy < 0
	                            ? static_cast<int>(m_game.generals[m_policyOfficer].specialty)
	                            : m_game.generals[m_policyOfficer].policy)
	                     : 0;
	m_message = U"都市の発展で政策枠が増えます。武将を選び、提案する政策を採用しよう。";
}
void CampaignScene::updatePolicies()
{
	if (Close().leftClicked() || KeyEscape.down())
	{
		openDomestic(m_domesticCity);
		return;
	}
	for (int n = 0; n < 3; ++n)
		if (Slot(n).leftClicked())
			m_policySlot = n;
	const auto members = policyCandidates();
	const int last = Max(0, (static_cast<int>(members.size()) - 1) / Rows());
	m_policyPage = Clamp(m_policyPage, 0, last);
	if (Page(false).leftClicked())
		m_policyPage = Max(0, m_policyPage - 1);
	if (Page(true).leftClicked())
		m_policyPage = Min(last, m_policyPage + 1);
	for (int row = 0; row < Rows(); ++row)
	{
		const int index = m_policyPage * Rows() + row;
		if (index >= static_cast<int>(members.size()))
			break;
		if (Row(row).leftClicked())
		{
			m_policyOfficer = members[index];
			const auto& g = m_game.generals[m_policyOfficer];
			m_policyChoice = g.policy < 0 ? static_cast<int>(g.specialty) : g.policy;
		}
	}
	for (int p = 0; p < 4; ++p)
		if (Choice(p).leftClicked())
			m_policyChoice = p;
	if (m_daysLeft != 0)
		return;
	if (Adopt().leftClicked())
		m_message = m_game.AdoptPolicy(m_policySlot, m_policyOfficer, static_cast<Duty>(m_policyChoice))
		                ? U"政策を採用。武将が国内にいる間、自勢力の全都市に効果が出ます。"
		                : U"有効な武将・本拠の金200・命令1・解放済みの枠が必要。同じ武将・政策の重複不可。";
	if (Withdraw().leftClicked())
		m_message = m_game.RemovePolicy(m_game.player, m_policySlot)
		                ? U"この政策枠を撤回しました。命令1を使用。"
		                : U"撤回には命令1が必要です。";
}
void CampaignScene::drawPolicies() const
{
	const int d = X(), slots = m_game.PolicySlots(m_game.player);
	Scene::Rect().draw(ColorF(.015, .03, .03, .92));
	campaignui::Panel(RectF(60, 102, Scene::Width()-120, Scene::Height()-126),true);
	FontAsset(U"campaignTitle")(U"施政 / 国の政策").draw(80, 116, ColorF(.97, .87, .62));
	DrawButton(Close(), U"内政へ戻る");
	for (int n = 0; n < 3; ++n)
	{
		const auto r = Slot(n);
		const int member = m_game.policyCouncil[m_game.player][n];
		r.rounded(4)
		    .draw(n < slots ? ColorF(.13, .23, .33) : ColorF(.055, .09, .14))
		    .drawFrame(n == m_policySlot ? 2 : 1,
		               ColorF(n == m_policySlot ? ColorF(.98, .79, .38) : ColorF(.41, .50, .57)));
		FontAsset(U"campaignSmall")(U"政策枠 {} / {}"_fmt(n + 1, n < slots ? U"解放済み" : U"休止中"))
		    .draw(r.x + 12, r.y + 7, ColorF(.94, .85, .58));
		if (member >= 0)
		{
			const auto& g = m_game.generals[member];
			campaignvisual::OfficerCard(RectF(r.x + 12, r.y + 34, 40, 47), g, m_campaignFaces);
			FontAsset(U"campaignBody")(text(g.name) + U" / " + String(nationalpolicy::Name(g.policy)))
			    .draw(r.x + 64, r.y + 35, ColorF(.96, .89, .72));
			FontAsset(U"campaignSmall")(
			    n < slots && m_game.PolicyEligible(member)
			        ? PolicyEffectSummary(g.policy,
			                              m_game.PolicyStrength(member, static_cast<Duty>(g.policy)))
			        : U"出陣・不在・枠不足で効果休止")
			    .draw(r.x + 12, r.y + 87, ColorF(.65, .86, .74));
		}
		else
			FontAsset(U"campaignBody")(n < slots ? U"武将の提案を採用できる"
			                           : n == 1  ? U"発展都市を作ると解放"
			                                     : U"大都市を作ると解放")
			    .draw(r.x + 12, r.y + 45, ColorF(.74, .81, .74));
	}
	FontAsset(U"campaignSmall")(U"政策枠 {} / 3　政治力で効果が変化。同じ政策・武将は1枠まで。"_fmt(slots))
	    .draw(80, 309, ColorF(.83, .88, .77));
	FontAsset(U"campaignBody")(U"国内にいる武将 / 政治力順").draw(80, 328, ColorF(.95, .83, .52));
	const auto members = policyCandidates();
	for (int row = 0; row < Rows(); ++row)
	{
		const int index = m_policyPage * Rows() + row;
		if (index >= static_cast<int>(members.size()))
			break;
		const int id = members[index];
		const auto& g = m_game.generals[id];
		const auto r = Row(row);
		DrawButton(r, U"    {} / 政{} / {}"_fmt(text(g.name), g.politics, text(m_game.cities[g.home].name)));
		campaignvisual::OfficerCard(RectF(r.x + 4, r.y + 2, 25, 30), g, m_campaignFaces);
		if (id == m_policyOfficer)
			r.drawFrame(2, ColorF(.97, .78, .38));
	}
	DrawButton(Page(false), U"前の武将", m_policyPage > 0);
	DrawButton(Page(true), U"次の武将",
	           m_policyPage < Max(0, (static_cast<int>(members.size()) - 1) / Rows()));
	if (m_policyOfficer >= 0)
	{
		const auto& g = m_game.generals[m_policyOfficer];
		campaignvisual::OfficerCard(RectF(d, 350, 72, 86), g, m_campaignFaces);
		FontAsset(U"campaignBody")(text(g.name) + U"の提案").draw(d + 90, 352, ColorF(.96, .87, .64));
		FontAsset(U"campaignSmall")(U"政治 {} / 本拠の金 {}"_fmt(g.politics, m_game.cities[g.home].gold))
		    .draw(d + 90, 381, ColorF(.82, .88, .78));
		FontAsset(U"campaignSmall")(
		    U"得意：{} / 個性：{}"_fmt(text(Campaign::DutyName(g.specialty)), text(TraitName(g.trait))))
		    .draw(d + 90, 405, ColorF(.82, .88, .78));
		FontAsset(U"campaignSmall")(U"兼務可 / 出陣・不在・負傷中は休止")
		    .draw(d + 90, 429, ColorF(.82, .88, .78));
	}
	for (int p = 0; p < 4; ++p)
	{
		DrawButton(Choice(p), String(nationalpolicy::Name(p)));
		if (p == m_policyChoice)
			Choice(p).drawFrame(2, ColorF(.98, .78, .38));
	}
	const int power =
	    m_policyOfficer >= 0 ? m_game.PolicyStrength(m_policyOfficer, static_cast<Duty>(m_policyChoice)) : 0;
	FontAsset(U"campaignSmall")(PolicyEffectSummary(m_policyChoice, power))
	    .draw(d, 547, ColorF(.94, .87, .66));
	FontAsset(U"campaignSmall")(nationalpolicy::Description(m_policyChoice))
	    .draw(d, 573, ColorF(.91, .75, .53));

	const bool orders = m_daysLeft == 0 && m_game.result == 0 && m_game.commands > 0;
	DrawButton(Adopt(), U"採用 / 本拠の金200・命令1",
	           orders && m_policySlot < slots && m_game.PolicyEligible(m_policyOfficer));
	DrawButton(Withdraw(), U"撤回 / 命令1", orders && m_game.policyCouncil[m_game.player][m_policySlot] >= 0);
	(void)FontAsset(U"campaignSmall")(m_message).draw(
	    RectF(80, Scene::Height() - 60, Scene::Width() - 160, 36), ColorF(.96, .85, .57));
}
