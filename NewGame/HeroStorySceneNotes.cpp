#include "HeroStoryScene.hpp"

void HeroStoryScene::drawNotes() const
{
	const auto& c = hero::Book()[s.chapter];

	Button(B(6), notePage == 0   ? U"仲間の人物像へ"
	             : notePage == 1 ? U"選択の記録へ"
	             : notePage == 2 ? U"仲間同士の関係へ"
	                             : U"この章の史話へ");
	if (notePage == 0)
	{
		(void)FontAsset(U"campaignBody")(String(c.history))
		    .draw(RectF(285, 200, Scene::Width() - 630, 110), ColorF(.92, .86, .70));
		(void)FontAsset(U"campaignSmall")(
		    U"{}\n出典：{}\n会話・選択・任務・地形・戦法は創作。年代間は章ごとに進みます。"_fmt(
		        String(c.basis), String(c.source)))
		    .draw(RectF(285, 318, Scene::Width() - 630, 100), ColorF(.70, .79, .74));
		(void)FontAsset(U"campaignBody")(U"史実の結末を読むだけでなく、その道中で何を大切にするかを選びます。"
		                                 U"\n桃園の誓いは演義の物語。赤壁の火攻めは周瑜・黄蓋らの働き。\n劉備"
		                                 U"の視点から仲間と連携する、創作の英雄譚として描いています。")
		    .draw(RectF(285, 455, Scene::Width() - 630, 170), ColorF(.83, .85, .75));
	}
	else if (notePage == 1)
	{
		const Array<String> bio{U"人を迎え、仲間と民をつなぐ。信望は兵の耐久に反映。",
		                        U"張飛とともに早くから劉備に仕える。近接の切り札。",
		                        U"長坂で少数の騎兵と後衛を担う。敵を止める戦法。",
		                        U"長坂で幼い劉禅らを守る。味方と民を回復できる。",
		                        U"劉備が三度訪れた人材。仲間を再行動させる。",
		                        U"定軍山で夏侯淵を討つ。遠方へ一射を放つ。"};
		for (int i = 0; i < 6; ++i)
		{
			FontAsset(U"campaignBody")(
			    U"{}　{} {}　{}"_fmt(String(hero::Name(i)), i == 0 ? U"信望" : U"劉備への親密",
			                         i == 0 ? s.virtue : s.Affinity(0, i), String(hero::SkillName(i))))
			    .draw(285, 200 + i * 78, ColorF(.94, .84, .61));
			(void)FontAsset(U"campaignSmall")(bio[i]).draw(RectF(285, 233 + i * 78, Scene::Width() - 635, 44),
			                                               ColorF(.72, .82, .72));
		}
		FontAsset(U"campaignSmall")(U"人物像の参照：三国志 巻32・35・36 / 戦法の効果は創作")
		    .draw(285, Scene::Height() - 95, ColorF(.65, .74, .65));
	}
	else if (notePage == 2)
	{
		FontAsset(U"campaignBody")(U"あなたと仲間が歩んだ道（最近の記録）")
		    .draw(285, 200, ColorF(.94, .84, .61));
		int first = Max(0, static_cast<int>(s.journal.size()) - 8);
		for (int i = first; i < static_cast<int>(s.journal.size()); ++i)
			(void)FontAsset(U"campaignSmall")(T(s.journal[i]))
			    .draw(RectF(285, 245 + (i - first) * 54, Scene::Width() - 635, 50), ColorF(.77, .85, .74));
	}
	else
	{
		FontAsset(U"campaignBody")(U"仲間同士の親密度 ― 役割を組み合わせる")
		    .draw(285, 202, ColorF(.94, .84, .61));
		(void)FontAsset(U"campaignSmall")(
		    U"親密40 / 60 / "
		    U"80で連携が強化。2マス内の生存する仲間が必要。\n突破・射撃・軍師は攻撃、守護は被害軽減、旗頭は闘"
		    U"志を支える。\n同じ役割の連携は半減。各効果の最大値を使い、人数だけでは重ならない。")
		    .draw(RectF(285, 240, Scene::Width() - 635, 80), ColorF(.72, .82, .72));
		int row = 0;
		for (int a = 0; a < s.Unlocked(); ++a)
			for (int b = a + 1; b < s.Unlocked(); ++b)
			{
				FontAsset(U"campaignSmall")(
				    U"{} × {}　親密 {}"_fmt(T(hero::Name(a)), T(hero::Name(b)), s.Affinity(a, b)))
				    .draw(285, 340 + row * 23, ColorF(.86, .81, .63));
				++row;
			}
	}
}
