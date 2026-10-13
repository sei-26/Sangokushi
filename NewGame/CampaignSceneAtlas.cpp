#include "CampaignScene.hpp"
#include "CampaignUI.hpp"
#include "ChinaAtlas.hpp"
#include "CampaignVisuals.hpp"
using namespace frontline;
namespace
{
	ColorF AtlasColor(int f)
	{
		return f == 0   ? ColorF(.19, .43, .16)
		       : f == 1 ? ColorF(.20, .22, .68)
		       : f == 2 ? ColorF(.65, .14, .14)
		                : ColorF(.40, .39, .36);
	}
	String Crest(int f)
	{
		return f == 0 ? U"蜀" : f == 1 ? U"魏" : f == 2 ? U"呉" : U"〇";
	}
} // namespace
RectF CampaignScene::atlasViewport() const
{
	const RectF space(340, 82, Scene::Width() - 604, Scene::Height() - 150);
	const double ratio =
	    m_atlasTexture ? static_cast<double>(m_atlasTexture.width()) / m_atlasTexture.height() : .8;
	const double h = Min(space.h, space.w / ratio);
	return RectF(Arg::center = space.center(), h * ratio, h);
}
RectF CampaignScene::atlasRect() const
{
	const auto view = atlasViewport();
	const double z = m_atlasCamera.zoom;
	return RectF(Arg::center = view.center() +
	                           Vec2((.5 - m_atlasCamera.x) * view.w * z, (.5 - m_atlasCamera.y) * view.h * z),
	             view.w * z, view.h * z);
}
void CampaignScene::updateAtlasCamera()
{
	const auto view = atlasViewport();
	const double ox = m_atlasCamera.x, oy = m_atlasCamera.y, oz = m_atlasCamera.zoom;
	if (KeyHome.down() || button(53).leftClicked())
		m_atlasCamera.Reset();
	if (button(54).leftClicked())
		m_atlasCamera.Zoom(1.4, view.w * .5, view.h * .5, view.w, view.h);
	if (button(55).leftClicked())
		m_atlasCamera.Zoom(1 / 1.4, view.w * .5, view.h * .5, view.w, view.h);
	if (view.mouseOver() && !overMapUI())
	{
		const auto p = Cursor::PosF() - view.pos;
		if (Mouse::Wheel() != 0)
			m_atlasCamera.Zoom(std::pow(1.25, -Mouse::Wheel()), p.x, p.y, view.w, view.h);
		if (MouseM.pressed())
		{
			const auto d = Cursor::DeltaF();
			m_atlasCamera.Pan(d.x, d.y, view.w, view.h);
		}
	}
	const double step = 500 * Scene::DeltaTime();
	m_atlasCamera.Pan((KeyLeft.pressed() ? step : 0) - (KeyRight.pressed() ? step : 0),
	                  (KeyUp.pressed() ? step : 0) - (KeyDown.pressed() ? step : 0), view.w, view.h);
	if (ox != m_atlasCamera.x || oy != m_atlasCamera.y || oz != m_atlasCamera.zoom)
	{
		m_positions.clear();
		m_hits.clear();
	}
}
Vec2 CampaignScene::atlasPosition(int tile) const
{
	const auto p = chinaatlas::Project(tile % Width, tile / Width);
	const auto r = atlasRect();
	return r.pos + Vec2(p.x * r.w, p.y * r.h);
}
void CampaignScene::ensureAtlasGrid() const
{
	if (m_atlasGrid)
		return;
	const Size size = m_atlasTexture ? m_atlasTexture.size() : Size(1122, 1402);
	m_atlasGrid = RenderTexture(size, ColorF(0, 0));
	const double cell = size.x / 64.;
	const int rows = static_cast<int>(std::ceil(size.y / (cell * hexgrid::Row))) + 1;
	m_atlasCells.resize(64 * rows);
	const ScopedRenderTarget2D target(m_atlasGrid);
	// Transparent overlays need alpha writes; Default2D preserves destination alpha.
	const ScopedRenderStates2D alpha(BlendState::MaxAlpha);
	const ScopedRenderStates2D unclipped(RasterizerState::SolidCullNone);
	for (int y = 0; y < rows; ++y)
		for (int x = 0; x < 64; ++x)
		{
			const auto c = hexgrid::Center(x, y);
			const auto shape = campaignvisual::TileShape(Vec2(c.first * cell, c.second * cell), cell, true);
			m_atlasCells[y * 64 + x] = shape;
			shape.drawFrame(1.6, ColorF(.05, .03, .015, .28));
			shape.drawFrame(1.0, ColorF(.93, .82, .54, .45));
		}
}
int CampaignScene::atlasGridCellAt(Vec2 point) const
{
	const auto r = atlasRect();
	if (!atlasViewport().contains(point) || !r.contains(point))
		return -1;
	ensureAtlasGrid();
	const double cell = m_atlasGrid.width() / 64.;
	const auto p = hexgrid::Pick((point.x - r.x) / r.w * m_atlasGrid.width() / cell,
	                             (point.y - r.y) / r.h * m_atlasGrid.height() / cell);
	const int rows = static_cast<int>(m_atlasCells.size()) / 64;
	return p.first >= 0 && p.first < 64 && p.second >= 0 && p.second < rows ? p.second * 64 + p.first : -1;
}
void CampaignScene::drawAtlasContents() const
{
	const auto r = atlasRect();
	if (m_atlasTexture)
		m_atlasTexture.resized(r.w, r.h).draw(r.pos);
	else
		r.draw(ColorF(.26, .21, .14));
	r.draw(ColorF(.055, .035, .02, .23));
	unsigned hash = 2166136261u;
	for (const auto& city : m_game.cities)
		hash = (hash ^ static_cast<unsigned>(city.owner + 1)) * 16777619u;
	if (!m_atlasTerritory || hash != m_atlasOwnerHash)
	{
		constexpr int w = 320, h = 400;
		Image paint(w, h, Color(0, 0, 0, 0));
		std::array<int, w * h> owners{};
		owners.fill(-1);
		for (int y = 0; y < h; ++y)
			for (int x = 0; x < w; ++x)
			{
				if (m_atlasSource)
				{
					const auto pixel =
					    m_atlasSource[y * m_atlasSource.height() / h][x * m_atlasSource.width() / w];
					if (pixel.b > pixel.r + 4)
						continue;
				}
				const Vec2 p((x + .5) / w, (y + .5) / h);
				double best = 10;
				int owner = -1;
				for (size_t i = 0; i < m_game.cities.size() && i < chinaatlas::Cities.size(); ++i)
				{
					const auto a = chinaatlas::Cities[i];
					const double d = (p - Vec2(a.x, a.y)).lengthSq();
					if (d < best)
					{
						best = d;
						owner = m_game.cities[i].owner;
					}
				}
				const double fade = 1 - Clamp((best - .015) / .065, 0., 1.);
				if (fade <= 0)
					continue;
				owners[y * w + x] = owner;
				paint[y][x] = ColorF(AtlasColor(owner), fade * .48).toColor();
			}
		for (int y = 1; y < h - 1; ++y)
			for (int x = 1; x < w - 1; ++x)
				if (owners[y * w + x] >= 0 &&
				    ((owners[y * w + x + 1] >= 0 && owners[y * w + x + 1] != owners[y * w + x]) ||
				     (owners[(y + 1) * w + x] >= 0 && owners[(y + 1) * w + x] != owners[y * w + x])))
					paint[y][x] = Color(186, 159, 80, 115);
		m_atlasTerritory = Texture(paint, TextureDesc::Mipped);
		m_atlasOwnerHash = hash;
	}
	if (m_territoryView)
		m_atlasTerritory.resized(r.w, r.h).draw(r.pos);
	ensureAtlasGrid();
	if (m_gridVisible)
		m_atlasGrid.resized(r.w, r.h).draw(r.pos);
	const int hover = overMapUI() ? -1 : atlasGridCellAt(Cursor::PosF());
	if (m_gridVisible && hover >= 0 && static_cast<size_t>(hover) < m_atlasCells.size())
		m_atlasCells[hover]
		    .scaled(r.w / m_atlasGrid.width(), r.h / m_atlasGrid.height())
		    .movedBy(r.pos)
		    .draw(ColorF(1., .85, .32, .16))
		    .drawFrame(1.4, ColorF(1., .89, .43, .9));
	r.drawFrame(1, ColorF(.49, .40, .24, .60));
	// Routes come from actual army orders; the atlas only projects the coordinates.
	for (const auto& army : m_game.armies)
	{
		if (army.troops <= 0 || army.path.empty() || army.faction != m_game.player)
			continue;
		Vec2 last = atlasPosition(army.tile);
		for (int tile : army.path)
		{
			const Vec2 next = atlasPosition(tile);
			Line(last, next).draw(2, ColorF(1., .83, .16, .65));
			last = next;
		}
		if (army.path.size() > 1)
		{
			const Vec2 start = atlasPosition(army.path[army.path.size() - 2]);
			const Vec2 direction = last - start;
			if (direction.lengthSq() > 1e-6)
			{
				const Vec2 unit = direction.normalized(), side(-unit.y, unit.x);
				Triangle(last, last - unit * 10 + side * 4, last - unit * 10 - side * 4)
				    .draw(ColorF(1., .84, .16, .95));
			}
		}
	}
	Array<RectF> labelAreas;
	Array<RectF> shields;
	for (const auto& city : m_game.cities)
		shields << RectF(Arg::center = atlasPosition(city.tile), 28, 34);
	for (size_t i = 0; i < m_game.cities.size(); ++i)
	{
		const auto& city = m_game.cities[i];
		const Vec2 p = atlasPosition(city.tile);
		const bool selected = static_cast<int>(i) == m_city;
		const auto badge = Polygon{Array<Vec2>{p + Vec2(-12, -16), p + Vec2(12, -16), p + Vec2(12, 9),
		                                       p + Vec2(0, 17), p + Vec2(-12, 9)}};
		badge.movedBy(2, 3).draw(ColorF(0, 0, 0, .60));
		badge.draw(AtlasColor(city.owner))
		    .drawFrame(selected ? 2.5 : 1.2, selected ? ColorF(1., .85, .25) : ColorF(.88, .87, .78));
		FontAsset(U"campaignBody")(Crest(city.owner)).drawAt(p + Vec2(0, -1), ColorF(.96, .95, .84));
		const auto glyph = FontAsset(U"campaignSmall")(text(city.name));
		const auto bounds = glyph.region();
		const std::array<Vec2, 6> offsets{{{0, 29}, {0, -29}, {34, 0}, {-34, 0}, {34, 24}, {-34, 24}}};
		Vec2 label = p + offsets[0];
		int score = 100000;
		for (size_t k = 0; k < offsets.size(); ++k)
		{
			const Vec2 q = p + offsets[k];
			const RectF area(Arg::center = q, bounds.w + 6, 21);
			int cost = static_cast<int>(k);
			for (const auto& other : labelAreas)
				if (area.intersects(other))
					cost += 20;
			for (const auto& other : shields)
				if (area.intersects(other))
					cost += 10;
			if (!atlasViewport().contains(q))
				cost += 100;
			if (cost < score)
			{
				score = cost;
				label = q;
			}
		}
		labelAreas << RectF(Arg::center = label, bounds.w + 6, 21);
		glyph.drawAt(label + Vec2(1, 1), ColorF(.06, .035, .015, .98));
		glyph.drawAt(label, selected ? ColorF(1., .96, .49) : ColorF(.99, .85, .21));
	}
	for (const auto& army : m_game.armies)
		if (army.troops > 0)
		{
			const Vec2 p = atlasPosition(army.tile) + Vec2(16, 0);
			Circle(p, 5).draw(campaignui::FactionColor(army.faction)).drawFrame(1, ColorF(.98, .89, .58));
		}
}
void CampaignScene::drawAtlas() const
{
	Scene::Rect().draw(ColorF(.055, .045, .035));
	const auto view = atlasViewport();
	const auto previous = Graphics2D::GetScissorRect();
	Graphics2D::SetScissorRect(Rect(static_cast<int>(view.x), static_cast<int>(view.y),
	                                static_cast<int>(view.w), static_cast<int>(view.h)));
	{
		const ScopedRenderStates2D clipping(RasterizerState::SolidCullNoneScissor);
		drawAtlasContents();
	}
	Graphics2D::SetScissorRect(previous);
	view.drawFrame(1, ColorF(.49, .40, .24, .7));
	(void)FontAsset(U"campaignSmall")(
	    U"倍率 {:.1f} / ホイールで拡縮\n中ドラッグ・矢印キーで移動"_fmt(m_atlasCamera.zoom))
	    .draw(RectF(Scene::Width() - 246, 278, 224, 64), ColorF(.97, .91, .73));
	if (!panelVisible())
	{
		campaignui::Panel(RectF(16, 94, 306, 244), true);
		FontAsset(U"campaignTitle")(U"天下勢力図").draw(30, 105, ColorF(.97, .88, .60));
		FontAsset(U"campaignSmall")(U"都市の支配勢力・進軍を俯瞰").draw(30, 150, ColorF(.84, .88, .86));
		for (int f = 0; f < 3; ++f)
		{
			int count = 0;
			for (const auto& city : m_game.cities)
				if (city.owner == f)
					++count;
			RectF(30, 189 + f * 32, 20, 22).draw(AtlasColor(f)).drawFrame(1, ColorF(.82, .80, .66));
			FontAsset(U"campaignSmall")(Crest(f)).drawAt(Vec2(40, 200 + f * 32), ColorF(.95, .93, .82));
			FontAsset(U"campaignBody")(text(Campaign::FactionName(f)) + U"軍 / {}都市"_fmt(count))
			    .draw(60, 186 + f * 32, ColorF(.92, .94, .89));
		}
		FontAsset(U"campaignSmall")(U"都市選択 → 命令 / F → 戦場図").draw(30, 300, ColorF(.92, .84, .63));
	}
	drawTerritoryLegend();
}
