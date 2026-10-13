#include "StoryPresentation.hpp"

void StoryPresentation::TerrainTile(const hero::Story& s, int tile, const RectF& c) const
{
	const int terrain = s.terrain[tile];
	const double z = c.w, grain = ((tile * 17) % 13) * .003;
	const ColorF earth =
	    s.chapter == 5 ? ColorF(.29 + grain, .32, .21) : ColorF(.27 + grain, .30 + grain, .20);
	c.draw(terrain == 2 ? ColorF(.065, .18 + grain, .24) : earth);
	if (terrain == 2)
	{
		for (int k = 0; k < 6; ++k)
		{
			const double y = c.y + z * (.08 + k * .16);
			const double offset = std::sin(m_time * 1.5 + tile + k) * z * .06;
			Line(c.x + z * .14 + offset, y, c.x + z * .72 + offset, y)
			    .draw(k % 2 ? 1.2 : 2, ColorF(.44, .69, .70, .18));
		}
		if (s.chapter == 4 && (tile % hero::W) % 2 == 0)
		{
			const Vec2 p = c.center().movedBy(0, z * .14);
			Ellipse(p.movedBy(0, z * .08), z * .38, z * .12).draw(ColorF(0., .3));
			Quad(p.movedBy(-z * .36, 0), p.movedBy(z * .36, 0), p.movedBy(z * .24, z * .14),
			     p.movedBy(-z * .22, z * .14))
			    .draw(ColorF(.37, .20, .10));
			Line(p, p.movedBy(0, -z * .42)).draw(2, ColorF(.77, .61, .34));
			Triangle(p.movedBy(0, -z * .42), p.movedBy(z * .26, -z * .06), p.movedBy(0, -z * .06))
			    .draw(ColorF(.71, .56, .31));
		}
	}
	else
	{
		// Deterministic detail: no per-frame random allocations or textures.
		for (int k = 0; k < 8; ++k)
		{
			const Vec2 p(c.x + z * (.10 + ((tile * 7 + k * 13) % 79) / 100.),
			             c.y + z * (.12 + ((tile * 11 + k * 19) % 73) / 100.));
			Line(p, p.movedBy(2, -3)).draw(1, ColorF(.57, .57, .31, .24));
		}
		Line(c.x, c.y, c.rightX(), c.y).draw(2, ColorF(.75, .72, .42, .20));
		Line(c.x, c.bottomY() - 1, c.rightX(), c.bottomY() - 1).draw(3, ColorF(.02, .04, .025, .28));
	}
	if (terrain == 3)
	{
		RectF(c.x + z * .12, c.y, z * .76, z).draw(ColorF(.38, .27, .16));
		for (int k = 0; k < 7; ++k)
			Line(c.x + z * .12, c.y + k * z / 7, c.x + z * .88, c.y + k * z / 7)
			    .draw(2, ColorF(.64, .50, .30));
		for (double x : {c.x + z * .08, c.x + z * .91})
		{
			Line(x, c.y + 2, x, c.bottomY() - 2).draw(3, ColorF(.18, .13, .075));
			for (int k = 0; k < 3; ++k)
				Circle(x, c.y + z * (.15 + k * .34), 2).draw(ColorF(.77, .61, .32));
		}
	}
	if (terrain == 1)
		for (int k = 0; k < 4; ++k)
		{
			const Vec2 p = c.pos + Vec2(z * (.24 + (k % 2) * .40), z * (.43 + (k / 2) * .31));
			Ellipse(p.movedBy(z * .04, z * .10), z * .19, z * .07).draw(ColorF(0., .24));
			Line(p, p.movedBy(0, -z * .27)).draw(3, ColorF(.25, .18, .09));
			Triangle(p.movedBy(-z * .18, -z * .08), p.movedBy(0, -z * .44), p.movedBy(z * .18, -z * .08))
			    .draw(ColorF(.12, .29, .17));
			Triangle(p.movedBy(-z * .14, -z * .19), p.movedBy(0, -z * .46), p.movedBy(z * .03, -z * .19))
			    .draw(ColorF(.25, .43, .23));
		}
	c.drawFrame(.6, ColorF(.64, .63, .42, .28));
	if (s.ObjectiveTile() == tile)
	{
		const Vec2 p = c.center().movedBy(z * .27, -z * .08);
		RectF(p.x - z * .24, p.y, z * .43, z * .25).draw(ColorF(.24, .22, .14));
		Triangle(p.movedBy(-z * .30, 0), p.movedBy(-z * .025, -z * .18), p.movedBy(z * .26, 0))
		    .draw(ColorF(.46, .33, .16));
		Line(p.movedBy(0, -z * .14), p.movedBy(0, -z * .42)).draw(2, ColorF(.88, .76, .44));
		RectF(p.x, p.y - z * .42, z * .21, z * .13)
		    .draw(s.objectiveProgress >= 2 ? ColorF(.34, .73, .49) : ColorF(.92, .66, .20));
	}
}
