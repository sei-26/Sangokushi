#include "CampaignScene.hpp"
#include "CampaignSave.hpp"
#include "CampaignUI.hpp"
#include "CampaignVisuals.hpp"
using namespace frontline;
using namespace campaignui;

void CampaignScene::drawHUD() const
{
	if (!m_started)
		return;
	Panel(RectF(0, 0, Scene::Width(), 76));
	int ruler = 0, gold = 0, food = 0, cities = 0, troops = 0;
	for (int g = 0; g < static_cast<int>(m_game.generals.size()); ++g)
		if (m_game.generals[g].name == Campaign::FactionName(m_game.player))
		{
			ruler = g;
			break;
		}
	for (const auto& city : m_game.cities)
		if (city.owner == m_game.player)
		{
			gold += city.gold;
			food += city.food;
			troops += city.troops;
			++cities;
		}
	for (const auto& army : m_game.armies)
		if (army.faction == m_game.player && army.troops > 0)
			troops += army.troops;
	campaignvisual::OfficerCard(RectF(14, 8, 48, 58), m_game.generals[ruler], m_campaignFaces);
	RectF(67, 10, 3, 52).draw(FactionColor(m_game.player));
	FontAsset(U"campaignBody")(text(Campaign::FactionName(m_game.player)) + U"軍")
	    .draw(82, 8, ColorF(.98, .91, .73));
	const String periods[]{U"上旬", U"中旬", U"下旬"};
	FontAsset(U"campaignSmall")(
	    U"{}年 {}月 {}"_fmt(m_game.day / 360 + 1, (m_game.day / 30) % 12 + 1, periods[(m_game.day / 10) % 3]))
	    .draw(82, 37, ColorF(.86, .89, .87));
	const auto metric = [](int x, const String& label, const String& value) {
		FontAsset(U"campaignSmall")(label).draw(x, 10, ColorF(.68, .76, .80));
		FontAsset(U"campaignBody")(value).draw(x, 35, ColorF(.98, .95, .84));
	};
	metric(270, U"命令書", U"{} / {}"_fmt(m_game.commands, m_game.commandBudget));
	metric(400, U"金", Format(gold));
	metric(528, U"兵糧", Format(food));
	metric(674, U"都市 / 総兵力", U"{} / {}"_fmt(cities, troops));
	const String status = m_game.result == 1   ? U"天下統一"
	                      : m_game.result == 2 ? U"勢力滅亡"
	                      : m_daysLeft > 0     ? (m_paused ? U"一時停止" : U"進行中")
	                                           : U"戦略フェイズ";
	FontAsset(U"campaignSmall")(status).draw(Scene::Width() - 362, 12, ColorF(.92, .85, .61));
	DrawButton(button(15), m_speed == 0 ? U"速度 ×1" : m_speed == 1 ? U"速度 ×2" : U"速度 ×5");
	DrawButton(button(0), m_daysLeft > 0 ? (m_paused ? U"▶ 再開" : U"Ⅱ 一時停止") : U"▶ 進行開始",
	           m_game.result == 0);
	if (panelVisible())
		drawPanel();
	Panel(RectF(0, Scene::Height() - 58, Scene::Width(), 58));
	DrawButton(button(50), U"内政");
	DrawButton(button(17), U"任免", m_daysLeft == 0);
	DrawButton(button(51), U"外交・計略", m_daysLeft == 0);
	DrawButton(button(18), U"情報");
	DrawButton(button(52), U"設定");
	DrawButton(button(56), U"施政", m_daysLeft == 0);
	DrawButton(button(57), m_supplyView ? U"補給 ON" : U"補給");
	if (m_game.hexMap)
		DrawButton(button(58), m_atlasView ? U"戦場図 / F" : U"全国図 / F");
	if (m_game.hexMap)
		DrawButton(button(59), m_gridVisible ? U"マス ON / H" : U"マス OFF / H");
	if (!m_game.hexMap)
		DrawButton(button(16), U"1日進行", m_daysLeft == 0);
	FontAsset(U"campaignSmall")(U"群雄戦線 / {}日経過"_fmt(m_game.day))
	    .draw(Scene::Width() - 244, Scene::Height() - 40, ColorF(.73, .79, .80));
	if (m_atlasView)
	{
		const RectF log(Scene::Width() - 246, 94, 224, 172);
		Panel(log);
		FontAsset(U"campaignSmall")(m_daysLeft > 0 ? U"戦況報告" : U"全国概況・助言")
		    .draw(log.x + 10, log.y + 8, ColorF(.86, .82, .65));
		(void)FontAsset(U"campaignSmall")(m_message).draw(RectF(log.x + 10, log.y + 36, 204, 70),
		                                                  ColorF(.91, .93, .90));
		if (!m_game.log.empty())
			(void)FontAsset(U"campaignSmall")(text(m_game.log.back()))
			    .draw(RectF(log.x + 10, log.y + 111, 204, 50), ColorF(.74, .82, .86));
	}
	else
	{
		const double logX = panelVisible() ? 340 : 20;
		const RectF log(logX, Scene::Height() - 142, Scene::Width() - logX - 270, 70);
		Panel(log);
		FontAsset(U"campaignSmall")(m_daysLeft > 0 ? U"戦況報告" : U"作戦・助言")
		    .draw(log.x + 10, log.y + 6, ColorF(.86, .82, .65));
		(void)FontAsset(U"campaignSmall")(m_message).draw(RectF(log.x + 104, log.y + 6, log.w - 116, 25),
		                                                  ColorF(.91, .93, .90));
		if (!m_game.log.empty())
			(void)FontAsset(U"campaignSmall")(text(m_game.log.back()))
			    .draw(RectF(log.x + 10, log.y + 37, log.w - 20, 25), ColorF(.74, .82, .86));
	}
	DrawButton(button(49), m_panelHidden ? U"情報表示" : U"情報隠す");
	DrawButton(button(53), U"全図");
	DrawButton(button(54), U"＋");
	DrawButton(button(55), U"−");
	if (m_settingsOpen)
	{
		Panel(RectF(Scene::Width() - 310, 82, 292, 226), true);
		FontAsset(U"campaignBody")(U"設定 / Escで閉じる")
		    .draw(Scene::Width() - 296, 94, ColorF(.97, .91, .75));
		m_musicPlayer.Draw(Vec2(Scene::Width() - 296, 134), 26);
		DrawButton(button(10), U"保存", m_daysLeft == 0);
		DrawButton(button(11), U"読込", m_daysLeft == 0);
		DrawButton(button(12), U"勢力選択へ", m_daysLeft == 0);
		DrawButton(Rect(Scene::Width() - 296, 266, 126, 24),
		           m_presentation.SoundEnabled() ? U"効果音 ON" : U"効果音 OFF");
		DrawButton(Rect(Scene::Width() - 162, 266, 126, 24),
		           m_spectacle.Enabled() ? U"演出 ON" : U"演出 OFF");
	}
}

