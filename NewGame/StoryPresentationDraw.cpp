#include "StoryPresentation.hpp"

void StoryPresentation::Landscape(const RectF& r, double opacity) const
{
	if (!m_land)
		return;
	double scale = Max(r.w / m_land.width(), r.h / m_land.height());
	m_land.scaled(scale).drawAtClipped(r.center(), r, ColorF(1, opacity));
}

void StoryPresentation::Portrait(int who, const RectF& r, double opacity) const
{
	if (!m_faces)
	{
		r.draw(ColorF(.15, .25, .22));
		return;
	}
	int w = m_faces.width() / 3, h = m_faces.height() / 2;
	who = Clamp(who, 0, 5);
	m_faces(Rect(w * (who % 3), h * (who / 3), w, h)).resized(r.w, r.h).draw(r.pos, ColorF(1, opacity));
}

void StoryPresentation::Atmosphere() const
{
	for (int i = 0; i < 28; ++i)
	{
		double x = std::fmod(i * 119.3 + m_time * (9 + i % 4), Scene::Width()),
		       y = Scene::Height() - std::fmod(i * 57.1 + m_time * (17 + i % 7), Scene::Height());
		Circle(x, y, 1 + i % 2 * .5).draw(ColorF(1, .61, .24, .08 + (i % 3) * .035));
	}
}

