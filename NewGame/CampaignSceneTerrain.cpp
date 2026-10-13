#include "CampaignScene.hpp"
#include "CampaignUI.hpp"
#include "CampaignVisuals.hpp"
using namespace frontline;
void CampaignScene::drawTerrainRelief(double cell) const
{
	if (!m_game.hexMap)
		return;
	const auto area = mapRect();
	const double hx = area.w / cell / 2, hy = area.h / (cell * m_camera.Pitch()) / 2;
	const int left = Max(0, static_cast<int>(std::floor(m_camera.x - hx)) - 4),
	          right = Min(Width - 1, static_cast<int>(std::ceil(m_camera.x + hx)) + 4),
	          top = Max(0, static_cast<int>(std::floor((m_camera.y - hy) / hexgrid::Row)) - 4),
	          bottom = Min(Height - 1, static_cast<int>(std::ceil((m_camera.y + hy) / hexgrid::Row)) + 4);
	const ScopedRenderStates2D filtering(SamplerState::ClampAniso);
	for (int y = top; y <= bottom; ++y)
		for (int x = left; x <= right; ++x)
		{
			const int tile = Campaign::At(x, y);
			const auto terrain = m_game.tiles[tile].terrain;
			if (terrain != Terrain::Mountain && terrain != Terrain::Forest)
				continue;
			int neighbors = 0;
			for (int n : m_game.MapNeighbors(tile))
				if (m_game.tiles[n].terrain == terrain)
					++neighbors;
			if (terrain == Terrain::Forest)
				for (int n : m_game.MapNeighbors(tile))
				{
					const auto next = m_game.tiles[n].terrain;
					// Fill shared woodland gaps; sparse saplings soften only dry land edges.
					const bool joined = next == Terrain::Forest && n > tile;
					const bool fringe =
					    next == Terrain::Plain && (static_cast<unsigned>(tile * 17 + n * 31) % 5 == 0);
					if (joined || fringe)
						m_mapArt.Relief(tileCenter(tile).lerp(tileCenter(n), joined ? .5 : .36),
						                cell * (joined ? .70 : .32), terrain, tile + n, joined ? 2 : 0);
				}
			if (!m_mapArt.Relief(tileCenter(tile), cell, terrain, tile, neighbors))
				campaignvisual::TerrainRelief(tileCenter(tile), cell, terrain, tile);
		}
	// Borders remain readable above the native-resolution terrain sprites.
	for (int y = top; y <= bottom; ++y)
		for (int x = left; x <= right; ++x)
		{
			const int tile = Campaign::At(x, y), owner = m_game.tiles[tile].owner;
			if (owner < 0 || m_game.tiles[tile].terrain == Terrain::Sea)
				continue;
			const Vec2 p = tileCenter(tile);
			for (int n : m_game.MapNeighbors(tile))
			{
				const int other = m_game.tiles[n].owner;
				if (owner == other || (n < tile && other >= 0) || m_game.tiles[n].terrain == Terrain::Sea)
					continue;
				const Vec2 direction = tileCenter(n) - p, normal = direction.normalized();
				const auto edge = campaignvisual::TileEdge(p, cell, m_camera.Pitch(), direction);
				edge.draw(3.2, ColorF(.015, .025, .03, .94));
				edge.movedBy(-normal * .7).draw(1.3, campaignui::FactionColor(owner));
				if (other >= 0)
					edge.movedBy(normal * .7).draw(1.3, campaignui::FactionColor(other));
			}
		}
}

// Smooth biome colours are cached with the ground, never recomputed while panning.
void CampaignScene::drawTerrainTransitions(double pixels) const
{
	const auto sample = [this](Vec2 point) {
		double weight = 0, forest = 0, mountain = 0;
		const int row = static_cast<int>(std::floor(point.y / hexgrid::Row));
		const int col = static_cast<int>(std::floor(point.x));
		for (int y = Max(0, row - 2); y <= Min(Height - 1, row + 2); ++y)
			for (int x = Max(0, col - 2); x <= Min(Width - 1, col + 2); ++x)
			{
				const auto center = hexgrid::Center(x, y);
				const double distance = (point - Vec2(center.first, center.second)).lengthSq();
				const double influence = Max(0., 1. - distance / 2.25);
				const double w = influence * influence * influence;
				const auto terrain = m_game.tiles[Campaign::At(x, y)].terrain;
				weight += w;
				if (terrain == Terrain::Forest)
					forest += w;
				if (terrain == Terrain::Mountain)
					mountain += w;
			}
		if (weight > 0)
		{
			forest /= weight;
			mountain /= weight;
		}
		const double alpha = forest * .32 + mountain * .28;
		const double mix = mountain / Max(.001, forest + mountain);
		return Float4(static_cast<float>(.12 + mix * .23), static_cast<float>(.25 + mix * .04),
		              static_cast<float>(.10 + mix * .13), static_cast<float>(alpha));
	};
	const std::array<Vec2, 7> offsets{{{0, 0},
	                                   {0, -hexgrid::Radius},
	                                   {.5, -hexgrid::Radius * .5},
	                                   {.5, hexgrid::Radius * .5},
	                                   {0, hexgrid::Radius},
	                                   {-.5, hexgrid::Radius * .5},
	                                   {-.5, -hexgrid::Radius * .5}}};
	Buffer2D mesh(7, 6);
	for (int k = 0; k < 6; ++k)
		mesh.indices[k] = TriangleIndex{0, static_cast<Vertex2D::IndexType>(k + 1),
		                                static_cast<Vertex2D::IndexType>((k + 1) % 6 + 1)};
	for (int tile = 0; tile < TileCount; ++tile)
	{
		const auto terrain = m_game.tiles[tile].terrain;
		if (terrain == Terrain::Sea || terrain == Terrain::River || terrain == Terrain::Bridge)
			continue;
		const auto c = hexgrid::Center(tile % Width, tile / Width);
		for (int k = 0; k < 7; ++k)
		{
			const Vec2 point = Vec2(c.first, c.second) + offsets[k];
			mesh.vertices[k].set(Float2(point.x * pixels, point.y * pixels * m_camera.Pitch()), 0.f, 0.f,
			                     sample(point));
		}
		mesh.draw();
	}
}

void CampaignScene::drawHexGrid(double cell) const
{
	if (!m_game.hexMap || !m_gridVisible)
		return;
	const auto area = mapRect();
	const double hx = area.w / cell / 2, hy = area.h / (cell * m_camera.Pitch()) / 2;
	const int left = Max(0, static_cast<int>(std::floor(m_camera.x - hx)) - 2),
	          right = Min(Width - 1, static_cast<int>(std::ceil(m_camera.x + hx)) + 2),
	          top = Max(0, static_cast<int>(std::floor((m_camera.y - hy) / hexgrid::Row)) - 2),
	          bottom = Min(Height - 1, static_cast<int>(std::ceil((m_camera.y + hy) / hexgrid::Row)) + 2);
	const double alpha = Clamp(cell / 100., .22, .40);
	for (int y = top; y <= bottom; ++y)
		for (int x = left; x <= right; ++x)
		{
			const auto shape =
			    campaignvisual::TileShape(tileCenter(Campaign::At(x, y)), cell, true, m_camera.Pitch());
			shape.drawFrame(1.2, ColorF(.06, .045, .025, alpha * .75));
			shape.drawFrame(.55, ColorF(.91, .81, .57, alpha));
		}
}
