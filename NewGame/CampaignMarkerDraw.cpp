#include "CampaignVisuals.hpp"
#include "CampaignUI.hpp"

namespace campaignvisual
{
	void CityIcon(Vec2 p, double cell, int faction, bool selected)
	{
		const double z = Max(16., cell * .85);
		Ellipse(p.movedBy(z * .12, z * .18), z * .72, z * .25).draw(ColorF(.025, .04, .025, .38));
		if (selected)
			Ellipse(p, z * .78, z * .36).drawFrame(2, ColorF(.94, .79, .40));
		// Projected fortress: courtyard, sunlit front walls and shaded side walls.
		const auto face = [&](std::initializer_list<Vec2> points, ColorF color) {
			Polygon{Array<Vec2>(points)}.draw(color);
		};
		const Vec2 a = p + Vec2(-z * .65, -z * .10), b = p + Vec2(z * .22, -z * .32),
		           c = p + Vec2(z * .65, -z * .08), d = p + Vec2(-z * .22, z * .16);
		face({a, b, c, d}, ColorF(.69, .68, .49));
		face({a, d, d + Vec2(0, z * .27), a + Vec2(0, z * .27)}, ColorF(.71, .68, .50));
		face({d, c, c + Vec2(0, z * .27), d + Vec2(0, z * .27)}, ColorF(.35, .42, .35));
		for (const Vec2 corner : {a, b, c, d})
		{
			const Vec2 top = corner + Vec2(0, -z * .32);
			RectF(top.x - z * .09, top.y, z * .18, z * .35).draw(ColorF(.73, .71, .53));
			face({top + Vec2(-z * .15, 0), top + Vec2(0, -z * .13), top + Vec2(z * .15, 0),
			      top + Vec2(0, z * .06)},
			     ColorF(.16, .28, .25));
			Line(top + Vec2(0, z * .05), top + Vec2(z * .09, z * .02)).draw(1, ColorF(.63, .71, .49));
		}
		RectF(p.x - z * .22, p.y - z * .38, z * .40, z * .27).draw(ColorF(.79, .73, .50));
		face({p + Vec2(-z * .33, -z * .38), p + Vec2(-z * .06, -z * .59), p + Vec2(z * .30, -z * .39),
		      p + Vec2(0, -z * .31)},
		     ColorF(.20, .33, .29));
		for (int k = 1; k <= 5; ++k)
		{
			const double t = k / 6.;
			const Vec2 front = a + (d - a) * t;
			RectF(front.x - z * .025, front.y + z * .09, z * .05, z * .10).draw(ColorF(.29, .35, .29));
			RectF(front.x - z * .04, front.y - z * .055, z * .08, z * .08).draw(ColorF(.81, .77, .56));
			const Vec2 side = d + (c - d) * t;
			RectF(side.x - z * .025, side.y + z * .09, z * .05, z * .09).draw(ColorF(.16, .24, .23));
			RectF(side.x - z * .035, side.y - z * .04, z * .07, z * .06).draw(ColorF(.52, .56, .40));
		}
		Line(p + Vec2(-z * .33, -z * .38), p + Vec2(0, -z * .31)).draw(.9, ColorF(.73, .66, .39));
		RectF(d.x - z * .09, d.y + z * .06, z * .18, z * .21).draw(ColorF(.09, .16, .14));
		Line(p + Vec2(z * .15, -z * .62), p + Vec2(z * .15, -z * .94)).draw(1.3, ColorF(.74, .67, .45));
		Triangle(p + Vec2(z * .15, -z * .94), p + Vec2(z * .44, -z * .85), p + Vec2(z * .15, -z * .76))
		    .draw(campaignui::FactionColor(faction));
	}
	void ArmyIcon(Vec2 p, double cell, const frontline::Army& a, bool selected,
	              const frontline::General& general, const Texture& faces)
	{
		const double z = Max(16., cell * .88);
		if (selected)
			Ellipse(p.movedBy(0, z * .3), z * .74, z * .32)
			    .draw(ColorF(.96, .79, .32, .15))
			    .drawFrame(2, ColorF(.99, .84, .44));
		Ellipse(p.movedBy(z * .10, z * .43), z * .65, z * .24).draw(ColorF(.025, .04, .03, .35));
		if (cell > 20)
			for (int row = 0; row < 2; ++row)
				for (int column = 0; column < 4; ++column)
				{
					const Vec2 soldier = p + Vec2((column - 1.5) * z * .22, z * (.42 + row * .15));
					Line(soldier, soldier + Vec2(0, z * .12))
					    .draw(z * .065, campaignui::FactionColor(a.faction));
					Circle(soldier, z * .035).draw(ColorF(.79, .76, .57));
					Line(soldier + Vec2(z * .07, -z * .10), soldier + Vec2(z * .07, z * .10))
					    .draw(.7, ColorF(.71, .72, .62));
				}
		Line(p.movedBy(-z * .35, -z * .65), p.movedBy(-z * .35, z * .52)).draw(2.5, ColorF(.15, .12, .07));
		RectF(p.x - z * .30 + 2, p.y - z * .43 + 2, z * .76, z * .75).draw(ColorF(.025, .035, .025, .5));
		const double flutter = std::sin(Scene::Time() * 2.2 + a.general * .8) * z * .035;
		const auto flag =
		    Polygon{Array<Vec2>{p + Vec2(-z * .30, -z * .43), p + Vec2(z * .46, -z * .43 + flutter),
		                        p + Vec2(z * .37, -z * .05 + flutter), p + Vec2(z * .46, z * .32 + flutter),
		                        p + Vec2(-z * .30, z * .32)}};
		flag.draw(campaignui::FactionColor(a.faction)).drawFrame(1, ColorF(.92, .81, .56));
		Line(p + Vec2(-z * .26, -z * .37), p + Vec2(z * .35, -z * .37 + flutter))
		    .draw(.8, ColorF(1, .92, .61, .65));
		const String icon = a.arm == frontline::Arm::Transport ? U"糧"
		                    : a.arm == frontline::Arm::Siege   ? U"城"
		                    : a.arm == frontline::Arm::Bow     ? U"弓"
		                    : a.arm == frontline::Arm::Cavalry ? U"騎"
		                                                       : U"槍";
		OfficerCard(RectF(p.x - z * .30, p.y - z * .43, z * .76, z * .75), general, faces);
		const Vec2 badge = p.movedBy(z * .43, z * .28);
		Circle(badge, Max(8., z * .16))
		    .draw(campaignui::FactionColor(a.faction))
		    .drawFrame(1, ColorF(.96, .84, .53));
		FontAsset(U"campaignSmall")(icon).drawAt(badge, ColorF(.99, .96, .83));
		RectF(p.x - z * .30, p.y + z * .36, z * .76, 3).draw(ColorF(.04, .06, .045));
		RectF(p.x - z * .30, p.y + z * .36, z * .76 * Min(1., a.troops / 6000.), 3)
		    .draw(a.morale < 40 ? ColorF(.93, .47, .28) : ColorF(.86, .77, .46));
	}
} // namespace campaignvisual