void StoryPresentation::Board(const hero::Story& s, int selected, const RectF& r) const
{
	double z = r.w / hero::W;
	RectF(r.x - 8, r.y - 8, r.w + 16, r.h + 16)
	    .draw(ColorF(.04, .055, .045, .95))
	    .drawFrame(2, ColorF(.57, .44, .23));
	for (int y = 0; y < hero::H; ++y)
		for (int x = 0; x < hero::W; ++x)
		{
			int tile = y * hero::W + x;
			RectF c(r.x + x * z, r.y + y * z, z, z);
			int terrain = s.terrain[tile];
			double variation = ((x * 17 + y * 31) % 7) * .007;
			c.draw(terrain == 2 ? ColorF(.10 + variation, .24, .29)
			                    : ColorF(.24 + variation, .27 + variation, .20));
			c.drawFrame(.6, ColorF(.49, .47, .34, .38));
			if (terrain == 2)
			{
				for (int k = 0; k < 3; ++k)
				{
					double yy = c.y + z * (.2 + k * .27);
					Line(c.x + 7 + std::sin(m_time * 1.3 + y + k) * 3, yy, c.x + z - 7, yy)
					    .draw(1, ColorF(.41, .65, .67, .25));
				}
			}
			if (terrain == 3)
			{
				RectF(c.x + 3, c.y + 4, z - 6, z - 8).draw(ColorF(.45, .35, .22));
				for (int k = 1; k < 6; ++k)
					Line(c.x + 3, c.y + k * z / 6, c.x + z - 3, c.y + k * z / 6)
					    .draw(1, ColorF(.16, .14, .10));
			}
			if (terrain == 1)
				for (int k = 0; k < 3; ++k)
				{
					Vec2 p(c.x + z * (.23 + k * .25), c.y + z * (.35 + (k % 2) * .25));
					Triangle(p + Vec2(0, -z * .18), p + Vec2(-z * .12, z * .08), p + Vec2(z * .12, z * .08))
					    .draw(ColorF(.10, .23, .14));
				}
			if (s.ObjectiveTile() == tile)
			{
				c.draw(ColorF(.94, .69, .22, .22)).drawFrame(3, ColorF(.97, .77, .33));
				FontAsset(U"campaignSmall")(s.objectiveProgress >= 2 ? U"確保" : U"拠点")
				    .drawAt(c.center(), ColorF(.99, .88, .61));
			}
			if (s.chapter == 3 && x == 10 && y == 3)
			{
				c.draw(ColorF(.83, .64, .23, .22)).drawFrame(3, ColorF(.96, .78, .34));
				FontAsset(U"campaignSmall")(U"渡し場").drawAt(c.center(), ColorF(.96, .83, .54));
			}
			if (s.BattleReady() && selected >= 0 && selected < static_cast<int>(s.units.size()))
			{
				const auto& a = s.units[selected];
				int d = std::abs(x - a.x) + std::abs(y - a.y), u = s.At(x, y);
				if (s.deepRules && a.hp > 0 && !a.acted && terrain != 2)
					for (const auto& e : s.units)
						if (e.enemy && e.hp > 0 && !e.stunned &&
						    std::abs(e.x - x) + std::abs(e.y - y) <= e.range &&
						    battle::ClearRay(e.x, e.y, x, y, [&](int xx, int yy) {
							    return s.terrain[yy * hero::W + xx] == 1 || s.terrain[yy * hero::W + xx] == 2;
						    }))
						{
							c.drawFrame(1.8, ColorF(.94, .34, .19, .65));
							break;
						}
				if (a.hp > 0 && !a.acted && terrain != 2)
				{
					if (d == 1 && u < 0 && (!s.deepRules || !a.moved))
						c.draw(ColorF(.48, .75, .56, .18));
					if (u >= 0 && s.units[u].enemy && s.CanAttack(selected, u))
						c.draw(ColorF(.95, .25, .13, .25));
				}
			}
		}
	for (int i = 0; i < static_cast<int>(s.units.size()); ++i)
	{
		const auto& u = s.units[i];
		if (u.hp <= 0)
			continue;
		Vec2 p(r.x + (u.x + .5) * z, r.y + (u.y + .5) * z);
		ColorF color = u.enemy      ? ColorF(.69, .22, .14)
		               : u.civilian ? ColorF(.83, .65, .32)
		                            : ColorF(.26, .52, .35);
		Ellipse(p.movedBy(0, z * .21), z * .36, z * .12).draw(ColorF(0, .4));
		for (int k = -1; k <= 1; ++k)
		{
			Vec2 a = p + Vec2(k * z * .17, z * .07 + std::abs(k) * z * .09);
			Circle(a.x, a.y - z * .06, z * .044).draw(ColorF(.68, .60, .42));
			Line(a.x, a.y, a.x, a.y + z * .10).draw(z * .035, color);
			if (!u.civilian)
				Line(a.x + z * .035, a.y + z * .03, a.x + z * .09, a.y - z * .16)
				    .draw(1.3, ColorF(.71, .70, .57));
		}
		RectF flag(p.x - z * .34, p.y - z * .34, z * .68, z * .27);
		flag.draw(ColorF(color, u.acted ? .52 : 1)).drawFrame(1, ColorF(.92, .80, .49, .70));
		FontAsset(U"campaignSmall")(u.enemy      ? (s.chapter == 0 ? U"黄巾" : U"曹軍")
		                            : u.civilian ? U"避難民"
		                                         : String(hero::Name(u.hero).c_str()))
		    .drawAt(flag.center(), ColorF(.99, .94, .82, u.acted ? .45 : 1));
		if (i == selected && !u.enemy)
			RectF(p.x - z / 2 + 2, p.y - z / 2 + 2, z - 4, z - 4).drawFrame(2.5, ColorF(.97, .84, .40));
		RectF(p.x - z * .34, p.y + z * .34, z * .68, 4).draw(ColorF(.06));
		RectF(p.x - z * .34, p.y + z * .34, z * .68 * u.hp / u.maxHp, 4)
		    .draw(u.hp <= u.maxHp / 3 ? ColorF(.90, .27, .15) : ColorF(.72, .83, .42));
		if (u.guarding)
			FontAsset(U"campaignSmall")(U"守").drawAt(p.movedBy(z * .32, -z * .32), ColorF(.6, .88, .92));
		if (u.stunned > 0)
			FontAsset(U"campaignSmall")(U"足止").drawAt(p.movedBy(0, z * .17), ColorF(.95, .84, .45));
	}
	for (const auto& n : m_numbers)
		if (n.age >= 0)
		{
			Vec2 p(r.x + (n.tile.x + .5) * z, r.y + (n.tile.y + .3) * z - n.age * 40);
			FontAsset(U"storyAccent")(n.amount > 0 ? U"+{}"_fmt(n.amount) : U"{}"_fmt(n.amount))
			    .drawAt(
			        p, ColorF(n.amount > 0 ? ColorF(.56, 1, .67) : ColorF(1, .60, .30), Max(0.0, 1 - n.age)));
			Circle(p, 10 + n.age * 25).drawFrame(1, ColorF(1, .75, .35, Max(0.0, .3 - n.age * .4)));
		}
}

void StoryPresentation::Overlay() const
{
	if (m_moment <= 0)
		return;
	double alpha = Min(1.0, m_moment * 3), y = Scene::Height() * .37;
	RectF panel(0, y, Scene::Width(), 185);
	panel.draw(ColorF(.025, .035, .035, .96 * alpha));
	Line(0, y, Scene::Width(), y).draw(2, ColorF(.87, .68, .29, alpha));
	Line(0, y + 185, Scene::Width(), y + 185).draw(2, ColorF(.87, .68, .29, alpha));
	Portrait(m_hero, RectF(Scene::Width() * .16, y, 185, 185), alpha);
	FontAsset(U"storyDisplay")(m_title).draw(Scene::Width() * .16 + 215, y + 24,
	                                         ColorF(.99, .86, .50, alpha));
	(void)FontAsset(U"campaignBody")(m_quote).draw(
	    RectF(Scene::Width() * .16 + 220, y + 110, Scene::Width() * .66 - 240, 65),
	    ColorF(.96, .93, .80, alpha));
}
