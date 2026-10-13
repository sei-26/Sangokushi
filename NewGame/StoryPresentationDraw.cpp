#include "StoryPresentation.hpp"
#include "CampaignVisuals.hpp"
#include "PortraitCatalog.hpp"

void StoryPresentation::Landscape(const RectF& r, double opacity) const
{
	const Texture& art = m_chapterArt ? m_chapterArt : m_land;
	if (!art)
		return;
	if (m_chapterArt)
	{
		const int cw = art.width() / 3, ch = art.height() / 2;
		const Rect crop((m_chapter % 3) * cw + 2, (m_chapter / 3) * ch + 2, cw - 4, ch - 4);
		const double scale = Max(r.w / crop.w, r.h / crop.h);
		const auto region = art(crop).scaled(scale);
		region.drawAtClipped(r.center(), r, ColorF(1, opacity));
	}
	else
	{
		const double scale = Max(r.w / art.width(), r.h / art.height());
		art.scaled(scale).drawAtClipped(r.center(), r, ColorF(1, opacity));
	}
}
void StoryPresentation::PortraitName(const String& name, const RectF& r, double opacity) const
{
	const ScopedColorMul2D tint(ColorF(1, opacity));
	if (campaignvisual::PortraitIndex(name.toUTF32()) < 0)
	{
		Landscape(r, .9);
		r.draw(ColorF(.02, .04, .045, .28));
		r.drawFrame(2, ColorF(.73, .61, .36));
		return;
	}
	frontline::General officer;
	officer.name = name.toUTF32();
	campaignvisual::OfficerCard(r, officer, m_faces);
}
void StoryPresentation::Portrait(int who, const RectF& r, double opacity) const
{
	PortraitName(String(hero::Name(Clamp(who, 0, 5)).c_str()), r, opacity);
}

