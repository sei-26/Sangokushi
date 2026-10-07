#include "CampaignScene.hpp"
#include "CampaignSave.hpp"
#include "CampaignUI.hpp"

using namespace frontline;
using namespace campaignui;

void CampaignScene::drawMap() const
{
	const RectF area = mapRect();
	const double cell = m_camera.Cell(area.w, area.h);
	if (!m_mapTexture || m_mapRevision != m_game.revision || m_mapSupply != m_supplyView ||
	    (m_supplyView && m_mapDay != m_game.day))
	{
		if (!m_mapTexture)
			m_mapTexture = RenderTexture(Width * 32, Height * 32, ColorF(0.08, 0.12, 0.13));
		const auto supply = m_game.Supply(m_game.player);
		{
			const ScopedRenderTarget2D target(m_mapTexture);
			for (int i = 0; i < TileCount; ++i)
			{
				const auto& tile = m_game.tiles[i];
				const RectF rect((i % Width) * 32, (i / Width) * 32, 32, 32);
				ColorF color(0.32, 0.37, 0.29);
				if (tile.terrain == Terrain::Forest)
					color = ColorF(0.17, 0.29, 0.23);
				else if (tile.terrain == Terrain::Mountain)
					color = ColorF(0.36, 0.34, 0.31);
				else if (tile.terrain == Terrain::River || tile.terrain == Terrain::Sea)
					color = ColorF(0.12, 0.24, 0.32);
				rect.draw(color);
				if (tile.owner >= 0)
					rect.draw(ColorF(FactionColor(tile.owner), 0.28));
				if (m_supplyView && supply[i] >= 0)
					rect.stretched(-3).draw(ColorF(0.4, 0.95, 0.72, 0.22));
				if (tile.terrain == Terrain::Mountain)
					Triangle(rect.center().movedBy(0, -8), rect.center().movedBy(-9, 7),
					         rect.center().movedBy(9, 7))
					    .draw(ColorF(0.6, 0.57, 0.51, 0.4));
				else if (tile.terrain == Terrain::Forest)
				{
					Circle(rect.center().movedBy(-4, 0), 6).draw(ColorF(0.1, 0.2, 0.15));
					Circle(rect.center().movedBy(5, -3), 5).draw(ColorF(0.12, 0.24, 0.16));
				}
				else if (tile.terrain == Terrain::Bridge)
					RectF(rect.x + 4, rect.y + 13, 24, 6).draw(ColorF(0.72, 0.63, 0.43));
				rect.drawFrame(0.5, ColorF(0.05, 0.10, 0.12, 0.35));
				if (i % Width + 1 < Width && tile.owner != m_game.tiles[i + 1].owner)
					Line(rect.tr(), rect.br()).draw(1.5, ColorF(0.82, 0.80, 0.63, 0.6));
				if (i / Width + 1 < Height && tile.owner != m_game.tiles[i + Width].owner)
					Line(rect.bl(), rect.br()).draw(1.5, ColorF(0.82, 0.80, 0.63, 0.6));
			}
		}
		m_mapRevision = m_game.revision;
		m_mapDay = m_game.day;
		m_mapSupply = m_supplyView;
	}
	area.draw(ColorF(.055, .1, .13));
	const auto oldScissor = Graphics2D::GetScissorRect();
	Graphics2D::SetScissorRect(Rect(static_cast<int>(area.x), static_cast<int>(area.y),
	                                static_cast<int>(area.w), static_cast<int>(area.h)));
	{
		const ScopedRenderStates2D clipping(RasterizerState::SolidCullNoneScissor);
		m_mapTexture(RectF(0, 0, m_game.MapWidth() * 32, m_game.MapHeight() * 32))
		    .scaled(cell / 32)
		    .drawAt(area.center() + Vec2((m_game.MapWidth() / 2. - m_camera.x) * cell,
		                                 (m_game.MapHeight() / 2. - m_camera.y) * cell));
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
		if (m_army >= 0 && m_army < static_cast<int>(m_game.armies.size()) &&
		    m_game.armies[m_army].troops > 0)
		{
			const auto& army = m_game.armies[m_army];
			Vec2 last = tileCenter(army.tile);
			for (int p : army.path)
			{
				const Vec2 next = tileCenter(p);
				Line(last, next).draw(2, ColorF(0.93, 0.79, 0.4, 0.9));
				last = next;
			}
			RectF(Arg::center(tileCenter(army.target)), cell, cell).drawFrame(2, ColorF(0.97, 0.79, 0.36));
		}
		for (int i = 0; i < static_cast<int>(m_game.cities.size()); ++i)
		{
			const auto& city = m_game.cities[i];
			const Vec2 center = tileCenter(city.tile);
			RectF(Arg::center(center), Max(12., cell * 0.75), Max(12., cell * 0.75))
			    .draw(ColorF(0.07, 0.09, 0.10))
			    .drawFrame(2, FactionColor(city.owner));
			RectF(Arg::center(center.movedBy(0, -3)), cell * 0.45, cell * 0.35)
			    .draw(FactionColor(city.owner));
			RectF(center.x - cell * 0.4, center.y + cell * 0.5, cell * 0.8, 3).draw(ColorF(0.04, 0.07, 0.08));
			RectF(center.x - cell * 0.4, center.y + cell * 0.5, cell * 0.8 * Min(1.0, city.troops / 10000.0),
			      3)
			    .draw(FactionColor(city.owner));
			if (i == m_city)
				Circle(center, cell * 0.7).drawFrame(2, ColorF(0.98, 0.82, 0.4));
			FontAsset(U"campaignSmall")(text(city.name))
			    .drawAt(center.movedBy(0, -Max(16., cell * 0.85)), ColorF(0.05, 0.06, 0.07));
			FontAsset(U"campaignSmall")(text(city.name))
			    .drawAt(center.movedBy(0, -Max(16., cell * 0.85) - 1), ColorF(0.96, 0.94, 0.83));
		}
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
			Circle(center, cell * 0.33)
			    .draw(FactionColor(army.faction))
			    .drawFrame(1.5, ColorF(0.96, 0.93, 0.8));
			if (i == m_army)
				Circle(center, cell * 0.47).drawFrame(2, ColorF(0.98, 0.82, 0.4));
			const String icon = army.arm == Arm::Siege     ? U"城"
			                    : army.arm == Arm::Bow     ? U"弓"
			                    : army.arm == Arm::Cavalry ? U"騎"
			                                               : U"槍";
			FontAsset(U"campaignSmall")(icon).drawAt(center, ColorF(0.04, 0.08, 0.08));
			bool nearby = false;
			for (int j = 0; j < static_cast<int>(m_game.armies.size()); ++j)
				if (j != i && m_game.armies[j].troops > 0 &&
				    Campaign::Distance(army.tile, m_game.armies[j].tile) <= 1)
					nearby = true;
			if (i == m_army || (count == 1 && !nearby))
				FontAsset(U"campaignSmall")(text(m_game.generals[army.general].name))
				    .drawAt(center.movedBy(0, cell * 0.7), ColorF(0.99, 0.96, 0.86));
			if (!army.supplied)
				Circle(center.movedBy(cell * 0.35, -cell * 0.35), 3).draw(ColorF(1.0, 0.55, 0.25));
			if (army.tacticLeft > 0)
				Line(center.movedBy(-cell * 0.3, cell * 0.4), center.movedBy(cell * 0.3, cell * 0.4))
				    .draw(3, ColorF(0.98, 0.81, 0.36));
		}
		for (const auto& hit : m_hits)
			FontAsset(U"campaignSmall")(U"-{}"_fmt(hit.amount))
			    .drawAt(hit.position.movedBy(0, -12 - (0.75 - hit.time) * 30),
			            ColorF(1.0, 0.7, 0.4, Min(1.0, hit.time * 2)));
		for (int i = 0; i < static_cast<int>(m_game.armies.size()); ++i)
			for (int j = i + 1; j < static_cast<int>(m_game.armies.size()); ++j)
			{
				const auto& a = m_game.armies[i];
				const auto& b = m_game.armies[j];
				const int rangeA =
				    a.arm == Arm::Bow
				        ? (a.tacticLeft > 0 && m_game.generals[a.general].tactic == Tactic::Volley ? 3 : 2)
				        : 1;
				const int rangeB =
				    b.arm == Arm::Bow
				        ? (b.tacticLeft > 0 && m_game.generals[b.general].tactic == Tactic::Volley ? 3 : 2)
				        : 1;
				if (a.troops <= 0 || b.troops <= 0 || !m_game.Hostile(a.faction, b.faction) ||
				    Campaign::Distance(a.tile, b.tile) > Max(rangeA, rangeB))
					continue;
				const Vec2 center = (tileCenter(a.tile) + tileCenter(b.tile)) / 2;
				Line(center.movedBy(-4, -5), center.movedBy(4, 5)).draw(2, ColorF(1.0, 0.72, 0.35));
				Line(center.movedBy(4, -5), center.movedBy(-4, 5)).draw(2, ColorF(1.0, 0.72, 0.35));
			}
		const int hover = mouseTile();
		if (Campaign::Valid(hover))
			RectF(Arg::center(tileCenter(hover)), cell, cell).drawFrame(1, ColorF(0.96, 0.95, 0.8, 0.8));
	}
	Graphics2D::SetScissorRect(oldScissor);
	area.drawFrame(2, ColorF(.48, .55, .48));
	if (m_camera.zoom > 1)
	{
		const auto mini = miniRect();
		mini.stretched(4).draw(ColorF(.02, .05, .07, .95));
		m_mapTexture(RectF(0, 0, m_game.MapWidth() * 32, m_game.MapHeight() * 32))
		    .resized(mini.w, mini.h)
		    .draw(mini.pos);
		for (const auto& city : m_game.cities)
			Circle(mini.pos + Vec2((city.tile % Width + .5) / m_game.MapWidth() * mini.w,
			                       (city.tile / Width + .5) / m_game.MapHeight() * mini.h),
			       2.5)
			    .draw(FactionColor(city.owner));
		const double hw = Min(m_game.MapWidth() / 2., area.w / cell / 2),
		             hh = Min(m_game.MapHeight() / 2., area.h / cell / 2);
		RectF(mini.x + (m_camera.x - hw) / m_game.MapWidth() * mini.w,
		      mini.y + (m_camera.y - hh) / m_game.MapHeight() * mini.h, hw * 2 / m_game.MapWidth() * mini.w,
		      hh * 2 / m_game.MapHeight() * mini.h)
		    .drawFrame(2, ColorF(.98, .85, .46));
	}
	const int y = static_cast<int>(area.bottomY() + 12);
	DrawButton(Rect(24, y, 100, 30), U"全図 / Home");
	DrawButton(Rect(134, y, 70, 30), U"＋ 拡大");
	DrawButton(Rect(214, y, 70, 30), U"－ 縮小");
	FontAsset(U"campaignSmall")(
	    U"{}都市 / {}×{}マス　倍率 {:.1f}　ホイール：拡縮 / 中ボタン・矢印：移動"_fmt(
	        m_game.cities.size(), m_game.MapWidth(), m_game.MapHeight(), m_camera.zoom))
	    .draw(300, y + 5, ColorF(.8, .85, .73));
}
