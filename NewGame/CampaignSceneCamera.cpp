#include "CampaignScene.hpp"
#include "CampaignSave.hpp"
#include "CampaignUI.hpp"
#include "CampaignVisuals.hpp"

using namespace frontline;
using namespace campaignui;

bool CampaignScene::panelVisible() const
{
	return m_started && !m_panelHidden &&
	       (m_city >= 0 || m_region >= 0 ||
	        (m_army >= 0 && m_army < static_cast<int>(m_game.armies.size()) &&
	         m_game.armies[m_army].troops > 0));
}

bool CampaignScene::overMapUI() const
{
	const Vec2 p = Cursor::PosF();
	if (m_settingsOpen)
		return true;
	if (m_started && territoryLegendRect().contains(p))
		return true;
	if (!m_presentation.Busy() && m_spectacle.ReportTile() >= 0 && m_spectacle.ReportRect().contains(p))
		return true;
	if (p.y < 76 || p.y >= Scene::Height() - 58)
		return true;
	if (!m_atlasView && m_camera.zoom > 1 && miniRect().stretched(5).contains(p))
		return true;
	for (int id : {49, 53, 54, 55})
		if (button(id).mouseOver())
			return true;
	if (panelVisible() && RectF(16, 86, 306, Scene::Height() - 160).contains(p))
		return true;
	if (m_atlasView)
		return RectF(Scene::Width() - 246, 94, 224, 172).contains(p);
	const double logX = panelVisible() ? 340 : 20;
	return RectF(logX, Scene::Height() - 142, Scene::Width() - logX - 270, 70).contains(p);
}

void CampaignScene::fitMap()
{
	m_atlasView = false;
	m_atlasCamera.Reset();
	m_camera.Fit(m_game.MapWidth(), m_game.MapHeight(), m_game.hexMap, true);
	m_panelHidden = false;
	m_camera.zoom = 1.8;
	for (const auto& city : m_game.cities)
		if (city.owner == m_game.player)
		{
			const auto p = m_game.hexMap
			                   ? hexgrid::Center(city.tile % frontline::Width, city.tile / frontline::Width)
			                   : std::pair<double, double>{city.tile % frontline::Width + .5,
			                                               city.tile / frontline::Width + .5};
			m_camera.x = p.first;
			m_camera.y = p.second;
			break;
		}
}

RectF CampaignScene::mapRect() const
{
	return RectF(0, 0, Scene::Width(), Scene::Height());
}

RectF CampaignScene::miniRect() const
{
	const auto r = mapRect();
	return RectF(Scene::Width() - 246, Scene::Height() - 228, 224, 144);
}

Vec2 CampaignScene::tileCenter(int tile) const
{
	if (m_atlasView && m_game.hexMap)
		return atlasPosition(tile);
	const auto r = mapRect();
	const double c = m_camera.Cell(r.w, r.h);
	const auto p = m_game.hexMap ? hexgrid::Center(tile % Width, tile / Width)
	                             : std::pair<double, double>{tile % Width + .5, tile / Width + .5};
	return r.center() + Vec2((p.first - m_camera.x) * c, (p.second - m_camera.y) * c * m_camera.Pitch());
}

int CampaignScene::mouseTile() const
{
	const auto r = mapRect();
	if (!r.mouseOver() || overMapUI())
		return -1;
	if (m_atlasView && m_game.hexMap)
	{
		if (!atlasViewport().mouseOver())
			return -1;
		if (MouseL.down() || MouseR.down())
		{
			double closest = 17 * 17;
			int found = -1;
			for (const auto& city : m_game.cities)
			{
				const double d = atlasPosition(city.tile).distanceFromSq(Cursor::PosF());
				if (d < closest)
				{
					closest = d;
					found = city.tile;
				}
			}
			if (found >= 0)
				return found;
			for (const auto& army : m_game.armies)
				if (army.troops > 0 &&
				    (atlasPosition(army.tile) + Vec2(16, 0)).distanceFrom(Cursor::PosF()) < 9)
					return army.tile;
		}
		return -1;
	}
	if (MouseL.down() || MouseR.down())
		for (const auto& city : m_game.cities)
			if (tileCenter(city.tile).distanceFrom(Cursor::PosF()) < 11 ||
			    campaignvisual::CityBannerRect(
			        tileCenter(city.tile) + Vec2(0, -Max(29., m_camera.Cell(r.w, r.h) * .94)),
			        text(city.name) + (city.level > 0 ? U" / " + String(citygrowth::Name(city.level)) : U""))
			        .mouseOver())
				return city.tile;
	const auto p = Cursor::PosF() - r.pos;
	return m_camera.Tile(p.x, p.y, r.w, r.h, Width);
}

void CampaignScene::updateCamera()
{
	if (m_atlasView && m_game.hexMap)
	{
		updateAtlasCamera();
		return;
	}
	const auto r = mapRect();
	const auto p = Cursor::PosF() - r.pos;
	const double ox = m_camera.x, oy = m_camera.y, oz = m_camera.zoom;
	m_camera.Clamp(r.w, r.h);
	const double step = 500 * Scene::DeltaTime();
	m_camera.Pan((KeyLeft.pressed() ? step : 0) - (KeyRight.pressed() ? step : 0),
	             (KeyUp.pressed() ? step : 0) - (KeyDown.pressed() ? step : 0), r.w, r.h);
	if (r.mouseOver() && !overMapUI() && Mouse::Wheel() != 0)
		m_camera.Zoom(std::pow(1.25, -Mouse::Wheel()), p.x, p.y, r.w, r.h);
	if (r.mouseOver() && !overMapUI() && MouseM.pressed())
	{
		const auto d = Cursor::DeltaF();
		m_camera.Pan(d.x, d.y, r.w, r.h);
	}
	if (KeyHome.down() || button(53).leftClicked())
		m_camera.Fit(m_game.MapWidth(), m_game.MapHeight(), m_game.hexMap, true);
	if (button(54).leftClicked())
		m_camera.Zoom(1.4, r.w / 2, r.h / 2, r.w, r.h);
	if (button(55).leftClicked())
		m_camera.Zoom(1 / 1.4, r.w / 2, r.h / 2, r.w, r.h);
	if (!m_atlasView && m_camera.zoom > 1 && miniRect().mouseOver() && MouseL.pressed())
	{
		const auto n = miniRect();
		const auto q = Cursor::PosF() - n.pos;
		m_camera.x = q.x / n.w * m_camera.ExtentX();
		m_camera.y = q.y / n.h * m_camera.ExtentY();
		m_camera.Clamp(r.w, r.h);
	}
	if (ox != m_camera.x || oy != m_camera.y || oz != m_camera.zoom)
	{
		m_positions.clear();
		m_hits.clear();
	}
}
