#include "CampaignSpectacle.hpp"
#include "CampaignVisuals.hpp"
void CampaignSpectacle::DrawMap(const frontline::Campaign& game, const std::function<Vec2(int)>& center,
                                double cell) const
{
	if (!m_enabled)
		return;
	for (int i = 0; i < static_cast<int>(game.armies.size()); ++i)
	{
		const auto& a = game.armies[i];
		if (a.troops <= 0)
			continue;
		const Vec2 p = center(a.tile);
		if (!a.path.empty() && !a.retreat)
			for (int k = 0; k < 5; ++k)
			{
				const double t = std::fmod(m_time * .5 + k * .19 + i * .1, 1.);
				Ellipse(p.movedBy(-cell * .12 - k * 3, cell * .3 - t * 6), cell * (.11 + t * .15), cell * .07)
				    .draw(ColorF(.66, .55, .37, (1 - t) * .12));
			}
		const int c = game.CityAt(a.target);
		if (!a.retreat && c >= 0 && game.Hostile(a.faction, game.cities[c].owner) &&
		    game.MapDistance(a.tile, a.target) <= 1)
		{
			const Vec2 target = center(a.target);
			for (int k = 0; k < 10; ++k)
			{
				const double t = std::fmod(m_time * .6 + k * .117, 1.);
				const Vec2 flame = target + Vec2((k % 5 - 2) * cell * .13, -cell * .3 - t * cell * .8);
				Circle(flame, cell * (.07 + (1 - t) * .09)).draw(ColorF(.13, .12, .11, (1 - t) * .26));
				if (t < .5)
					Triangle(flame + Vec2(-cell * .08, cell * .1), flame + Vec2(0, -cell * .22),
					         flame + Vec2(cell * .08, cell * .1))
					    .draw(ColorF(1., .40, .10, (.5 - t) * .8));
			}
			const double t = std::fmod(m_time * .7 + i * .17, 1.);
			const Vec2 shot = p.lerp(target, t).movedBy(0, -std::sin(t * Math::Pi) * cell * .5);
			Circle(shot, 3).draw(ColorF(1., .8, .35));
			Circle(shot, 8).draw(ColorF(1., .45, .12, .12));
		}
		for (int j = i + 1; j < static_cast<int>(game.armies.size()); ++j)
		{
			const auto& b = game.armies[j];
			if (b.troops <= 0 || !game.Hostile(a.faction, b.faction) ||
			    (!game.CanStrike(i, b.tile) && !game.CanStrike(j, a.tile)))
				continue;
			const Vec2 q = center(b.tile), impact = (p + q) * .5;
			const double t = std::fmod(m_time * 1.5 + i * .27 + j * .11, 1.);
			for (int k = 0; k < 10; ++k)
			{
				const double angle = k * Math::TwoPi / 10 + i;
				const Vec2 dir(std::cos(angle), std::sin(angle) * .5);
				const Vec2 spark = impact + dir * (t * cell * .65);
				Line(spark - dir * 5, spark).draw(1.5, ColorF(1., .78, .37, (1 - t) * .65));
			}
			if (a.arm == frontline::Arm::Bow || b.arm == frontline::Arm::Bow)
			{
				const Vec2 from = a.arm == frontline::Arm::Bow ? p : q,
				           to = a.arm == frontline::Arm::Bow ? q : p;
				for (int k = 0; k < 4; ++k)
				{
					const double flight = std::fmod(m_time * .8 + k * .19, 1.);
					const Vec2 pos =
					    from.lerp(to, flight).movedBy(0, -std::sin(flight * Math::Pi) * cell * .45);
					Line(pos - Vec2(8, -4), pos).draw(1.5, ColorF(.97, .91, .65, .8));
				}
			}
			for (int k = 0; k < 4; ++k)
				Ellipse(impact.movedBy((k - 2) * cell * .1, -t * cell * .2), cell * (.12 + t * .18),
				        cell * (.07 + t * .05))
				    .draw(ColorF(.65, .57, .42, (1 - t) * .13));
		}
	}
	// Captured cities glow briefly; the report never changes ownership itself.
	if (!m_reports.empty() && m_reports.front().event.kind == battlefieldmoments::Kind::Capture)
	{
		const Vec2 p = center(m_reports.front().event.tile);
		const double t = std::fmod(m_age, .9) / .9;
		Circle(p, cell * (.8 + t * 1.4)).drawFrame(3, ColorF(1., .82, .36, (1 - t) * .7));
	}
}
RectF CampaignSpectacle::ReportRect() const
{
	if (!m_enabled || m_reports.empty())
		return RectF(0, 0, 0, 0);
	const bool major = m_reports.front().event.kind == battlefieldmoments::Kind::Capture;
	return RectF(24, major ? Scene::Height() * .24 : Scene::Height() - 304, Min(760., Scene::Width() - 390.),
	             major ? 190 : 136);
}
RectF CampaignSpectacle::CloseRect() const
{
	const auto r = ReportRect();
	return RectF(r.rightX() - 32, r.y + 8, 24, 24);
}
void CampaignSpectacle::DrawReports(const Texture& faces) const
{
	if (!m_enabled || m_reports.empty())
		return;
	const auto& r = m_reports.front();
	const bool major = r.event.kind == battlefieldmoments::Kind::Capture;
	const double alpha = Min(1., Min((m_age + .05) / .2, (5.5 - m_age) / .6));
	const auto rect = ReportRect();
	const double w = rect.w, x = rect.x, y = rect.y, height = rect.h;
	const ColorF accent = r.friendly ? ColorF(.98, .79, .35) : ColorF(.97, .50, .28);
	RectF(x, y, w, height).draw(ColorF(.025, .055, .058, .96 * alpha)).drawFrame(2, ColorF(accent, alpha));
	{
		const ScopedColorMul2D fade(ColorF(1, alpha));
		campaignvisual::OfficerCard(RectF(x + 12, y + 12, major ? 120 : 82, height - 24), r.officer, faces);
	}
	const double tx = x + (major ? 150 : 108);
	FontAsset(U"campaignTitle")(r.title).draw(major ? 36 : 25, tx, y + 14, ColorF(accent, alpha));
	FontAsset(U"campaignSmall")(String(r.officer.name.c_str()) + U" / 戦場の声")
	    .draw(tx, y + (major ? 64 : 49), ColorF(.9, .90, .77, alpha));
	(void)FontAsset(U"campaignBody")(r.quote).draw(
	    RectF(tx, y + (major ? 94 : 75), w - (tx - x) - 18, major ? 64 : 40), ColorF(.96, .95, .85, alpha));
	CloseRect().draw(ColorF(.17, .23, .23, alpha));
	FontAsset(U"campaignSmall")(U"×").drawAt(CloseRect().center(), ColorF(.95, .89, .73, alpha));
	FontAsset(U"campaignSmall")(ReportRect().mouseOver() ? U"閲覧中は表示保持 / クリックで現場へ"
	                                                     : U"クリックで現場へ / ×で閉じる")
	    .draw(tx, y + height - 24, ColorF(.67, .78, .74, alpha));
}
