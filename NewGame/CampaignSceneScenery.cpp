#include "CampaignScene.hpp"
#include "CampaignVisuals.hpp"
using namespace frontline;

// These local paths and hamlets are scenery, not movement or income modifiers.
void CampaignScene::drawCityLandscape(double pixels) const
{
	const auto point = [this, pixels](int tile) {
		const auto p = hexgrid::Center(tile % Width, tile / Width);
		return Vec2(p.first * pixels, p.second * pixels * m_camera.Pitch());
	};
	for (const auto& city : m_game.cities)
	{
		for (int y = Max(0, city.tile / Width - 4); y <= Min(Height - 1, city.tile / Width + 4); ++y)
			for (int x = Max(0, city.tile % Width - 4); x <= Min(Width - 1, city.tile % Width + 4); ++x)
			{
				const int tile = Campaign::At(x, y), distance = m_game.MapDistance(tile, city.tile);
				if (distance < 1 || distance > 3 || m_game.tiles[tile].terrain != Terrain::Plain ||
				    m_game.CityAt(tile) >= 0)
					continue;
				const Vec2 center = point(tile);
				if (distance <= 2)
					for (int n : m_game.MapNeighbors(tile))
						if (m_game.MapDistance(n, city.tile) == distance - 1 &&
						    m_game.tiles[n].terrain == Terrain::Plain)
						{
							const Vec2 next = point(n);
							Line(center + Vec2(0, pixels * .12), next + Vec2(0, pixels * .12))
							    .draw(pixels * .075, ColorF(.25, .22, .14, .35));
							Line(center + Vec2(0, pixels * .12), next + Vec2(0, pixels * .12))
							    .draw(pixels * .032, ColorF(.70, .62, .42, .62));
							break;
						}
				const unsigned seed = static_cast<unsigned>(tile) * 2654435761u;
				const int density = 2 + city.farming / 25 + city.level;
				if (seed % 10 >= static_cast<unsigned>(density))
					continue;
				for (int k = 0; k < 2; ++k)
					campaignvisual::Fields(center + Vec2((k ? .22 : -.18) * pixels, -.08 * pixels),
					                       pixels * .72, tile + k);
				if (distance > 2 || seed % 3 != 0)
					continue;
				for (int k = 0; k < 2 + city.level; ++k)
				{
					const Vec2 p = center + Vec2((k % 3 - 1) * pixels * .13, (k / 3) * pixels * .12);
					const double w = pixels * .11, h = pixels * .09;
					RectF(p.x - w * .5, p.y - h, w, h).draw(ColorF(.67, .61, .44, .90));
					RectF(p.x, p.y - h, w * .5, h).draw(ColorF(.40, .37, .28, .85));
					Triangle(p + Vec2(-w * .65, -h), p + Vec2(0, -h * 1.65), p + Vec2(w * .65, -h))
					    .draw(ColorF(.27, .29, .26, .95));
				}
			}
	}
}

void CampaignScene::drawWaterSurface(double cell) const
{
	if (!m_game.hexMap || cell < 12)
		return;
	const RectF area = mapRect();
	const double hx = area.w / cell / 2, hy = area.h / (cell * m_camera.Pitch()) / 2;
	const int left = Max(0, static_cast<int>(std::floor(m_camera.x - hx)) - 2),
	          right = Min(Width - 1, static_cast<int>(std::ceil(m_camera.x + hx)) + 2),
	          top = Max(0, static_cast<int>(std::floor((m_camera.y - hy) / hexgrid::Row)) - 2),
	          bottom = Min(Height - 1, static_cast<int>(std::ceil((m_camera.y + hy) / hexgrid::Row)) + 2);
	const double time = Scene::Time();
	for (int y = top; y <= bottom; ++y)
		for (int x = left; x <= right; ++x)
		{
			const int tile = Campaign::At(x, y);
			const auto terrain = m_game.tiles[tile].terrain;
			if (terrain != Terrain::Sea && terrain != Terrain::River)
				continue;
			const bool sea = terrain == Terrain::Sea;
			const unsigned hash = static_cast<unsigned>(tile) * 2654435761u;
			const Vec2 center = tileCenter(tile);
			Vec2 direction(1, .06);
			if (!sea)
			{
				int first = -1, last = -1;
				for (int n : m_game.MapNeighbors(tile))
					if (m_game.tiles[n].terrain == Terrain::River ||
					    m_game.tiles[n].terrain == Terrain::Bridge)
					{
						if (first < 0 || n < first)
							first = n;
						if (last < 0 || n > last)
							last = n;
					}
				if (first >= 0 && last != first)
					direction = (tileCenter(last) - tileCenter(first)).normalized();
			}
			for (int k = 0; k < (cell >= 25 ? 3 : 1); ++k)
			{
				const double phase = std::fmod(time * (sea ? .10 : .16) + (hash % 1000) * .001 + k * .31, 1.);
				const double envelope = std::sin(phase * Math::Pi);
				const Vec2 p = center + direction * ((phase - .5) * cell * .40) +
				               Vec2((static_cast<int>((hash >> (k * 3)) % 9) - 4) * cell * .028,
				                    (k - 1) * cell * m_camera.Pitch() * .12);
				const Vec2 tangent = sea ? Vec2(1, .07) : direction;
				const double length = cell * (sea ? .12 : .08);
				Line(p - tangent * length, p + tangent * length)
				    .draw(Max(.65, cell * .009), ColorF(.71, .87, .86, envelope * (sea ? .15 : .19)));
			}
		}
}
