#include "HeroStoryScene.hpp"

void HeroStoryScene::drawCivilActions() const
{

	const bool xu = m_story.chapter == 1;
	FontAsset(U"campaignTitle")(xu ? U"戦火の徐州を立て直す" : U"隆中の門をたたく")
	    .draw(285, 215, ColorF(.92, .84, .64));
	(void)FontAsset(U"campaignBody")(
	    m_story.deepRules ? text(m_story.ObjectiveHelp())
	    : xu ? U"3回以上働き、6回以内に民心60以上にする。\n施しだけでなく、市を開く判断も必要だ。"
	         : U"訪問を3回重ね、諸葛亮を迎える。\n糧を用意し、待つ仲間の気持ちにも心を配ろう。")
	    .draw(RectF(285, 280, Scene::Width() - 630, 120), ColorF(.83, .87, .77));
	FontAsset(U"campaignTitle")(
	    xu ? U"民心 {} / 100　施策 {} / 6"_fmt(m_story.order, m_story.tasks)
	       : U"訪問 {} / 3　関羽 {}　張飛 {}"_fmt(m_story.visits, m_story.bonds[1], m_story.bonds[2]))
	    .draw(285, 430, ColorF(.95, .81, .52));
	drawButton(buttonRect(0), xu ? U"糧を配る / 糧25" : U"訪ねる / 糧20",
	           m_story.food >= (xu ? 25 : 20) &&
	               (xu || !m_story.deepRules || m_story.civilActions[2] >= m_story.visits));
	if (m_story.deepRules)
		(void)FontAsset(U"campaignSmall")(
		    xu ? U"救済 {} / 商い {} / 訓練 {}"_fmt(m_story.civilActions[0], m_story.civilActions[1],
		                                            m_story.civilActions[2])
		       : U"仲間との対話 {} / 次の訪問前に {} 回必要"_fmt(m_story.civilActions[2], m_story.visits))
		    .draw(RectF(285, 495, Scene::Width() - 630, 60), ColorF(.78, .85, .72));
	drawButton(buttonRect(1), xu ? U"市を開く / 金+30" : U"旅支度 / 金10", true);
	drawButton(buttonRect(2), xu ? U"郷勇を鍛える / 金20" : U"仲間と語る", xu ? m_story.gold >= 20 : true);
}
