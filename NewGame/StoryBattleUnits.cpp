#include "StoryPresentation.hpp"

void StoryPresentation::UnitFigure(const hero::Story& s, int index, int selected, const RectF& r) const
{
	const auto& u = s.units[index];
	if (u.hp <= 0)
	{
		bool pending = false;
		for (const auto& n : m_numbers)
			if (n.defeated && n.tile == Vec2(u.x, u.y) && n.age < (n.hasSource && n.ranged ? .32 : .12))
				pending = true;
		if (!pending)
			return;
	}
	const double z = r.w / hero::W;
	Vec2 tile(u.x, u.y);
	for (const auto& motion : m_motion)
		if (motion.index == index)
		{
			const double t = Clamp(motion.age / .38, 0., 1.);
			tile = motion.from.lerp(motion.to, t * t * (3 - 2 * t));
		}
	Vec2 p = r.pos + (tile + Vec2(.5, .5)) * z;
	double hurt = 0;
	for (const auto& n : m_numbers)
		if (n.amount < 0 && n.tile == Vec2(u.x, u.y) && n.age >= (n.hasSource && n.ranged ? .32 : 0.) &&
		    n.age < .4)
			hurt = Max(hurt, 1 - n.age / .4);
	p.x += std::sin(m_time * 70) * hurt * 3;
	const ColorF color = u.enemy      ? ColorF(.74, .25, .15)
	                     : u.civilian ? ColorF(.82, .64, .33)
	                                  : ColorF(.27, .60, .43);
	Ellipse(p.movedBy(0, z * .21), z * .43, z * .15).draw(ColorF(0, .45));
	Ellipse(p.movedBy(0, z * .22), z * .43, z * .15)
	    .drawFrame(index == selected ? 2.5 : 1, ColorF(index == selected ? ColorF(1., .84, .37) : color, .9));
	for (int row = 0; row < 2; ++row)
		for (int k = -2; k <= 2; ++k)
		{
			const double sway = std::sin(m_time * 2 + k + row) * z * .012;
			const Vec2 a = p + Vec2(k * z * .14, z * (.06 + row * .16) + sway);
			Ellipse(a.movedBy(0, z * .09), z * .05, z * .02).draw(ColorF(0, .35));
			RectF(a.x - z * .036, a.y, z * .072, z * .11).draw(color);
			Circle(a.movedBy(0, -z * .025), z * .042).draw(ColorF(.71, .62, .46));
			if (!u.civilian)
			{
				Circle(a.movedBy(0, -z * .05), z * .042).draw(ColorF(.24, .27, .26));
				Line(a.movedBy(z * .06, z * .06), a.movedBy(z * .11, -z * .18))
				    .draw(1.2, ColorF(.85, .81, .62));
			}
		}
	const Vec2 pole = p.movedBy(-z * .34, -z * .09);
	Line(pole, pole.movedBy(0, -z * .38)).draw(2, ColorF(.73, .61, .36));
	const double wave = std::sin(m_time * 3 + index) * z * .02;
	Quad(pole.movedBy(0, -z * .38), pole.movedBy(z * .19, -z * .34 + wave),
	     pole.movedBy(z * .19, -z * .16 + wave), pole.movedBy(0, -z * .20))
	    .draw(color);
	if (u.hero >= 0)
		Portrait(u.hero, RectF(p.x - z * .17, p.y - z * .20, z * .34, z * .40), u.acted ? .58 : 1);
	if (hurt > 0)
		Circle(p, z * .27).draw(ColorF(1., .61, .32, hurt * .22));
	const RectF flag(p.x - z * .43, p.y - z * .49, z * .86, z * .27);
	flag.draw(ColorF(.04, .075, .065, .94)).drawFrame(1, ColorF(color, .9));
	const String name = u.enemy      ? (u.range > 1      ? U"弓兵"
	                                    : s.chapter == 0 ? U"黄巾"
	                                                     : U"曹軍")
	                    : u.civilian ? U"避難民"
	                                 : String(hero::Name(u.hero).c_str());
	FontAsset(U"campaignSmall")(name).drawAt(flag.center(), ColorF(.99, .94, .82, u.acted ? .55 : 1));
	RectF(p.x - z * .38, p.y + z * .39, z * .76, 5).draw(ColorF(.035));
	RectF(p.x - z * .38, p.y + z * .39, z * .76 * u.hp / u.maxHp, 5)
	    .draw(u.hp <= u.maxHp / 3 ? ColorF(.95, .30, .15)
	          : u.enemy           ? ColorF(.85, .53, .30)
	                              : ColorF(.52, .88, .57));
	if (u.guarding)
		Circle(p.movedBy(z * .30, 0), z * .13)
		    .draw(ColorF(.09, .27, .32, .85))
		    .drawFrame(2, ColorF(.58, .90, 1.));
	if (u.stunned > 0)
		FontAsset(U"campaignSmall")(U"足止").drawAt(p.movedBy(0, z * .16), ColorF(1., .87, .38));
	if (index == selected && !u.enemy)
		RectF(r.x + u.x * z + 1, r.y + u.y * z + 1, z - 2, z - 2)
		    .drawFrame(2, ColorF(.97, .84, .40, .7 + std::sin(m_time * 3) * .2));
}

void StoryPresentation::ActionTrails(const RectF& r) const
{
	const double z = r.w / hero::W;
	for (const auto& n : m_numbers)
	{
		if (n.age < 0 || n.age > .65 || !n.hasSource)
			continue;
		const Vec2 from = r.pos + (n.source + Vec2(.5, .5)) * z;
		const Vec2 to = r.pos + (n.tile + Vec2(.5, .5)) * z;
		const double t = Clamp(n.age / .32, 0., 1.), fade = Clamp((.65 - n.age) / .25, 0., 1.);
		const ColorF color = n.amount > 0 ? ColorF(.39, 1., .67) : ColorF(1., .79, .36);
		if (n.ranged && n.amount < 0)
		{
			const Vec2 head = from.lerp(to, t).movedBy(0, -std::sin(t * Math::Pi) * z * .55);
			Line(from.lerp(head, .55), head).draw(2, ColorF(color, fade));
			Circle(head, 3).draw(ColorF(1., .94, .70, fade));
		}
		else
		{
			Line(from, from.lerp(to, t)).draw(8 * (1 - t) + 2, ColorF(color, fade * .5));
			Line(to.movedBy(-z * .26, z * .25), to.movedBy(z * .26, -z * .25)).draw(3, ColorF(color, fade));
		}
		if (n.defeated && n.age > .25)
			FontAsset(U"campaignBody")(U"撃破！").drawAt(to.movedBy(0, -z * .45 - n.age * 18),
			                                             ColorF(1., .83, .34, fade));
	}
}
