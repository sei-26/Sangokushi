#include "CampaignScene.hpp"
#include "CampaignSave.hpp"
#include "CampaignUI.hpp"

using namespace frontline;
using namespace campaignui;

RectF CampaignScene::mapRect() const
{
	return RectF(24, 104, Scene::Width() - 390, Scene::Height() - 270);
}

RectF CampaignScene::miniRect() const
{
	const auto r = mapRect();
	return RectF(r.rightX() - 202, r.bottomY() - 146, 190, 134);
}

Vec2 CampaignScene::tileCenter(int tile) const
{
	const auto r = mapRect();
	const double c = m_camera.Cell(r.w, r.h);
	return r.center() + Vec2((tile % Width + .5 - m_camera.x) * c, (tile / Width + .5 - m_camera.y) * c);
}

int CampaignScene::mouseTile() const
{
	const auto r = mapRect();
	if (!r.mouseOver() || (m_camera.zoom > 1 && miniRect().mouseOver()))
		return -1;
	if (MouseL.down() || MouseR.down())
		for (const auto& city : m_game.cities)
			if (tileCenter(city.tile).distanceFrom(Cursor::PosF()) < 11)
				return city.tile;
	const auto p = Cursor::PosF() - r.pos;
	return m_camera.Tile(p.x, p.y, r.w, r.h, Width);
}

void CampaignScene::updateCamera()
{
	const auto r = mapRect();
	const auto p = Cursor::PosF() - r.pos;
	const double ox = m_camera.x, oy = m_camera.y, oz = m_camera.zoom;
	m_camera.Clamp(r.w, r.h);
	const double step = 500 * Scene::DeltaTime();
	m_camera.Pan((KeyLeft.pressed() ? step : 0) - (KeyRight.pressed() ? step : 0),
	             (KeyUp.pressed() ? step : 0) - (KeyDown.pressed() ? step : 0), r.w, r.h);
	if (r.mouseOver() && Mouse::Wheel() != 0)
		m_camera.Zoom(std::pow(1.25, -Mouse::Wheel()), p.x, p.y, r.w, r.h);
	if (r.mouseOver() && MouseM.pressed())
	{
		const auto d = Cursor::DeltaF();
		m_camera.Pan(d.x, d.y, r.w, r.h);
	}
	const int y = static_cast<int>(r.bottomY() + 12);
	if (KeyHome.down() || Rect(24, y, 100, 30).leftClicked())
		m_camera.Fit(m_game.MapWidth(), m_game.MapHeight());
	if (Rect(134, y, 70, 30).leftClicked())
		m_camera.Zoom(1.4, r.w / 2, r.h / 2, r.w, r.h);
	if (Rect(214, y, 70, 30).leftClicked())
		m_camera.Zoom(1 / 1.4, r.w / 2, r.h / 2, r.w, r.h);
	if (m_camera.zoom > 1 && miniRect().mouseOver() && MouseL.pressed())
	{
		const auto n = miniRect();
		const auto q = Cursor::PosF() - n.pos;
		m_camera.x = q.x / n.w * m_game.MapWidth();
		m_camera.y = q.y / n.h * m_game.MapHeight();
		m_camera.Clamp(r.w, r.h);
	}
	if (ox != m_camera.x || oy != m_camera.y || oz != m_camera.zoom)
	{
		m_positions.clear();
		m_hits.clear();
	}
}
