#include "CampaignScene.hpp"
#include "CampaignSave.hpp"
#include "CampaignUI.hpp"
#include "CampaignVisuals.hpp"

using namespace frontline;
using namespace campaignui;

void CampaignScene::drawMenu() const
{
	Scene::Rect().draw(ColorF(.02, .04, .045));
	m_story.DrawBackdrop(Scene::Rect(), .74);
	Scene::Rect().draw(ColorF(0.02, 0.06, 0.08, 0.58));
	FontAsset(U"campaignTitle")(U"群 雄 戦 線")
	    .drawAt(Scene::Center().x, Scene::Center().y - 240, ColorF(0.95, 0.86, 0.61));
	FontAsset(U"campaignBody")(U"一枚の大陸を制し、天下を統一する。")
	    .drawAt(Scene::Center().x, Scene::Center().y - 190, ColorF(0.76, 0.85, 0.81));
	const Rect story(Scene::Center().x - 465, Scene::Center().y - 145, 930, 86);
	story.rounded(8)
	    .draw(story.mouseOver() ? ColorF(.24, .34, .44) : ColorF(.10, .18, .27))
	    .drawFrame(2, ColorF(.78, .65, .36));
	m_story.DrawBackdrop(story, .48);
	story.draw(ColorF(.02, .05, .04, .45));
	FontAsset(U"campaignTitle")(U"ストーリーモード ― 劉備の旗")
	    .draw(story.x + 24, story.y + 9, ColorF(.96, .85, .59));
	FontAsset(U"campaignSmall")(U"全6章の英雄譚 / 会話・内政・護衛・決戦　　クリックで開く")
	    .draw(story.x + 28, story.y + 57, ColorF(.81, .88, .76));
	FontAsset(U"campaignSmall")(U"君主を選んで大陸統一を目指す")
	    .drawAt(Scene::Center().x, Scene::Center().y - 39, ColorF(.72, .81, .74));
	const Array<String> notes{U"山岳で守る", U"中原を押さえる", U"渡河点を守る"};
	const int start = Scene::Center().x - 465;
	for (int f = 0; f < 3; ++f)
	{
		const Rect card(start + f * 320, Scene::Center().y - 20, 290, 155);
		Panel(card, true);
		RectF(card.x + 1, card.y + 2, 3, card.h - 4).draw(FactionColor(f));
		int ruler = 0, count = 0, cityCount = 0;
		for (int g = 0; g < static_cast<int>(m_game.generals.size()); ++g)
		{
			if (m_game.generals[g].name == Campaign::FactionName(f))
				ruler = g;
			if (m_game.generals[g].faction == f)
				++count;
		}
		for (const auto& city : m_game.cities)
			if (city.owner == f)
				++cityCount;
		campaignvisual::OfficerCard(RectF(card.x + 18, card.y + 18, 66, 80), m_game.generals[ruler],
		                            m_campaignFaces);
		FontAsset(U"campaignBody")(text(Campaign::FactionName(f)))
		    .draw(card.x + 102, card.y + 18, ColorF(.98, .91, .74));
		FontAsset(U"campaignSmall")(U"都市 {} / 武将 {}"_fmt(cityCount, count))
		    .draw(card.x + 102, card.y + 51, ColorF(.79, .85, .87));
		(void)FontAsset(U"campaignSmall")(notes[f]).draw(RectF(card.x + 102, card.y + 78, 172, 40),
		                                                 ColorF(.87, .89, .83));
		RectF(card.x + 14, card.bottomY() - 33, card.w - 28, 25).draw(ColorF(.08, .14, .23));
		FontAsset(U"campaignSmall")(U"この勢力で開始")
		    .drawAt(Vec2(card.center().x, card.bottomY() - 20), ColorF(.96, .90, .75));
	}
	DrawButton(Rect(Scene::Center().x - 225, Scene::Center().y + 165, 450, 38), U"続きから");
	FontAsset(U"campaignSmall")(U"自由戦略 / 君主を選び、大陸統一を目指す")
	    .drawAt(Scene::Center().x, Scene::Center().y + 232, ColorF(0.64, 0.73, 0.71));
	if (m_message.starts_with(U"新作のセーブ"))
		FontAsset(U"campaignSmall")(m_message).drawAt(Scene::Center().x, Scene::Center().y + 260,
		                                              ColorF(0.98, 0.78, 0.48));
}
