#include "CampaignScene.hpp"
#include "CampaignUI.hpp"
#include "CampaignVisuals.hpp"
using namespace frontline;
using namespace campaignui;
namespace
{
	Rect CloseGovernance()
	{
		return Rect(Scene::Width() - 220, 116, 140, 32);
	}
	int CandidateRows()
	{
		return Max(1, Min(8, (Scene::Height() - 390) / 42));
	}
	Rect Candidate(int row)
	{
		return Rect(80, 274 + row * 42, 370, 38);
	}
	Rect GovernAction(bool dismiss)
	{
		return Rect(Scene::Width() / 2 + 24 + (dismiss ? 228 : 0),
		            Scene::Height() - (Scene::Height() < 800 ? 92 : 128), 216, 36);
	}
	Rect GovernPage(bool next)
	{
		return Rect(next ? 270 : 80, 274 + CandidateRows() * 42 + 10, 180, 30);
	}
} // namespace
void CampaignScene::updateGovernance()
{
	if (m_governCity < 0 || m_governCity >= static_cast<int>(m_game.cities.size()))
	{
		m_councilMode = 0;
		return;
	}
	if (CloseGovernance().leftClicked() || KeyEscape.down())
	{
		m_councilMode = 0;
		openInformation(0, m_governCity);
		return;
	}
	const auto staff = m_game.Available(m_governCity);
	const int last = Max(0, (static_cast<int>(staff.size()) - 1) / CandidateRows());
	m_governPage = Clamp(m_governPage, 0, last);
	if (GovernPage(false).leftClicked())
		m_governPage = Max(0, m_governPage - 1);
	if (GovernPage(true).leftClicked())
		m_governPage = Min(last, m_governPage + 1);
	for (int row = 0; row < CandidateRows(); ++row)
	{
		const int n = m_governPage * CandidateRows() + row;
		if (n >= static_cast<int>(staff.size()))
			break;
		if (Candidate(row).leftClicked())
			m_governChoice = staff[n];
	}
	const bool orders = m_daysLeft == 0 && m_game.cities[m_governCity].owner == m_game.player;
	if (orders && GovernAction(false).leftClicked() && m_governChoice >= 0)
	{
		m_message = m_game.AppointGovernor(m_governCity, m_governChoice)
		                ? U"太守を任命。武将は都市運営に専念します。"
		                : U"待機中の所属武将・金100・命令1が必要です。";
		if (m_game.Governor(m_governCity) == m_governChoice)
			m_governChoice = -1;
	}
	if (orders && GovernAction(true).leftClicked())
		m_message = m_game.AppointGovernor(m_governCity, -1) ? U"太守を解任。通常任務に戻せます。"
		                                                     : U"解任には命令1が必要です。";
}
void CampaignScene::drawGovernance() const
{
	if (m_governCity < 0 || m_governCity >= static_cast<int>(m_game.cities.size()))
		return;
	const auto& city = m_game.cities[m_governCity];
	const int d = Scene::Width() / 2 + 24, w = Scene::Width() - d - 96;
	Scene::Rect().draw(ColorF(.02, .04, .06, .86));
	Rect(60, 102, Scene::Width() - 120, Scene::Height() - (Scene::Height() < 800 ? 126 : 154))
	    .rounded(8)
	    .draw(ColorF(.075, .13, .14))
	    .drawFrame(2, ColorF(.56, .57, .39));
	FontAsset(U"campaignTitle")(text(city.name) + U" / 都市運営").draw(80, 116, ColorF(.96, .86, .61));
	DrawButton(CloseGovernance(), U"城情報へ戻る");
	FontAsset(U"campaignBody")(String(cityidentity::Name(m_game.CityKind(m_governCity))))
	    .draw(80, 170, ColorF(.95, .81, .51));
	(void)FontAsset(U"campaignSmall")(String(cityidentity::Description(m_game.CityKind(m_governCity))))
	    .draw(RectF(80, 202, Scene::Width() - 160, 38), ColorF(.80, .87, .79));
	FontAsset(U"campaignBody")(U"太守候補 / 待機中の所属武将").draw(80, 242, ColorF(.9, .9, .82));
	const auto staff = m_game.Available(m_governCity);
	for (int row = 0; row < CandidateRows(); ++row)
	{
		const int n = m_governPage * CandidateRows() + row;
		if (n >= static_cast<int>(staff.size()))
			break;
		const int id = staff[n];
		const auto& g = m_game.generals[id];
		const auto card = Candidate(row);
		DrawButton(card, U"    {} / 政{}・統{}"_fmt(text(g.name), g.politics, g.leadership));
		campaignvisual::OfficerCard(RectF(card.x + 4, card.y + 3, 28, 32), g, m_campaignFaces);
		if (id == m_governChoice)
			card.drawFrame(2, ColorF(.97, .78, .39));
	}
	if (staff.empty())
		FontAsset(U"campaignSmall")(U"候補なし。異動や任務の終了を待とう。")
		    .draw(80, 280, ColorF(.80, .85, .76));
	DrawButton(GovernPage(false), U"前の候補", m_governPage > 0);
	DrawButton(GovernPage(true), U"次の候補",
	           (m_governPage + 1) * CandidateRows() < static_cast<int>(staff.size()));
	const int current = m_game.Governor(m_governCity);
	const int displayed = m_governChoice >= 0 ? m_governChoice : current;
	FontAsset(U"campaignBody")(U"現在の太守：" +
	                           (current >= 0 ? text(m_game.generals[current].name) : U"空席"))
	    .draw(d, 242, ColorF(.95, .86, .65));
	if (displayed >= 0)
	{
		const auto& g = m_game.generals[displayed];
		campaignvisual::OfficerCard(RectF(d, 282, 88, 104), g, m_campaignFaces);
		FontAsset(U"campaignBody")(text(g.name) + (m_governChoice >= 0 ? U" / 候補" : U" / 在任中"))
		    .draw(d + 106, 286, ColorF(.94, .87, .66));
		(void)FontAsset(U"campaignSmall")(
		    U"政治 {} / 統率 {}\n個性：{} / 得意：{}"_fmt(g.politics, g.leadership, text(TraitName(g.trait)),
		                                                  text(Campaign::DutyName(g.specialty))))
		    .draw(RectF(d + 106, 324, w - 106, 68), ColorF(.82, .88, .78));
	}
	const int gold = m_game.CityIncome(m_governCity, false), food = m_game.CityIncome(m_governCity, true),
	          logistics = m_game.CityLogistics(m_governCity), damage = m_game.CityDamagePercent(m_governCity);
	String value =
	    U"現在の都市効果\n月収：金 {} / 糧 {}（府の収入は別）\n実効兵站 {} / 守備兵の被害 {}%\n募兵1回：兵力 +{}"_fmt(
	        gold, food, logistics, damage, m_game.Recruitment(m_governCity));
	if (m_governChoice >= 0)
	{
		// Candidate overrides keep the preview read-only without copying the campaign.

		value += U"\n任命後 → 金 {} / 糧 {}\n兵站 {} / 守備兵の被害 {}%"_fmt(
		    m_game.CityIncome(m_governCity, false, m_governChoice),
		    m_game.CityIncome(m_governCity, true, m_governChoice),
		    m_game.CityLogistics(m_governCity, m_governChoice),
		    m_game.CityDamagePercent(m_governCity, m_governChoice));
	}
	(void)FontAsset(U"campaignSmall")(value).draw(RectF(d, 400, w, 166), ColorF(.84, .9, .81));
	(void)FontAsset(U"campaignSmall")(
	    Scene::Height() < 800
	        ? U"太守は他の任務と兼務不可。解任して再任用。\n任命：金100・命令1 / 解任：命令1。"
	        : U"政治・統率・兵站適性で都市運営の効果が変わる。\n親密な担当武将の内政開発量も上がる。\n太守は"
	          U"他の任務と兼務不可。解任して再任用。\n任命：金100・命令1 / 解任：命令1。")
	    .draw(RectF(d, Scene::Height() < 800 ? 566 : 558, w, Scene::Height() < 800 ? 52 : 96),
	          ColorF(.77, .83, .75));
	const bool orders =
	    m_daysLeft == 0 && city.owner == m_game.player && m_game.result == 0 && m_game.commands > 0;
	DrawButton(GovernAction(false), U"候補を太守に任命", orders && m_governChoice >= 0 && city.gold >= 100);
	DrawButton(GovernAction(true), U"太守を解任", orders && current >= 0);
	(void)FontAsset(U"campaignSmall")(m_message).draw(
	    RectF(80, Scene::Height() - (Scene::Height() < 800 ? 54 : 84), Scene::Width() - 160, 38),
	    ColorF(.95, .84, .56));
}
