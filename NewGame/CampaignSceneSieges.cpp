#include "CampaignScene.hpp"
#include "SiegeStatus.hpp"
void CampaignScene::drawSieges(double cell) const
{
	if (!m_started || cell < 15)
		return;
	const double right = panelVisible() ? Scene::Width() - 342. : Scene::Width() - 20.;
	for (int city = 0; city < static_cast<int>(m_game.cities.size()); ++city)
	{
		const auto status = siegestatus::Of(m_game, city);
		if (status.armies == 0)
			continue;
		const auto& c = m_game.cities[city];
		const Vec2 p = tileCenter(c.tile);
		if (p.x < 0 || p.x > right || p.y < 100 || p.y > Scene::Height() - 160)
			continue;
		const auto neighbors = m_game.MapNeighbors(c.tile);
		for (size_t k = 0; k < neighbors.size(); ++k)
		{
			const Vec2 n = tileCenter(neighbors[k]);
			const Vec2 d = (n - p).normalized();
			const Vec2 a = p + d * (cell * .8), b = p + d * (cell * 1.18);
			Line(a, b).draw(status.occupied[k] ? 5 : 2,
			                status.occupied[k] ? ColorF(1., .42, .17, .85) : ColorF(.86, .79, .56, .28));
		}
		const double width = 320, x = Clamp(p.x - width * .5, 22., Max(22., right - width)),
		             y = p.y + Max(45., cell * .86);
		const RectF plate(x, y, width, 112);
		plate.rounded(4).draw(ColorF(.025, .055, .06, .95)).drawFrame(1, ColorF(.91, .61, .29));
		FontAsset(U"campaignBody")(U"{} / 守備兵 {}"_fmt(text(c.name), c.troops))
		    .draw(x + 10, y + 7, ColorF(.99, .88, .66));
		FontAsset(U"campaignSmall")(
		    U"攻城配置 {}隊・{}兵 / 攻城隊 {}"_fmt(status.armies, status.troops, status.engines))
		    .draw(x + 10, y + 36, ColorF(.87, .90, .82));
		FontAsset(U"campaignSmall")(U"最大包囲 {}方向 / 攻城補正 +{}%"_fmt(status.fronts, status.bonus))
		    .draw(x + 10, y + 59, ColorF(.98, .74, .44));
		FontAsset(U"campaignSmall")(status.unsupplied > 0 ? U"攻撃側 {}隊の補給が途絶"_fmt(status.unsupplied)
		                                                  : U"攻撃側の補給：接続")
		    .draw(x + 10, y + 82, status.unsupplied > 0 ? ColorF(1., .52, .30) : ColorF(.63, .86, .70));
	}
}
