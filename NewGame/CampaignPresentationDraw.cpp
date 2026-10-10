#include "CampaignPresentation.hpp"
#include "CampaignVisuals.hpp"
#include "CampaignUI.hpp"
using frontline::Tactic;
ColorF CampaignPresentation::Color(Tactic tactic)
{
	if (tactic == Tactic::Fire || tactic == Tactic::SiegeStrike)
		return ColorF(1., .39, .12);
	if (tactic == Tactic::Fortify || tactic == Tactic::Volley)
		return ColorF(.40, .83, 1.);
	if (tactic == Tactic::Supply || tactic == Tactic::Rally)
		return ColorF(.51, 1., .58);
	if (tactic == Tactic::Ambush)
		return ColorF(.76, .57, 1.);
	return ColorF(1., .80, .30);
}
String CampaignPresentation::Call(Tactic tactic)
{
	const String calls[]{U"全軍、志を一つに！",
	                     U"敵陣を切り開け！",
	                     U"ここは一歩も譲らぬ！",
	                     U"矢を放て！",
	                     U"炎で敵の陣を崩せ！",
	                     U"兵糧をつなぎ、戦線を支えよ！",
	                     U"騎兵、今こそ突き進め！",
	                     U"地の利を生かせ。今だ！",
	                     U"城門を砕け！"};
	return calls[static_cast<int>(tactic)];
}
void CampaignPresentation::DrawMap(const std::function<Vec2(int)>& center, double cell) const
{
	if (!Busy() || m_elapsed < .8)
		return;
	const auto& e = m_queue.front().event;
	const Vec2 p = center(e.target), source = center(e.tile);
	const double t = Clamp((m_elapsed - .8) / .85, 0., 1.), fade = 1. - t;
	const auto color = Color(e.tactic);
	const double radius = Max(26., cell * .8) + t * Max(75., cell * 2.5);
	if (e.target != e.tile)
		Line(source, p).draw(5, ColorF(color, fade * .8));
	Circle(p, radius).drawFrame(3 + 5 * fade, ColorF(color, fade * .8));
	Circle(p, radius * .72).draw(ColorF(color, fade * .09)).drawFrame(1, ColorF(1., .94, .71, fade * .6));
	const bool shield = e.tactic == Tactic::Fortify;
	const bool fire = e.tactic == Tactic::Fire;
	Circle(p, Max(20., cell * .7) * (1 + t)).draw(ColorF(color, fade * .20));
	if (fire)
		for (int i = 0; i < 9; ++i)
		{
			const Vec2 base = p + Vec2((i - 4) * cell * .22, std::sin(i * 1.7) * cell * .20 - t * 35);
			const double size = Max(12., cell * .3) * fade, height = size * (2 + (i % 3) * .5);
			Triangle(base + Vec2(-size, 0), base + Vec2(0, -height), base + Vec2(size, 0))
			    .draw(ColorF(1., .29, .06, fade * .85));
			Triangle(base + Vec2(-size * .45, 0), base + Vec2(0, -height * .65), base + Vec2(size * .45, 0))
			    .draw(ColorF(1., .86, .28, fade));
		}

	for (int i = 0; i < 24; ++i)
	{
		const double angle = i * Math::TwoPi / 24 + e.general * .31;
		const Vec2 direction(std::cos(angle), std::sin(angle));
		const Vec2 point = p + direction * (radius * (.55 + (i % 4) * .12));
		if (shield)
			Line(p + direction * radius,
			     p + Vec2(std::cos(angle + Math::TwoPi / 24), std::sin(angle + Math::TwoPi / 24)) * radius)
			    .draw(3, ColorF(color, fade));
		else if (fire)
			Circle(point.movedBy(0, -t * 60), 2 + fade * (i % 4 + 2))
			    .draw(ColorF(1., .3 + (i % 3) * .13, .08, fade));
		else if (e.tactic == Tactic::Volley)
			Line(point.movedBy(-14, -34 * fade), point).draw(2, ColorF(color, fade));
		else
			Line(point - direction * (18 + 25 * fade), point).draw(2 + fade * 2, ColorF(color, fade));
	}
}
void CampaignPresentation::DrawOverlay(const Texture& faces) const
{
	if (!Busy())
		return;
	const auto& moment = m_queue.front();
	const auto color = Color(moment.event.tactic);
	const double enter = Clamp(m_elapsed / .18, 0., 1.), leave = Clamp((1.02 - m_elapsed) / .17, 0., 1.),
	             alpha = enter * leave;
	if (alpha <= 0)
		return;
	const double w = Scene::Width(), h = Scene::Height();
	const double top = h * .24, height = Min(300., h * .39), slide = (1. - enter) * w * .20;
	RectF(0, 0, w, h).draw(ColorF(.015, .02, .03, alpha * .26));
	const RectF band(slide, top, w, height);
	band.draw(ColorF(.025, .045, .055, alpha * .97));
	RectF(slide, top, w, 4).draw(ColorF(color, alpha));
	RectF(slide, top + height - 4, w, 4).draw(ColorF(.96, .81, .40, alpha));
	for (int i = 0; i < 26; ++i)
	{
		const double y = top + 12 + (i * 53) % static_cast<int>(height - 24);
		const double x = std::fmod(i * 117 + m_elapsed * 550, w);
		Line(x, y, x + 90 + (i % 4) * 50, y - 22).draw(1 + (i % 3), ColorF(color, alpha * .16));
	}
	const double portrait = height - 32, left = w * .08 + slide;
	campaignvisual::OfficerCard(RectF(left, top + 16, portrait, portrait), moment.officer, faces);
	const double tx = left + portrait + 34;
	FontAsset(U"campaignBody")(String(moment.officer.name.c_str()) + U" — " +
	                           String(frontline::Campaign::FactionName(moment.event.faction).c_str()) + U"軍")
	    .draw(28, tx, top + 26, ColorF(.98, .91, .71, alpha));
	FontAsset(U"campaignTitle")(String(frontline::TacticName(moment.event.tactic).c_str()))
	    .draw(52, tx, top + 75, ColorF(color, alpha));
	FontAsset(U"campaignBody")(Call(moment.event.tactic))
	    .draw(24, tx, top + 151, ColorF(.98, .96, .85, alpha));
	FontAsset(U"campaignSmall")(U"戦法発動　/　クリック・Spaceで次へ　/　M：音の切替")
	    .draw(tx, top + height - 45, ColorF(.73, .81, .80, alpha));
	if (m_elapsed < .08)
		RectF(0, 0, w, h).draw(ColorF(.99, .87, .60, (1. - m_elapsed / .08) * .17));
}