void StoryPresentation::Atmosphere() const
{
	if (m_chapter == 3)
	{
		for (int k = 0; k < 90; ++k)
		{
			const double x = std::fmod(k * 173.7 + m_time * 45, Scene::Width()),
			             y = std::fmod(k * 87.3 + m_time * 420, Scene::Height());
			Line(x, y, x - 6, y + 24).draw(1, ColorF(.70, .86, .94, .12));
		}
	}
	if (m_chapter == 4)
	{
		for (int k = 0; k < 65; ++k)
		{
			const double x = std::fmod(k * 137.1 + m_time * 18, Scene::Width()),
			             y = Scene::Height() -
			                 std::fmod(k * 71.2 + m_time * (38 + k % 5 * 9), Scene::Height());
			Line(x, y, x + 3, y + 7).draw(1.5, ColorF(1., .47 + .08 * (k % 4), .12, .22));
		}
	}
	if (m_chapter == 0)
	{
		for (int k = 0; k < 22; ++k)
		{
			const double x = std::fmod(k * 137.1 + m_time * 24, Scene::Width()),
			             y = std::fmod(k * 71.2 + m_time * 16, Scene::Height());
			Ellipse(x + std::sin(m_time + k) * 12, y, 3, 1.5).draw(ColorF(.98, .65, .66, .26));
		}
	}
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
	const ScopedRenderStates2D clipped(RasterizerState::SolidCullNoneScissor);
	struct RestoreScissor
	{
		Rect previous;
		~RestoreScissor()
		{
			Graphics2D::SetScissorRect(previous);
		}
	} restore{Graphics2D::GetScissorRect()};
	Graphics2D::SetScissorRect(r.asRect());
	for (int y = 0; y < hero::H; ++y)
		for (int x = 0; x < hero::W; ++x)
		{
			int tile = y * hero::W + x;
			RectF c(r.x + x * z, r.y + y * z, z, z);
			int terrain = s.terrain[tile];
			TerrainTile(s, tile, c);
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
		UnitFigure(s, i, selected, r);
	ActionTrails(r);
	for (const auto& n : m_numbers)
		if (n.age >= (n.hasSource && n.ranged ? .32 : 0.))
		{
			const double age = n.age - (n.hasSource && n.ranged ? .32 : 0.);
			Vec2 p(r.x + (n.tile.x + .5) * z, r.y + (n.tile.y + .3) * z - age * 40);
			FontAsset(U"storyAccent")(n.amount > 0 ? U"+{}"_fmt(n.amount) : U"{}"_fmt(n.amount))
			    .drawAt(p,
			            ColorF(n.amount > 0 ? ColorF(.56, 1, .67) : ColorF(1, .60, .30), Max(0.0, 1 - age)));
			if (n.amount < 0)
			{
				const Vec2 impact(r.x + (n.tile.x + .5) * z, r.y + (n.tile.y + .5) * z);
				const double fade = Max(0., 1 - age);
				Circle(impact, 8 + age * z * .8).drawFrame(3 * fade, ColorF(1., .68, .23, fade * .8));
				for (int k = 0; k < 18; ++k)
				{
					const double angle = k * Math::TwoPi / 18;
					const Vec2 dir(std::cos(angle), std::sin(angle));
					const Vec2 spark = impact + dir * (8 + age * 70);
					Line(spark - dir * 12, spark).draw(2, ColorF(1., .84, .46, fade));
				}
			}
			Circle(p, 10 + age * 25).drawFrame(1, ColorF(1, .75, .35, Max(0.0, .3 - age * .4)));
		}
	BattleSpectacle(s, r);
}

void StoryPresentation::Overlay() const
{
	if (m_moment <= 0)
		return;
	const double elapsed = MomentSeconds - m_moment, enter = Clamp((elapsed + .04) / .18, 0., 1.),
	             fade = Clamp((m_moment - .85) / .35, 0., 1.), alpha = enter * fade;
	const double w = Scene::Width(), h = Scene::Height(), top = h * .22, height = Min(350., h * .48),
	             slide = (1 - enter) * w * .18;
	const ColorF accent = m_hero == 4   ? ColorF(1., .47, .16)
	                      : m_hero == 3 ? ColorF(.46, .88, 1.)
	                                    : ColorF(1., .79, .30);
	if (m_celebration)
	{
		const Vec2 origin(w * .5, h * .5);
		for (int i = 0; i < 42; ++i)
		{
			const double angle = i * Math::TwoPi / 42;
			const Vec2 d(std::cos(angle), std::sin(angle));
			Line(origin + d * 80, origin + d * (w * .75)).draw(i % 3 + 1, ColorF(1., .75, .26, alpha * .10));
		}
		for (int i = 0; i < 60; ++i)
		{
			const double t = std::fmod(elapsed * .6 + i * .093, 1.);
			const Vec2 pos(std::fmod(i * 137.7 + elapsed * 30, w), t * h);
			RectF(pos, 3 + i % 3, 6 + i % 4)
			    .rotated(elapsed + i)
			    .draw(ColorF(1., .65 + (i % 3) * .1, .25, alpha * .7));
		}
	}
	Scene::Rect().draw(ColorF(.006, .015, .02, alpha * .55));
	RectF(0, 0, w, 44).draw(ColorF(0., alpha * .9));
	RectF(0, h - 44, w, 44).draw(ColorF(0., alpha * .9));
	RectF(slide, top, w, height).draw(ColorF(.025, .042, .052, alpha * .97));
	for (int i = 0; i < 60; ++i)
	{
		const double yy = top + 12 + (i * 47) % static_cast<int>(height - 24),
		             xx = std::fmod(i * 123 + elapsed * 950, w);
		Line(xx, yy, xx + 140 + i % 5 * 45, yy - 30).draw(1 + i % 3, ColorF(accent, alpha * .22));
	}
	const double left = w * .07 + slide;
	Circle(Vec2(left + height * .42, top + height * .5), height * .6)
	    .draw(ColorF(accent, alpha * .12))
	    .drawFrame(2, ColorF(accent, alpha * .35));
	Portrait(m_hero, RectF(left, top - 20, height * .86, height + 40), alpha);
	RectF(0, top, w, 3).draw(ColorF(accent, alpha));
	RectF(0, top + height, w, 3).draw(ColorF(accent, alpha));
	const double tx = left + height * .86 + 34;
	FontAsset(U"campaignTitle")(String(hero::Name(m_hero).c_str()))
	    .draw(30, tx, top + 24, ColorF(.98, .9, .67, alpha));
	(void)FontAsset(U"storyDisplay")(m_title).draw(RectF(tx, top + 78, w - tx - 54, 120),
	                                               ColorF(accent, alpha));
	(void)FontAsset(U"storyDialogue")(m_quote).draw(RectF(tx, top + 210, w - tx - 54, 90),
	                                                ColorF(.98, .96, .84, alpha));
	FontAsset(U"campaignSmall")(U"Space・Enterで演出を送る")
	    .draw(tx, top + height - 25, ColorF(.77, .85, .81, alpha));
	if (elapsed < .12)
		Scene::Rect().draw(ColorF(1., .86, .59, (1 - elapsed / .12) * .15));
}
