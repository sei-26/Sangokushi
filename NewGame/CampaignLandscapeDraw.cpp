#include "CampaignVisuals.hpp"
#include "HexGrid.hpp"
#include "CampaignUI.hpp"

namespace campaignvisual
{
	RectF CityBannerRect(Vec2 p, const String& label)
	{
		const double w = FontAsset(U"campaignBody")(label).region().w + 30;
		if (p.x < 0 || p.x > Scene::Width() || p.y < 80 || p.y > Scene::Height() - 58)
			return RectF(-1000, -1000, 0, 0);
		p.x = Clamp(p.x, w * .5 + 4, Scene::Width() - w * .5 - 4);
		return RectF(Arg::center = p, w, 27);
	}
	void CityBanner(Vec2 p, const String& label, int faction, bool selected)
	{
		const auto glyph = FontAsset(U"campaignBody")(label);
		const RectF plate = CityBannerRect(p, label);
		if (plate.w <= 0)
			return;
		p = plate.center();
		plate.rounded(2).draw(ColorF(.045, .03, .015, .80));
		if (selected)
			plate.drawFrame(1.4, ColorF(.98, .84, .39));
		const Vec2 emblem(plate.x + 12, p.y);
		const auto shield =
		    Polygon{Array<Vec2>{emblem + Vec2(-9, -11), emblem + Vec2(9, -11), emblem + Vec2(9, 6),
		                        emblem + Vec2(0, 12), emblem + Vec2(-9, 6)}};
		const ColorF colour = faction == 0   ? ColorF(.18, .43, .18)
		                      : faction == 1 ? ColorF(.22, .26, .65)
		                      : faction == 2 ? ColorF(.65, .16, .14)
		                                     : ColorF(.4);
		shield.draw(colour).drawFrame(1, ColorF(.89, .86, .70));
		FontAsset(U"campaignSmall")(faction == 0   ? U"蜀"
		                            : faction == 1 ? U"魏"
		                            : faction == 2 ? U"呉"
		                                           : U"〇")
		    .drawAt(emblem, ColorF(.96, .94, .82));
		glyph.drawAt(p + Vec2(12, 1), ColorF(.05, .03, .01));
		glyph.drawAt(p + Vec2(12, 0), ColorF(.99, .86, .30));
	}
	void MapLabel(Vec2 p, const String& label, int faction, bool selected, bool small)
	{
		const auto glyph = FontAsset(small ? U"campaignSmall" : U"campaignBody")(label);
		const auto bounds = glyph.region();
		if (p.x < 0 || p.x > Scene::Width() || p.y < 96 || p.y > Scene::Height() - 132)
			return;
		p.x = Clamp(p.x, (bounds.w + 22) * .5 + 4, Scene::Width() - (bounds.w + 22) * .5 - 4);
		p.y = Max(p.y, 112.);
		const RectF plate(Arg::center = p, bounds.w + 22, small ? 22. : 29.);
		plate.movedBy(1, 2).rounded(3).draw(ColorF(0, 0, 0, .35));
		plate.rounded(3)
		    .draw(ColorF(.025, .045, .04, .95))
		    .drawFrame(selected ? 2 : 1, selected ? ColorF(.99, .85, .48) : ColorF(.52, .59, .53, .9));
		RectF(plate.x + 3, plate.y + 4, 4, plate.h - 8).draw(campaignui::FactionColor(faction));
		glyph.drawAt(p + Vec2(3, 0), ColorF(.98, .97, .88));
	}

	void MapLight(const RectF& area)
	{
		const double rim = Min(70., area.w * .08);
		RectF(area.x, area.y, area.w, Min(145., area.h * .3))
		    .draw(Arg::top = ColorF(.72, .82, .76, .12), Arg::bottom = ColorF(.72, .82, .76, 0));
		RectF(area.x, area.bottomY() - rim, area.w, rim)
		    .draw(Arg::top = ColorF(.02, .06, .07, 0), Arg::bottom = ColorF(.02, .06, .07, .23));
		RectF(area.x, area.y, rim, area.h)
		    .draw(Arg::left = ColorF(.02, .06, .07, .16), Arg::right = ColorF(.02, .06, .07, 0));
		RectF(area.rightX() - rim, area.y, rim, area.h)
		    .draw(Arg::left = ColorF(.02, .06, .07, 0), Arg::right = ColorF(.02, .06, .07, .16));
	}

