#include "StoryPresentation.hpp"
void StoryPresentation::BattleSpectacle(const hero::Story& s, const RectF& r) const
{
	if (s.chapter == 4 && s.fireUsed)
	{
		const double cell = r.w / hero::W;
		for (int tile = 0; tile < hero::W * hero::H; ++tile)
			if (s.terrain[tile] == 2 && (tile % hero::W) % 2 == 0)
			{
				const Vec2 p = r.pos + Vec2((tile % hero::W + .5) * cell, (tile / hero::W + .5) * cell);
				for (int k = 0; k < 5; ++k)
				{
					const double flicker = .75 + .25 * std::sin(m_time * 12 + k + tile);
					const Vec2 base = p.movedBy((k - 2) * cell * .10, cell * .05);
					Triangle(base.movedBy(-cell * .09, 0), base.movedBy(0, -cell * .48 * flicker),
					         base.movedBy(cell * .09, 0))
					    .draw(ColorF(1., .29, .07, .8));
					Triangle(base.movedBy(-cell * .04, 0), base.movedBy(0, -cell * .28 * flicker),
					         base.movedBy(cell * .04, 0))
					    .draw(ColorF(1., .83, .22, .9));
					const double rise = std::fmod(m_time * .65 + k * .21, 1.);
					Circle(p.movedBy(k * 2 - 4, -cell * rise * .47), 2 + rise * 5)
					    .draw(ColorF(.16, .13, .10, (1 - rise) * .3));
				}
			}
	}
	if (m_moment <= 0 || m_moment > 1.1)
		return;
	const double cell = r.w / hero::W, t = Clamp((1.1 - m_moment) / 1.1, 0., 1.), fade = 1 - t;
	Vec2 origin = r.center();
	for (const auto& u : s.units)
		if (u.hero == m_hero)
		{
			origin = r.pos + Vec2((u.x + .5) * cell, (u.y + .5) * cell);
			break;
		}
	const ColorF color = m_hero == 4   ? ColorF(1., .40, .10)
	                     : m_hero == 1 ? ColorF(.46, 1., .62)
	                     : m_hero == 2 ? ColorF(.45, .85, 1.)
	                                   : ColorF(1., .82, .35);
	Circle(origin, cell * (.5 + t * 2.5)).drawFrame(3 + fade * 3, ColorF(color, fade * .8));
	for (int i = 0; i < 36; ++i)
	{
		const double angle = i * Math::TwoPi / 36;
		const Vec2 d(std::cos(angle), std::sin(angle));
		const Vec2 p = origin + d * cell * (.3 + t * 2.5);
		if (m_hero == 2)
		{
			Line(p, p + Vec2(-d.y, d.x) * cell * .14).draw(4, ColorF(color, fade));
		}
		else if (m_hero == 1)
		{
			Line(p - d * cell * .5, p).draw(2 + fade * 2, ColorF(color, fade));
		}
		else
			Circle(p.movedBy(0, -t * cell * .35), 2 + fade * 2).draw(ColorF(color, fade));
	}
	if (s.chapter == 4 && (s.fireUsed || m_hero == 4))
		for (const auto& u : s.units)
			if (u.enemy && u.hp > 0)
			{
				const Vec2 p = r.pos + Vec2((u.x + .5) * cell, (u.y + .5) * cell);
				for (int i = 0; i < 5; ++i)
				{
					const double size = cell * .12 * fade;
					const Vec2 f = p.movedBy((i - 2) * cell * .15, -std::sin(t * 6 + i) * cell * .12);
					Triangle(f + Vec2(-size, 0), f + Vec2(0, -size * 3.5), f + Vec2(size, 0))
					    .draw(ColorF(1., .38, .12, fade * .75));
				}
			}
}
