#include "CampaignMapArt.hpp"
#include "CampaignUI.hpp"
#include "HexGrid.hpp"

using namespace s3d;
namespace campaignvisual
{
	MapArt::MapArt()
	{
		if (FileSystem::Exists(U"MapArt/terrain-relief-v2.png"))
			m_relief = Texture(U"MapArt/terrain-relief-v2.png", TextureDesc::Mipped);
		if (FileSystem::Exists(U"MapArt/terrain-atlas-v1.png"))
			m_sprites = Texture(U"MapArt/terrain-atlas-v1.png", TextureDesc::Mipped);
		if (FileSystem::Exists(U"MapArt/ground-materials-v1.png"))
		{
			m_materials = Texture(U"MapArt/ground-materials-v1.png", TextureDesc::Mipped);
			const Image source(U"MapArt/ground-materials-v1.png");
			for (int i = 0; i < 4; ++i)
				m_groundTiles[i] =
				    Texture(source.clipped(Rect((i % 2) * source.width() / 2, (i / 2) * source.height() / 2,
				                                source.width() / 2, source.height() / 2)),
				            TextureDesc::Mipped);
		}
	}

	void MapArt::Sprite(int index, Vec2 p, double size, double baseline) const
	{
		const int x0 = index % 4 * m_sprites.width() / 4, x1 = (index % 4 + 1) * m_sprites.width() / 4;
		const int y0 = index / 4 * m_sprites.height() / 2, y1 = (index / 4 + 1) * m_sprites.height() / 2;
		m_sprites(Rect(x0, y0, x1 - x0, y1 - y0))
		    .resized(size, size)
		    .draw(p.x - size * .5, p.y - size * baseline);
	}

	bool MapArt::Ground(const RectF& r, frontline::Terrain terrain, int tile, double pitch) const
	{
		if (!m_materials)
			return false;
		using frontline::Terrain;
		const bool water = terrain == Terrain::Sea || terrain == Terrain::River || terrain == Terrain::Bridge;
		const int material = water ? (terrain == Terrain::Sea ? 3 : 2) : 0;
		const ScopedRenderStates2D filtering(SamplerState::RepeatAniso);
		const double h = hexgrid::Radius * r.w * pitch;
		const std::array<Vec2, 6> corners{{{0, -h},
		                                   {r.w * .5, -h * .5},
		                                   {r.w * .5, h * .5},
		                                   {0, h},
		                                   {-r.w * .5, h * .5},
		                                   {-r.w * .5, -h * .5}}};
		Buffer2D mesh(6, 4);
		for (int k = 0; k < 6; ++k)
		{
			const Vec2 p = r.center() + corners[k];
			const double x = p.x / r.w, y = p.y / (r.w * pitch);
			const float light = static_cast<float>(.94 + .035 * std::sin(x * .17 + y * .11) +
			                                       .025 * std::cos(x * .09 - y * .23));
			mesh.vertices[k].set(Float2(p), static_cast<float>(x / 8.), static_cast<float>(y / 8.),
			                     Float4(light * (terrain == Terrain::Mountain ? .88f : 1.f),
			                            light * (terrain == Terrain::Mountain ? .91f : 1.f),
			                            light * (terrain == Terrain::Mountain ? .86f : 1.f), 1.f));
		}
		for (int k = 0; k < 4; ++k)
			mesh.indices[k] = TriangleIndex{0, static_cast<Vertex2D::IndexType>(k + 1),
			                                static_cast<Vertex2D::IndexType>(k + 2)};
		mesh.draw(m_groundTiles[material]);
		if (!water)
		{
			for (auto& vertex : mesh.vertices)
			{
				const double x = vertex.pos.x / r.w, y = vertex.pos.y / (r.w * pitch);
				vertex.color.w = static_cast<float>(.12 + .16 * (1 - Clamp(y / 55., 0., 1.)) +
				                                    .05 * std::sin(x * .13 + y * .09));
			}
			mesh.draw(m_groundTiles[1]);
		}
		return true;
	}

	bool MapArt::Relief(Vec2 p, double w, frontline::Terrain terrain, int tile, int neighbors) const
	{
		if (m_relief && (terrain == frontline::Terrain::Mountain || terrain == frontline::Terrain::Forest))
		{
			const unsigned hash = static_cast<unsigned>(tile) * 2654435761u;
			const bool mountain = terrain == frontline::Terrain::Mountain;
			const bool dominantRidge = mountain && neighbors >= 4 && hash % 3 == 0;
			// Neighbouring ridge tiles share a geological shape; isolated peaks retain variety.
			const int ridge = (tile % frontline::Width / 3 + tile / frontline::Width / 3 * 7) % 4;
			const int index =
			    mountain ? (neighbors >= 2 ? (ridge + (hash % 3 == 0 ? 1 : 0)) % 4 : (hash >> 17) % 4)
			             : 4 + ((hash >> 17) % 4);
			const int x0 = (index % 4) * m_relief.width() / 4, x1 = (index % 4 + 1) * m_relief.width() / 4;
			const int y0 = (index / 4) * m_relief.height() / 2, y1 = (index / 4 + 1) * m_relief.height() / 2;
			const double variation = (hash % 1000) / 1000.;
			const double size =
			    w * (mountain ? (dominantRidge ? 1.95 : 1.24 + Min(neighbors, 4) * .075 + variation * .30)
			                  : .94 + Min(neighbors, 4) * .08 + variation * .28);
			const Vec2 offset(w * (mountain ? .05 * std::sin(tile * .7) : (variation - .5) * .32), w * .15);
			const auto sprite = m_relief(Rect(x0, y0, x1 - x0, y1 - y0)).scaled(size / (x1 - x0));
			sprite.draw(p.x + offset.x - sprite.size.x * .5, p.y + offset.y - sprite.size.y * .79);

			return true;
		}
		if (!m_sprites)
			return false;
		const unsigned hash = static_cast<unsigned>(tile) * 2654435761u;
		if (terrain == frontline::Terrain::Mountain)
			Sprite((hash >> 15) % 4, p + Vec2(0, w * .14), w * (1.34 + (hash % 5) * .025), .86);
		else if (terrain == frontline::Terrain::Forest)
			Sprite(4 + (hash >> 15) % 2, p + Vec2(0, w * .13), w * 1.14, .86);
		else
			return false;
		return true;
	}

	bool MapArt::City(Vec2 p, double cell, int faction, bool selected) const
	{
		if (!m_sprites)
			return false;
		const double size = Max(38., cell * 1.72);
		if (selected)
			Ellipse(p + Vec2(0, cell * .10), size * .48, size * .19).drawFrame(2, ColorF(.99, .85, .46));
		Sprite(6, p + Vec2(0, cell * .25), size, .86);
		Line(p + Vec2(cell * .26, -cell * .40), p + Vec2(cell * .26, -cell * .94))
		    .draw(1.3, ColorF(.82, .76, .54));
		Triangle(p + Vec2(cell * .26, -cell * .94), p + Vec2(cell * .63, -cell * .84),
		         p + Vec2(cell * .26, -cell * .68))
		    .draw(campaignui::FactionColor(faction));
		return true;
	}

	bool MapArt::Town(Vec2 p, double cell, int faction) const
	{
		if (!m_sprites)
			return false;
		Sprite(7, p + Vec2(0, cell * .15), Max(16., cell * .94), .86);
		Circle(p + Vec2(cell * .23, -cell * .28), Max(2., cell * .075))
		    .draw(campaignui::FactionColor(faction))
		    .drawFrame(.7, ColorF(.97, .90, .67));
		return true;
	}
} // namespace campaignvisual