	void Clash(Vec2 center, double cell, int seed)
	{
		const double pulse = std::fmod(Scene::Time() * 1.8 + seed * .17, 1.);
		const double size = Max(7., cell * .27);
		Ellipse(center, size * (.5 + pulse), size * (.25 + pulse * .5))
		    .drawFrame(1.2, ColorF(1., .77, .33, (1. - pulse) * .55));
		for (int k = 0; k < 4; ++k)
		{
			const double angle = k * Math::HalfPi + seed * .3;
			const Vec2 direction(std::cos(angle), std::sin(angle) * .6);
			Line(center + direction * (pulse * size), center + direction * (pulse * size + 3))
			    .draw(1.2, ColorF(1., .87, .52, (1. - pulse) * .8));
		}
		Line(center.movedBy(-4, -5), center.movedBy(4, 5)).draw(1.5, ColorF(1., .85, .49));
		Line(center.movedBy(4, -5), center.movedBy(-4, 5)).draw(1.5, ColorF(1., .85, .49));
	}
	Line TileEdge(Vec2 center, double width, double pitch, Vec2 direction)
	{
		const double h = hexgrid::Radius * width * pitch;
		const std::array<Vec2, 6> corners{{{0, -h},
		                                   {width * .5, -h * .5},
		                                   {width * .5, h * .5},
		                                   {0, h},
		                                   {-width * .5, h * .5},
		                                   {-width * .5, -h * .5}}};
		int best = 0;
		double score = -1e10;
		for (int i = 0; i < 6; ++i)
		{
			const Vec2 middle = (corners[i] + corners[(i + 1) % 6]) * .5;
			// Undo projection before comparing directions; diagonal banks use the correct edge.
			const double value = middle.x * direction.x + middle.y * direction.y / (pitch * pitch);
			if (value > score)
			{
				score = value;
				best = i;
			}
		}
		return Line(center + corners[best], center + corners[(best + 1) % 6]);
	}

	void Shore(Vec2 center, double width, double pitch, Vec2 direction, bool sea)
	{
		const auto bank = TileEdge(center, width, pitch, direction);
		// Keep the endpoints shared while softening the angular tile shoreline.
		LineString shore;
		const Vec2 normal = direction.normalized();
		const double phase = bank.begin.x * .037 + bank.begin.y * .021;
		for (int k = 0; k <= 12; ++k)
		{
			const double t = k / 12.;
			const double bend = std::sin(t * Math::Pi) * (.035 + .014 * std::sin(phase + t * Math::TwoPi));
			shore << bank.begin.lerp(bank.end, t) - normal * (width * bend);
		}
		shore.draw(width * .26, sea ? ColorF(.66, .65, .48, .12) : ColorF(.24, .36, .23, .16));
		shore.draw(width * .16, sea ? ColorF(.73, .70, .51, .22) : ColorF(.38, .44, .28, .26));
		shore.draw(width * .07, sea ? ColorF(.80, .78, .60, .34) : ColorF(.49, .54, .36, .30));
		shore.movedBy(normal * width * .03)
		    .draw(width * .018, sea ? ColorF(.88, .94, .88, .46) : ColorF(.64, .78, .69, .33));
	}

	void Fields(Vec2 p, double w, int seed)
	{
		const double z = w * .32;
		const Vec2 origin = p + Vec2(-w * .13, w * .17);
		const auto field = Polygon{Array<Vec2>{origin + Vec2(-z, -z * .22), origin + Vec2(z * .25, -z * .52),
		                                       origin + Vec2(z, z * .08), origin + Vec2(-z * .25, z * .38)}};
		field.draw(seed % 2 ? ColorF(.64, .60, .35, .66) : ColorF(.43, .53, .28, .65));
		for (int k = 0; k < 4; ++k)
		{
			const Vec2 a = origin + Vec2(-z * .82 + k * z * .32, -z * .18 - k * z * .05);
			Line(a, a + Vec2(z * .75, z * .43)).draw(.6, ColorF(.81, .76, .49, .55));
		}
	}
} // namespace campaignvisual
