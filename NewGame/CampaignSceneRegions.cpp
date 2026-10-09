#include "CampaignScene.hpp"
#include "CampaignUI.hpp"
#include "CampaignVisuals.hpp"

using namespace frontline;
using namespace campaignui;

void CampaignScene::drawRegions(double cell) const
{
	if (!m_game.hexMap)
		return;
	if (m_region >= 0 && m_region < static_cast<int>(m_game.regions.size()))
		for (int p = 0; p < TileCount; ++p)
			if (m_game.tileRegion[p] == m_region)
			{
				const auto shape = campaignvisual::TileShape(tileCenter(p), cell, true, m_camera.Pitch());
				shape.draw(ColorF(.95, .81, .36, .09));
				bool boundary = false;
				for (int n : m_game.MapNeighbors(p))
					if (m_game.tileRegion[n] != m_region)
						boundary = true;
				if (boundary)
					shape.drawFrame(1, ColorF(.95, .81, .36, .6));
			}
	for (int i = 0; i < static_cast<int>(m_game.regions.size()); ++i)
	{
		const auto& r = m_game.regions[i];
		if (m_game.CityAt(r.tile) >= 0)
			continue;
		const Vec2 p = tileCenter(r.tile);
		const double size = Max(3.0, cell * .22);
		if (!m_mapArt.Town(p, cell, r.owner))
		{
			Ellipse(p.movedBy(size * .4, size * .7), size * 1.7, size * .6).draw(ColorF(.03, .06, .04, .35));
			RectF(p.x - size, p.y - size * .4, size * 2, size * 1.2).draw(ColorF(.74, .68, .46));
			RectF(p.x, p.y - size * .4, size, size * 1.2).draw(ColorF(.41, .47, .33));
			Triangle(p + Vec2(-size * 1.5, -size * .3), p + Vec2(0, -size * 1.5),
			         p + Vec2(size * 1.5, -size * .3))
			    .draw(ColorF(.19, .32, .27));
			RectF(p.x - size * .65, p.y - size * .6, size * 1.3, size * .35).draw(FactionColor(r.owner));
		}
		if (m_camera.zoom >= 3.2 || i == m_region || mouseTile() == r.tile)
			campaignvisual::MapLabel(p + Vec2(0, Max(16., cell * .30)), text(r.name), r.owner, i == m_region,
			                         true);
	}
}

void CampaignScene::drawRegionPanel() const
{
	const int x = Scene::Width() - 316;
	const auto& r = m_game.regions[m_region];
	const auto line = [&](const String& value, int y) {
		FontAsset(U"campaignBody")(value).draw(x, y, ColorF(.88, .90, .80));
	};
	line(text(r.name) + U" / 地域", 110);
	line(U"支配：" + text(Campaign::FactionName(r.owner)), 154);
	line(U"所属都市：" + (r.city >= 0 ? text(m_game.cities[r.city].name) : U"なし"), 193);
	line(U"領土の占有率：{}％"_fmt(m_game.RegionCoverage(m_region)), 232);
	const bool connected = m_game.RegionConnected(m_region);
	line(connected ? U"都市への補給路：接続" : U"都市への補給路：途絶", 271);
	int gold = 0, food = 0;
	if (connected && r.city >= 0 && m_game.CityAt(r.tile) < 0)
	{
		const auto& c = m_game.cities[r.city];
		const int control = m_game.RegionCoverage(m_region);
		gold = (80 + c.commerce * 4) * control * c.order / 10000;
		food = (400 + c.farming * 15) * control * c.order / 10000;
	}
	line(U"月末追加収入：金 {} / 糧 {}"_fmt(gold, food), 310);
	(void)FontAsset(U"campaignSmall")(
	    U"府に兵1000以上・士気30以上の戦闘部隊を進めると占領。府に留まると5日ごとに周辺3マスを支配します。\n"
	    U"\n"
	    U"府の追加収入は占有率と所属都市の農政・商業・治安で増加。補給が途絶すると収入も止まります。")
	    .draw(RectF(x, 365, 278, 230), ColorF(.76, .81, .72));
}
