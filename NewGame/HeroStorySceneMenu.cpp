#include "HeroStoryScene.hpp"

void HeroStoryScene::drawLanding() const
{

	RectF(50, 110, 610, 400).draw(ColorF(.015, .035, .033, .62));
	FontAsset(U"campaignSmall")(U"HERO CHRONICLE  /  第一部・劉備").draw(90, 133, ColorF(.91, .73, .39));
	FontAsset(U"storyDisplay")(U"その背に、\n守るべき人がいる。").draw(86, 171, ColorF(.99, .91, .73));
	(void)FontAsset(U"campaignBody")(U"追手の蹄が迫る。橋の向こうには、仲間と民。\nあなたの決断で、英雄たちの"
	                                 U"志をつなげ。\n会話・内政・護衛・決戦を歩む、全6章の英雄譚。")
	    .draw(RectF(90, 303, 540, 100), ColorF(.89, .88, .78));
	for (int i = 0; i < 6; ++i)
	{
		fx.Portrait(i, RectF(90 + i * 86, 421, 74, 80));
		RectF(90 + i * 86, 421, 74, 80).drawFrame(1, ColorF(.77, .62, .35));
		FontAsset(U"campaignSmall")(T(hero::Name(i))).drawAt(127 + i * 86, 516, ColorF(.93, .84, .63));
	}
	const int cw = (Scene::Width() - 180) / 3;
	for (int i = 0; i < 6; ++i)
	{
		Rect card(90 + (i % 3) * cw, Scene::Height() - 226 + (i / 3) * 88, cw - 14, 76);
		card.draw(ColorF(.035, .065, .064, .9)).drawFrame(1, ColorF(.57, .44, .25));
		FontAsset(U"campaignSmall")(U"第{}章 / {}"_fmt(i + 1, String(hero::Book()[i].year)))
		    .draw(card.x + 16, card.y + 9, ColorF(.72, .65, .43));
		FontAsset(U"campaignBody")(String(hero::Book()[i].title))
		    .draw(card.x + 16, card.y + 32, ColorF(.94, .84, .63));
	}
	Button(B(3), U"新しく英雄譚を始める");
	Button(B(4), U"英雄譚の続きから");
	FontAsset(U"campaignSmall")(U"手動ターン制 / 自動保存 / 会話・分岐・戦場は創作")
	    .draw(90, Scene::Height() - 38, ColorF(.70, .77, .69));
}
