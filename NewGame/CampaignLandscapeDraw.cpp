#include "CampaignVisuals.hpp"
#include "HexGrid.hpp"
#include "CampaignUI.hpp"

namespace campaignvisual
{
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
		bank.draw(width * .095, sea ? ColorF(.66, .63, .40, .8) : ColorF(.40, .48, .29, .9));
		bank.draw(width * .035, sea ? ColorF(.80, .84, .68, .66) : ColorF(.47, .70, .64, .6));
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
