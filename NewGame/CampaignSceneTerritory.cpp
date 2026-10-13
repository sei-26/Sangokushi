#include "CampaignScene.hpp"
#include "CampaignUI.hpp"
RectF CampaignScene::territoryLegendRect() const
{
	return RectF(Scene::Width() - 246, Scene::Height() - 346, 224, 70);
}
void CampaignScene::drawTerritoryLegend() const
{
	const auto r = territoryLegendRect();
	campaignui::Panel(r);
	for (int f = 0; f < 3; ++f)
	{
		const double x = r.x + 10 + f * 70;
		RectF(x, r.y + 12, 8, 12).draw(campaignui::FactionColor(f));
		FontAsset(U"campaignSmall")(text(frontline::Campaign::FactionName(f)))
		    .draw(x + 13, r.y + 8, ColorF(.94, .95, .90));
	}
	campaignui::DrawButton(Rect(static_cast<int>(r.x + 12), static_cast<int>(r.bottomY() - 31), 200, 24),
	                       m_supplyView      ? U"補給表示 / Tab"
	                       : m_territoryView ? U"領土色 / T"
	                                         : U"自然地形 / T");
}