void CampaignScene::updateSettings()
{
	if (KeyEscape.down() || button(52).leftClicked())
	{
		m_settingsOpen = false;
		return;
	}
	if (Rect(Scene::Width() - 162, 266, 126, 24).leftClicked())
		m_spectacle.Toggle();
	if (m_daysLeft != 0)
		return;
	if (button(12).leftClicked())
	{
		m_started = false;
		m_settingsOpen = false;
		m_spectacle.Reset();
		m_army = -1;
		m_city = -1;
		return;
	}
	if (button(10).leftClicked())
	{
		try
		{
			m_message = SaveJSON(m_game).save(U"frontline-save.json") ? U"戦況を保存しました。"
			                                                          : U"保存できませんでした。";
		}
		catch (...)
		{
			m_message = U"保存できませんでした。";
		}
	}
	if (button(11).leftClicked())
	{
		Campaign loaded;
		if (LoadJSON(JSON::Load(U"frontline-save.json"), loaded))
		{
			m_game = std::move(loaded);
			m_region = -1;
			m_transportEstimateDay = -1;
			fitMap();
			m_city = -1;
			m_army = -1;
			m_positions.clear();
			m_hits.clear();
			m_presentation.Reset();
			m_spectacle.Reset();
			m_mapDay = -1;
			m_mapTexture = RenderTexture{};
			m_message = U"戦況を復元しました。";
			m_atlasView = m_game.hexMap;
			m_settingsOpen = false;
		}
		else
			m_message = U"有効なセーブが見つかりません。";
	}
}
