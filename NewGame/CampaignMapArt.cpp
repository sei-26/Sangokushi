#include "CampaignMapArt.hpp"
#include "CampaignUI.hpp"
#include "HexGrid.hpp"

using namespace s3d;
namespace campaignvisual
{
	MapArt::MapArt()
	{
		if (FileSystem::Exists(U"MapArt/terrain-atlas-v1.png"))
			m_sprites = Texture(U"MapArt/terrain-atlas-v1.png", TextureDesc::Mipped);
		if (FileSystem::Exists(U"MapArt/ground-materials-v1.png"))
			m_materials = Texture(U"MapArt/ground-materials-v1.png", TextureDesc::Mipped);
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
		const double h = hexgrid::Radius * r.w * pitch;
		const std::array<Vec2, 6> corners{{{0, -h},
		                                   {r.w * .5, -h * .5},
		                                   {r.w * .5, h * .5},
		                                   {0, h},
		                                   {-r.w * .5, h * .5},
		                                   {-r.w * .5, -h * .5}}};
		const unsigned hash = static_cast<unsigned>(tile) * 2654435761u;
		const double u0 = (material % 2) * .5 + .08 + (hash % 1000) / 1000. * .33;
		const double v0 = (material / 2) * .5 + .08 + ((hash >> 12) % 1000) / 1000. * .33;
		const Float4 tint = terrain == Terrain::Forest     ? Float4(.82f, .88f, .79f, 1.f)
		                    : terrain == Terrain::Mountain ? Float4(.86f, .91f, .85f, 1.f)
		                                                   : Float4(1.f, 1.f, 1.f, 1.f);
		Buffer2D mesh(6, 4);
		for (int k = 0; k < 6; ++k)
			mesh.vertices[k].set(Float2(r.center() + corners[k]),
			                     static_cast<float>(u0 + corners[k].x / r.w * .09),
			                     static_cast<float>(v0 + corners[k].y / (r.w * pitch) * .09), tint);
		for (int k = 0; k < 4; ++k)
			mesh.indices[k] = TriangleIndex{0, static_cast<Vertex2D::IndexType>(k + 1),
			                                static_cast<Vertex2D::IndexType>(k + 2)};
		mesh.draw(m_materials);
		if (!water)
		{
			const float dryness = static_cast<float>(
			    .12 + .09 * std::sin((tile % frontline::Width) * .13 + (tile / frontline::Width) * .19));
			for (auto& vertex : mesh.vertices)
			{
				vertex.tex.x += .5f;
				vertex.color.w = dryness;
			}
			mesh.draw(m_materials);
		}
		return true;
	}

	bool MapArt::Relief(Vec2 p, double w, frontline::Terrain terrain, int tile) const
	{
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
