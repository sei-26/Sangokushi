#include "HeroStoryScene.hpp"

void HeroStoryScene::drawCivilActions() const
{

	const bool xu = s.chapter == 1;
	FontAsset(U"campaignTitle")(xu ? U"戦火の徐州を立て直す" : U"隆中の門をたたく")
	    .draw(285, 215, ColorF(.92, .84, .64));
	(void)FontAsset(U"campaignBody")(
	    xu ? U"3回以上働き、6回以内に民心60以上にする。\n施しだけでなく、市を開く判断も必要だ。"
	       : U"訪問を3回重ね、諸葛亮を迎える。\n糧を用意し、待つ仲間の気持ちにも心を配ろう。")
	    .draw(RectF(285, 280, Scene::Width() - 630, 120), ColorF(.83, .87, .77));
	FontAsset(U"campaignTitle")(xu ? U"民心 {} / 100　施策 {} / 6"_fmt(s.order, s.tasks)
	                               : U"訪問 {} / 3　関羽 {}　張飛 {}"_fmt(s.visits, s.bonds[1], s.bonds[2]))
	    .draw(285, 430, ColorF(.95, .81, .52));
	Button(B(0), xu ? U"糧を配る / 糧25" : U"訪ねる / 糧20", s.food >= (xu ? 25 : 20));
	Button(B(1), xu ? U"市を開く / 金+30" : U"旅支度 / 金10", true);
	Button(B(2), xu ? U"郷勇を鍛える / 金20" : U"仲間と語る", xu ? s.gold >= 20 : true);
}
