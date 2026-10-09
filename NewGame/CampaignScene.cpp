#include "CampaignScene.hpp"
#include "CampaignSave.hpp"
#include "CampaignUI.hpp"

using namespace frontline;
using namespace campaignui;

CampaignScene::CampaignScene()
{
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
	RectF(0, 0, Scene::Width(), 96).draw(ColorF(.055, .095, .075));
	Line(24, 94, Scene::Width() - 24, 94).draw(1, ColorF(.72, .61, .36, .55));
	RectF(24, 22, 5, 46).draw(ColorF(.77, .62, .31));
	FontAsset(U"campaignTitle")(U"群雄戦線").draw(38, 22, ColorF(.96, .86, .61));
	const String status = m_game.result == 1   ? U"天下統一"
	                      : m_game.result == 2 ? U"勢力滅亡"
	                      : m_daysLeft > 0     ? (m_paused ? U"一時停止" : U"進行中")
	                                           : U"命令期間";
	FontAsset(U"campaignBody")(
	    U"{}軍　{}日経過　{}　命令 {} / 3"_fmt(text(Campaign::FactionName(m_game.player)), m_game.day, status,
	                                           m_game.commands))
	    .draw(225, 37, ColorF(0.81, 0.87, 0.82));
	DrawButton(button(0),
	           m_daysLeft > 0 ? (m_paused ? U"再開 / Space" : U"一時停止 / Space") : U"次の命令期へ / Enter",
	           m_game.result == 0);
	DrawButton(button(15), m_speed == 0 ? U"速度：じっくり ×1" : m_speed == 1 ? U"速度：×2" : U"速度：×5");
	DrawButton(button(16), m_game.hexMap ? U"10日間 同時進行" : U"1日だけ進める",
	           m_daysLeft == 0 && m_game.result == 0);
	DrawButton(button(17), U"武将・評定", !m_game.hexMap || m_daysLeft == 0);
	DrawButton(button(18), U"群雄の記録");
	if (panelVisible())
		drawPanel();
	else
	{
		RectF(Scene::Width() - 330, Scene::Height() - 194, 306, 180)
		    .rounded(3)
		    .draw(ColorF(.06, .10, .085, .94));
		DrawButton(button(10), U"保存", m_daysLeft == 0);
		DrawButton(button(11), U"読込", m_daysLeft == 0);
		DrawButton(button(12), U"勢力選択へ", m_daysLeft == 0);
		(void)FontAsset(U"campaignSmall")(m_message).draw(
		    RectF(Scene::Width() - 316, Scene::Height() - 84, 280, 60), ColorF(.94, .83, .55));
	}
	FontAsset(U"campaignSmall")(m_supplyView
	                                ? U"Tabで切替 / 緑：補給接続　赤：敵の封鎖　橙の印：補給途絶"
	                                : U"左クリック：選択　右クリック：進路　Shift+左：都市　Tab：補給表示")
	    .draw(24, Scene::Height() - 83, ColorF(0.73, 0.81, 0.77));

	const double logY = Scene::Height() - 64;
	RectF(24, logY, Scene::Width() - 390, 54).rounded(5).draw(ColorF(.06, .105, .085));
	const int first = Max(0, static_cast<int>(m_game.log.size()) - 2);
	for (int i = first; i < static_cast<int>(m_game.log.size()); ++i)
		FontAsset(U"campaignSmall")(text(m_game.log[i]))
		    .draw(36, logY + 5 + (i - first) * 22, ColorF(0.84, 0.87, 0.78));
	if (m_game.result != 0 && m_started)
		FontAsset(U"campaignTitle")(m_game.result == 1 ? U"天下統一 — 群雄を制す" : U"敗北 — 再び旗を掲げよ")
		    .drawAt(mapRect().center(), ColorF(0.99, 0.85, 0.44));
	if (!m_started)
		drawMenu();
	else if (m_councilMode != 0)
		drawCouncil();
}
