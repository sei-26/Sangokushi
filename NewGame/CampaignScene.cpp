#include "CampaignScene.hpp"
#include "CampaignSave.hpp"
#include "CampaignUI.hpp"

using namespace frontline;
using namespace campaignui;

CampaignScene::CampaignScene()
{
	if(FileSystem::Exists(U"MapArt/china-atlas-v1.png")) {m_atlasSource=Image(U"MapArt/china-atlas-v1.png");m_atlasTexture=Texture(m_atlasSource,TextureDesc::Mipped);}
	m_game.Reset(0);
	fitMap();
}

void CampaignScene::draw() const
{
	if (m_storyActive)
	{
		m_story.draw();
		return;
	}
	Scene::SetBackground(ColorF(.035, .060, .048));
	drawMap();
	drawHUD();
	if (m_game.result != 0 && m_started)
		FontAsset(U"campaignTitle")(m_game.result == 1 ? U"天下統一 — 群雄を制す" : U"敗北 — 再び旗を掲げよ")
		    .drawAt(mapRect().center(), ColorF(0.99, 0.85, 0.44));
	if (!m_started)
		drawMenu();
	else if (m_councilMode != 0)
		drawCouncil();
	if (m_infoOpen)
		drawInformation();

	if (m_started && m_councilMode == 0 && !m_infoOpen)
	{
		if (!m_presentation.Busy() && !m_settingsOpen)
			m_spectacle.DrawReports(m_campaignFaces);
		m_presentation.DrawOverlay(m_campaignFaces);
	}
}
