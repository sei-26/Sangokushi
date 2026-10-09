#include "CampaignVisuals.hpp"
#include "HexGrid.hpp"

namespace campaignvisual
{
	void TerrainRelief(Vec2 p, double w, frontline::Terrain terrain, int seed)
	{
		using frontline::Terrain;
		if (terrain == Terrain::Mountain)
		{
			unsigned hash = static_cast<unsigned>(seed) * 747796405u + 2891336453u;
			hash = ((hash >> ((hash >> 28u) + 4u)) ^ hash) * 277803737u;
			hash = (hash >> 22u) ^ hash;
			const double noise = (hash % 1000) / 1000.;
			const double height = w * (.44 + noise * .48);
			const Vec2 left = p + Vec2(-w * .71, w * .12), front = p + Vec2(w * .02, w * .26),
			           right = p + Vec2(w * .70, w * .08);
			const Vec2 peak = p + Vec2(w * (-.23 + noise * .32), -height);
			const Vec2 shoulder = p + Vec2(-w * .49, -height * .41), ridge = p + Vec2(w * .37, -height * .62);
			Ellipse(p + Vec2(w * .24, w * .17), w * .70, w * .22).draw(ColorF(.025, .065, .075, .26));
			Polygon{Array<Vec2>{left, shoulder, peak, ridge, right, front}}.draw(ColorF(.27, .37, .36));
			Triangle(left, shoulder, front).draw(ColorF(.42, .51, .43));
			Triangle(shoulder, peak, front).draw(ColorF(.56 + noise * .06, .62 + noise * .03, .49));
			Triangle(peak, ridge, front).draw(ColorF(.31, .44, .40));
			Triangle(ridge, right, front).draw(ColorF(.23, .34, .34));
			const Vec2 spine = peak + (front - peak) * .48;
			Triangle(peak, spine, shoulder).draw(ColorF(.65, .68, .52, .35));
			Line(peak, spine).draw(.8, ColorF(.78, .78, .59, .55));
			if (noise > .75)
				Triangle(peak, peak + Vec2(-w * .09, w * .13), peak + Vec2(w * .10, w * .16))
				    .draw(ColorF(.82, .85, .74, .72));
		}
		else if (terrain == Terrain::Forest)
		{
			for (int k = 0; k < 3 + seed % 2; ++k)
			{
				const Vec2 tree = p + Vec2((k % 3 - 1) * w * .25 + (seed % 5 - 2) * w * .035,
				                           (k / 3) * w * .17 + (k % 2) * w * .10);
				Ellipse(tree + Vec2(w * .10, w * .06), w * .19, w * .08).draw(ColorF(.03, .09, .05, .3));
				Line(tree, tree + Vec2(0, -w * .30)).draw(w * .04, ColorF(.31, .26, .15));
				Ellipse(tree + Vec2(0, -w * .28), w * (.15 + seed % 3 * .015), w * .22)
				    .draw(ColorF(.12, .26, .18));
				Ellipse(tree + Vec2(-w * .045, -w * .33), w * .12, w * .15)
				    .draw(ColorF(.26 + (seed % 5) * .012, .43 + (seed % 3) * .015, .29));
			}
		}
		else if (terrain == Terrain::Bridge)
		{
			RectF(p.x - w * .48, p.y - w * .05, w * .96, w * .19).draw(ColorF(.24, .21, .15));
			for (int k = 0; k < 7; ++k)
				RectF(p.x - w * .45 + k * w * .13, p.y - w * .10, w * .11, w * .15)
				    .draw(ColorF(.65, .56, .38));
			Line(p + Vec2(-w * .5, -w * .13), p + Vec2(w * .5, -w * .13)).draw(1.5, ColorF(.81, .71, .46));
		}
	}
	Polygon TileShape(Vec2 p, double w, bool hex, double pitch)
	{
		if (!hex)
			return RectF(Arg::center(p), w, w).asPolygon();
		return Polygon{Array<Vec2>{
		    p + Vec2(0, -hexgrid::Radius * w * pitch), p + Vec2(w * .5, -hexgrid::Radius * w * pitch / 2),
		    p + Vec2(w * .5, hexgrid::Radius * w * pitch / 2), p + Vec2(0, hexgrid::Radius * w * pitch),
		    p + Vec2(-w * .5, hexgrid::Radius * w * pitch / 2),
		    p + Vec2(-w * .5, -hexgrid::Radius * w * pitch / 2)}};
	}
	void TerrainTile(const RectF& r, frontline::Terrain terrain, int seed, bool hex, double pitch)
	{
		using frontline::Terrain;
		const double grain = ((seed * 17 + seed / frontline::Width * 23) % 11) * .0025;
		const bool water = terrain == Terrain::Sea || terrain == Terrain::River || terrain == Terrain::Bridge;
		if (!hex)
			r.draw(water                          ? ColorF(.105 + grain, .245 + grain, .27 + grain)
			       : terrain == Terrain::Forest   ? ColorF(.40 + grain, .415 + grain, .285)
			       : terrain == Terrain::Mountain ? ColorF(.40 + grain, .415 + grain, .285)
			                                      : ColorF(.40 + grain, .415 + grain, .285));
		if (hex)
		{
			const auto shape = TileShape(r.center(), r.w, true, pitch);
			const double climate =
			    .5 + .5 * std::sin((seed % frontline::Width) * .13 + (seed / frontline::Width) * .19);
			shape.draw(
			    water                        ? ColorF(.075 + grain, .24 + grain, .32 + grain)
			    : terrain == Terrain::Forest ? ColorF(.29 + grain, .39 + grain, .25)
			    : terrain == Terrain::Mountain
			        ? ColorF(.35 + grain, .43 + grain, .35)
			        : ColorF(.37 + climate * .10 + grain, .49 + climate * .08 + grain, .32 + climate * .04));
			if (water)
				for (int k = 0; k < 2; ++k)
					Line(r.center() + Vec2(-9 + seed % 5, (k * 7 - 3) * pitch),
					     r.center() + Vec2(8 + seed % 5, (k * 7 - 3) * pitch))
					    .draw(.7, ColorF(.54, .78, .80, .18));
			return;
		}
		if (water)
		{
			for (int k = 0; k < 3; ++k)
			{
				const double offset = (seed + k * 7) % 8;
				Line(r.x + 3 + offset, r.y + 7 + k * 9, r.x + 21 + offset, r.y + 7 + k * 9)
				    .draw(1, ColorF(.55, .71, .67, .16));
			}
		}
		else if (terrain == Terrain::Mountain)
		{
			for (int k = 0; k < 2; ++k)
			{
				const Vec2 p = r.pos + Vec2(10 + k * 11, 22 + k * 5);
				Triangle(p.movedBy(-10, 0), p.movedBy(0, -20 + k * 4), p.movedBy(10, 0))
				    .draw(ColorF(.25, .28, .235));
				Triangle(p.movedBy(-10, 0), p.movedBy(0, -20 + k * 4), p.movedBy(-2, -3))
				    .draw(ColorF(.63, .60, .43, .75));
				Line(p.movedBy(0, -20 + k * 4), p.movedBy(5, -5)).draw(.8, ColorF(.77, .72, .52, .6));
			}
		}
		else if (terrain == Terrain::Forest)
		{
			for (int k = 0; k < 3 + seed % 2; ++k)
			{
				const Vec2 p = r.pos + Vec2(7 + k % 2 * 16 + (seed % 5 - 2), 13 + k / 2 * 14 - (seed % 3));
				Circle(p.movedBy(1, 3), 5).draw(ColorF(.07, .12, .09, .3));
				Line(p.movedBy(0, -1), p.movedBy(0, 5)).draw(1.5, ColorF(.35, .29, .17));
				Triangle(p.movedBy(0, -9), p.movedBy(-6, 2), p.movedBy(6, 2)).draw(ColorF(.12, .23, .15));
				Triangle(p.movedBy(0, -9), p.movedBy(-6, 2), p.movedBy(-1, 0)).draw(ColorF(.31, .40, .23));
			}
		}
		else
		{
			for (int k = 0; k < 2; ++k)
				Line(r.x + 4 + (seed + k * 9) % 18, r.y + 8 + k * 13, r.x + 10 + (seed + k * 9) % 18,
				     r.y + 7 + k * 13)
				    .draw(.7, ColorF(.67, .64, .39, .18));
		}
		if (terrain == Terrain::Bridge)
		{
			RectF(r.x + 2, r.y + 11, 28, 10).draw(ColorF(.34, .27, .17));
			for (int k = 0; k < 7; ++k)
				RectF(r.x + 3 + k * 4, r.y + 12, 3, 8).draw(ColorF(.67, .56, .35));
			Line(r.x + 1, r.y + 10, r.x + 31, r.y + 10).draw(1.5, ColorF(.77, .66, .40));
		}
	}
} // namespace campaignvisual
