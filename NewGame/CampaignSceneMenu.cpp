#include "CampaignScene.hpp"
#include "CampaignSave.hpp"
#include "CampaignUI.hpp"

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
	    .draw(story.mouseOver() ? ColorF(.24, .34, .26) : ColorF(.13, .23, .20))
	    .drawFrame(2, ColorF(.78, .65, .36));
	m_story.DrawBackdrop(story, .48);
	story.draw(ColorF(.02, .05, .04, .45));
	FontAsset(U"campaignTitle")(U"新戦略　六角マスの領土と兵站")
	    .draw(story.x + 24, story.y + 9, ColorF(.96, .85, .59));
	FontAsset(U"campaignSmall")(U"HEXの一枚マップ / 六方向の進軍・包囲・補給　　下の勢力を選択")
	    .draw(story.x + 28, story.y + 57, ColorF(.81, .88, .76));
	FontAsset(U"campaignSmall")(U"君主を選んで大陸統一を目指す")
	    .drawAt(Scene::Center().x, Scene::Center().y - 39, ColorF(.72, .81, .74));
	const Array<String> notes{U"西方から進む。山と隘路を味方に。", U"中央を押さえる。多方面への備えを。",
	                          U"江南を守る。渡河点をめぐる戦い。"};
	const int start = Scene::Center().x - 465;
	for (int f = 0; f < 3; ++f)
	{
		const Rect card(start + f * 320, Scene::Center().y - 20, 290, 155);
		card.rounded(8)
		    .draw(ColorF(FactionColor(f), card.mouseOver() ? 0.4 : 0.18))
		    .drawFrame(2, FactionColor(f));
		FontAsset(U"campaignTitle")(text(Campaign::FactionName(f)))
		    .drawAt(card.center().movedBy(0, -30), FactionColor(f));
		FontAsset(U"campaignSmall")(notes[f]).drawAt(card.center().movedBy(0, 28), ColorF(0.91, 0.92, 0.84));
		FontAsset(U"campaignBody")(U"この勢力で始める")
		    .drawAt(card.center().movedBy(0, 58), ColorF(0.95, 0.86, 0.61));
	}
	DrawButton(Rect(Scene::Center().x - 225, Scene::Center().y + 165, 450, 38), U"続きから");
	FontAsset(U"campaignSmall")(U"新作の戦略試作版 / 史実の領土・年代を再現するシナリオではありません")
	    .drawAt(Scene::Center().x, Scene::Center().y + 232, ColorF(0.64, 0.73, 0.71));
	if (m_message.starts_with(U"新作のセーブ"))
		FontAsset(U"campaignSmall")(m_message).drawAt(Scene::Center().x, Scene::Center().y + 260,
		                                              ColorF(0.98, 0.78, 0.48));
}
