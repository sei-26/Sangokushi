#include "CampaignScene.hpp"
#include "CampaignSave.hpp"
#include "CampaignUI.hpp"
#include "CampaignVisuals.hpp"

using namespace frontline;
using namespace campaignui;

void CampaignScene::drawMap() const
{
	if (m_started && m_atlasView && m_game.hexMap)
	{
		drawAtlas();
		return;
	}
	constexpr double TerrainPixels = 48.;
	const RectF area = mapRect();
	const double cell = m_camera.Cell(area.w, area.h);
	if (!m_mapTexture || m_mapRevision != m_game.revision || m_mapSupply != m_supplyView ||
	    m_mapTerritory != m_territoryView || (m_supplyView && m_mapDay != m_game.day))
	{
		if (!m_mapTexture)
			m_mapTexture = RenderTexture(static_cast<int>(std::ceil((Width + .5) * TerrainPixels)),
			                             static_cast<int>(Height * TerrainPixels), ColorF(0.08, 0.12, 0.13));
		const auto supply = m_game.Supply(m_game.player);
		const auto blockade = m_game.SupplyBlockade(m_game.player);
		{
			const ScopedRenderTarget2D target(m_mapTexture);
			m_mapTexture.clear(ColorF(.055, .1, .13));
			for (int i = 0; i < TileCount; ++i)
			{
				const auto& tile = m_game.tiles[i];
				const auto p = m_game.hexMap ? hexgrid::Center(i % Width, i / Width)
				                             : std::pair<double, double>{i % Width + .5, i / Width + .5};
				const RectF rect(
				    Arg::center(Vec2(p.first * TerrainPixels, p.second * TerrainPixels * m_camera.Pitch())),
				    TerrainPixels, TerrainPixels);
				const auto shape =
				    campaignvisual::TileShape(rect.center(), TerrainPixels, m_game.hexMap, m_camera.Pitch());
				if (!m_game.hexMap || !m_mapArt.Ground(rect, tile.terrain, i, m_camera.Pitch()))
					campaignvisual::TerrainTile(rect, tile.terrain, i, m_game.hexMap, m_camera.Pitch());
				const bool water = tile.terrain == Terrain::Sea || tile.terrain == Terrain::River ||
				                   tile.terrain == Terrain::Bridge;
				if (!water && !m_game.hexMap)
					for (int n : m_game.MapNeighbors(i))
					{
						const auto t = m_game.tiles[n].terrain;
						if (t != Terrain::Sea && t != Terrain::River && t != Terrain::Bridge)
							continue;
						if (n == i - 1)
							Line(rect.tl(), rect.bl()).draw(2, ColorF(.76, .73, .48, .6));
						else if (n == i + 1)
							Line(rect.tr(), rect.br()).draw(2, ColorF(.76, .73, .48, .6));
						else if (n < i)
							Line(rect.tl(), rect.tr()).draw(2, ColorF(.76, .73, .48, .6));
						else
							Line(rect.bl(), rect.br()).draw(2, ColorF(.76, .73, .48, .6));
					}
				if (m_game.hexMap && !water)
					shape.draw(ColorF(.47, .29, .12, .18));
				if (tile.owner >= 0)
					shape.draw(
					    ColorF(FactionColor(tile.owner), m_territoryView ? (m_supplyView ? .18 : .28) : .07));
				if (m_supplyView && supply[i] >= 0)
					shape.draw(ColorF(0.4, 0.95, 0.72, 0.22));
				if (m_supplyView && blockade[i] && tile.terrain != Terrain::Sea)
					shape.draw(ColorF(1.0, .35, .2, .35)).drawFrame(1, ColorF(1.0, .52, .3));
				if (!m_game.hexMap)
					shape.drawFrame(0.35, ColorF(.10, .14, .09, .10));
				if (m_game.hexMap)
				{
					for (int n : m_game.MapNeighbors(i))
						if (tile.owner != m_game.tiles[n].owner && tile.terrain != Terrain::Sea &&
						    m_game.tiles[n].terrain != Terrain::Sea)
						{
							const auto neighbor = hexgrid::Center(n % Width, n / Width);
							const Vec2 direction((neighbor.first - p.first) * TerrainPixels,
							                     (neighbor.second - p.second) * TerrainPixels *
							                         m_camera.Pitch());
							campaignvisual::TileEdge(rect.center(), TerrainPixels, m_camera.Pitch(),
							                         direction)
							    .draw(.9, ColorF(.83, .77, .49, .46));
						}
				}
				if (!m_game.hexMap && i % Width + 1 < Width && tile.owner != m_game.tiles[i + 1].owner)
					Line(rect.tr(), rect.br()).draw(1.5, ColorF(0.82, 0.80, 0.63, 0.6));
				if (!m_game.hexMap && i / Width + 1 < Height && tile.owner != m_game.tiles[i + Width].owner)
					Line(rect.bl(), rect.br()).draw(1.5, ColorF(0.82, 0.80, 0.63, 0.6));
			}
			if (m_game.hexMap)
				drawTerrainTransitions(TerrainPixels);
			if (m_game.hexMap)
				for (int i = 0; i < TileCount; ++i)
				{
					const auto p = hexgrid::Center(i % Width, i / Width);
					const Vec2 center(p.first * TerrainPixels, p.second * TerrainPixels * m_camera.Pitch());
					const auto terrain = m_game.tiles[i].terrain;
					const bool water =
					    terrain == Terrain::Sea || terrain == Terrain::River || terrain == Terrain::Bridge;
					if (water)
						for (int n : m_game.MapNeighbors(i))
						{
							const auto land = m_game.tiles[n].terrain;
							if (land == Terrain::Sea || land == Terrain::River || land == Terrain::Bridge)
								continue;
							const auto next = hexgrid::Center(n % Width, n / Width);
							campaignvisual::Shore(
							    center, TerrainPixels, m_camera.Pitch(),
							    Vec2((next.first - p.first) * TerrainPixels,
							         (next.second - p.second) * TerrainPixels * m_camera.Pitch()),
							    terrain == Terrain::Sea);
						}
				}
			if (m_game.hexMap)
				drawCityLandscape(TerrainPixels);
			if (m_game.hexMap)
				for (int i = 0; i < TileCount; ++i)
					if (m_game.tiles[i].terrain == Terrain::Bridge)
					{
						const auto p = hexgrid::Center(i % Width, i / Width);
						campaignvisual::TerrainRelief(
						    Vec2(p.first * TerrainPixels, p.second * TerrainPixels * m_camera.Pitch()),
						    TerrainPixels, Terrain::Bridge, i);
					}
			if (m_game.hexMap)
				for (int i = 0; i < TileCount; ++i)
				{
					if (m_game.tiles[i].owner < 0 || m_game.tiles[i].terrain == Terrain::Sea)
						continue;
					const auto p = hexgrid::Center(i % Width, i / Width);
					for (int n : m_game.MapNeighbors(i))
						if ((n > i || m_game.tiles[n].owner < 0) &&
						    m_game.tiles[n].owner != m_game.tiles[i].owner &&
						    m_game.tiles[n].terrain != Terrain::Sea)
						{
							const auto q = hexgrid::Center(n % Width, n / Width);
							const auto edge = campaignvisual::TileEdge(
							    Vec2(p.first * TerrainPixels, p.second * TerrainPixels * m_camera.Pitch()),
							    TerrainPixels, m_camera.Pitch(),
							    Vec2((q.first - p.first) * TerrainPixels,
							         (q.second - p.second) * TerrainPixels * m_camera.Pitch()));
							const Vec2 normal =
							    Vec2(q.first - p.first, (q.second - p.second) * m_camera.Pitch())
							        .normalized();
							edge.draw(6.3, ColorF(.015, .025, .03, .95));
							edge.movedBy(-normal * 1.35).draw(2.4, FactionColor(m_game.tiles[i].owner));
							if (m_game.tiles[n].owner >= 0)
								edge.movedBy(normal * 1.35).draw(2.4, FactionColor(m_game.tiles[n].owner));
						}
				}
		}
		m_mapRevision = m_game.revision;
		m_mapDay = m_game.day;
		m_mapSupply = m_supplyView;
		m_mapTerritory = m_territoryView;
	}
	area.stretched(5).draw(ColorF(.05, .07, .055)).drawFrame(1, ColorF(.58, .49, .30));
	area.draw(ColorF(.055, .1, .13));
	const auto oldScissor = Graphics2D::GetScissorRect();
	Graphics2D::SetScissorRect(Rect(static_cast<int>(area.x), static_cast<int>(area.y),
	                                static_cast<int>(area.w), static_cast<int>(area.h)));
	{
		const ScopedRenderStates2D clipping(RasterizerState::SolidCullNoneScissor);
		m_mapTexture(RectF(0, 0, m_camera.ExtentX() * TerrainPixels,
		                   m_camera.ExtentY() * TerrainPixels * m_camera.Pitch()))
		    .scaled(cell / TerrainPixels)
		    .drawAt(area.center() + Vec2((m_camera.ExtentX() / 2. - m_camera.x) * cell,
		                                 (m_camera.ExtentY() / 2. - m_camera.y) * cell * m_camera.Pitch()));
		if (m_game.hexMap)
		{
			area.draw(ColorF(.25, .34, .29, .16));
			campaignvisual::MapLight(area);
		}
		if (!m_game.legacyLayout && m_camera.zoom < 1.6)
		{
			const std::array<std::tuple<int, int, String>, 7> regions{{{24, 9, U"涼 州"},
			                                                           {24, 49, U"益 州"},
			                                                           {51, 46, U"荊 州"},
			                                                           {72, 53, U"江 東"},
			                                                           {64, 8, U"河 北"},
			                                                           {59, 30, U"中 原"},
			                                                           {40, 58, U"交 州"}}};
			for (const auto& [x, y, label] : regions)
				FontAsset(U"campaignBody")(label).drawAt(tileCenter(Campaign::At(x, y)),
				                                         ColorF(.96, .88, .65, .55));
		}
		drawWaterSurface(cell);
		drawTerrainRelief(cell);
		drawHexGrid(cell);
		if (m_army >= 0 && m_army < static_cast<int>(m_game.armies.size()) &&
		    m_game.armies[m_army].troops > 0)
		{
			const auto& army = m_game.armies[m_army];
			Vec2 last = tileCenter(army.tile);
			for (int p : army.path)
			{
				const Vec2 next = tileCenter(p);
				Line(last, next).draw(4, ColorF(.08, .11, .07, .6));
				Line(last, next).draw(1.5, ColorF(0.98, 0.84, 0.45, 0.95));
				last = next;
			}
			campaignvisual::TileShape(tileCenter(army.target), cell, m_game.hexMap, m_camera.Pitch())
			    .drawFrame(2, ColorF(0.97, 0.79, 0.36));
		}
		drawRegions(cell);
		for (int i = 0; i < static_cast<int>(m_game.cities.size()); ++i)
		{
			const auto& city = m_game.cities[i];
			const Vec2 center = tileCenter(city.tile);
			if (!m_game.hexMap ||
			    !m_mapArt.City(center, cell * (1 + city.level * .12), city.owner, i == m_city))
				campaignvisual::CityIcon(center, cell * (1 + city.level * .12), city.owner, i == m_city);
			RectF(center.x - cell * 0.4, center.y + cell * 0.5, cell * 0.8, 3).draw(ColorF(0.04, 0.07, 0.08));
			RectF(center.x - cell * 0.4, center.y + cell * 0.5, cell * 0.8 * Min(1.0, city.troops / 10000.0),
			      3)
			    .draw(FactionColor(city.owner));
			campaignvisual::CityBanner(
			    center.movedBy(0, -Max(29., cell * .94)),
			    text(city.name) + (city.level > 0 ? U" / " + String(citygrowth::Name(city.level)) : U""),
			    city.owner, i == m_city);
		}
		struct ArmyLabel
		{
			Vec2 p;
			String name;
			int faction;
			bool selected;
		};
		drawFormationLinks(cell);
		std::vector<ArmyLabel> labels;
		for (int i = 0; i < static_cast<int>(m_game.armies.size()); ++i)
		{
			const auto& army = m_game.armies[i];
			if (army.troops <= 0)
				continue;
			Vec2 center = i < static_cast<int>(m_positions.size()) ? m_positions[i] : tileCenter(army.tile);
			int count = 0, order = 0;
			for (int j = 0; j < static_cast<int>(m_game.armies.size()); ++j)
				if (m_game.armies[j].troops > 0 && m_game.armies[j].tile == army.tile)
				{
					if (j < i)
						++order;
					++count;
				}
			center.x += (order - (count - 1) * 0.5) * cell * 0.48;
			center.y += cell * 0.2;
			campaignvisual::ArmyIcon(center, cell, army, i == m_army, m_game.generals[army.general],
			                         m_campaignFaces);
			bool nearby = false;
			for (int j = 0; j < static_cast<int>(m_game.armies.size()); ++j)
				if (j != i && m_game.armies[j].troops > 0 &&
				    m_game.MapDistance(army.tile, m_game.armies[j].tile) <= 2)
					nearby = true;
			if (i == m_army || (count == 1 && !nearby))
			{
				const double offset = Max(95., cell * (count * .3 + 1.3));
				const double right = Scene::Width() - 20.;
				const Vec2 label =
				    (i == m_army && nearby)
				        ? center + Vec2(0, -offset)
				        : center + Vec2(center.x + offset + 65 < right ? offset : -offset, cell * .12);
				Line(center, label).draw(1, ColorF(.98, .87, .55, .75));
				labels.push_back({label, text(m_game.generals[army.general].name) + U" {}"_fmt(army.troops),
				                  army.faction, i == m_army});
			}

			if (!army.supplied)
				Circle(center.movedBy(cell * 0.35, -cell * 0.35), 3).draw(ColorF(1.0, 0.55, 0.25));
			if (army.tacticLeft > 0)
				Line(center.movedBy(-cell * 0.3, cell * 0.4), center.movedBy(cell * 0.3, cell * 0.4))
				    .draw(3, ColorF(0.98, 0.81, 0.36));
		}
		for (const auto& label : labels)
			campaignvisual::MapLabel(label.p, label.name, label.faction, label.selected);
		for (const auto& hit : m_hits)
			FontAsset(U"campaignSmall")(U"-{}"_fmt(hit.amount))
			    .drawAt(hit.position.movedBy(0, -12 - (0.75 - hit.time) * 30),
			            ColorF(1.0, 0.7, 0.4, Min(1.0, hit.time * 2)));
		for (int i = 0; i < static_cast<int>(m_game.armies.size()); ++i)
			for (int j = i + 1; j < static_cast<int>(m_game.armies.size()); ++j)
			{
				const auto& a = m_game.armies[i];
				const auto& b = m_game.armies[j];

				if (a.troops <= 0 || b.troops <= 0 || !m_game.Hostile(a.faction, b.faction) ||
				    (!m_game.CanStrike(i, b.tile) && !m_game.CanStrike(j, a.tile)))
					continue;
				const Vec2 center = (tileCenter(a.tile) + tileCenter(b.tile)) / 2;
				campaignvisual::Clash(center, cell, i * 31 + j);
			}
		drawSieges(cell);
		m_spectacle.DrawMap(m_game, [&](int tile) { return tileCenter(tile); }, cell);
		m_presentation.DrawMap([&](int tile) { return tileCenter(tile); }, cell);
		const int hover = mouseTile();
		if (Campaign::Valid(hover))
			campaignvisual::TileShape(tileCenter(hover), cell, m_game.hexMap, m_camera.Pitch())
			    .drawFrame(1, ColorF(0.96, 0.95, 0.8, 0.8));
	}
	Graphics2D::SetScissorRect(oldScissor);
	area.drawFrame(1.5, ColorF(.74, .65, .42));
	if (m_camera.zoom > 1)
	{
		const auto mini = miniRect();
		campaignui::Panel(mini.stretched(5));
		m_mapTexture(RectF(0, 0, m_camera.ExtentX() * TerrainPixels,
		                   m_camera.ExtentY() * TerrainPixels * m_camera.Pitch()))
		    .resized(mini.w, mini.h)
		    .draw(mini.pos);
		for (const auto& city : m_game.cities)
		{
			const auto p = m_game.hexMap
			                   ? hexgrid::Center(city.tile % Width, city.tile / Width)
			                   : std::pair<double, double>{city.tile % Width + .5, city.tile / Width + .5};
			Circle(mini.pos +
			           Vec2(p.first / m_camera.ExtentX() * mini.w, p.second / m_camera.ExtentY() * mini.h),
			       2.5)
			    .draw(FactionColor(city.owner));
		}
		const double hw = Min(m_camera.ExtentX() / 2., area.w / cell / 2),
		             hh = Min(m_camera.ExtentY() / 2., area.h / (cell * m_camera.Pitch()) / 2);
		RectF(mini.x + (m_camera.x - hw) / m_camera.ExtentX() * mini.w,
		      mini.y + (m_camera.y - hh) / m_camera.ExtentY() * mini.h, hw * 2 / m_camera.ExtentX() * mini.w,
		      hh * 2 / m_camera.ExtentY() * mini.h)
		    .drawFrame(2, ColorF(.98, .85, .46));
	}
	if (m_started)
		drawTerritoryLegend();
}
