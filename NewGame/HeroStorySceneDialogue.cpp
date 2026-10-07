#include "HeroStoryScene.hpp"

void HeroStoryScene::drawDialogue() const
{
	const auto& c = hero::Book()[s.chapter];
	int stage = s.phase == 3 ? 2 : s.phase;
	if (stage == 1)
	{
		Button(B(0), U"担当：{}　切替"_fmt(s.planner < 0 ? U"未任命" : T(hero::Name(s.planner))));
		if (s.planner >= 0)
			(void)FontAsset(U"campaignSmall")(
			    T(officer::RoleName(officer::RoleOf(hero::Name(s.planner)))) +
			    U"\n糧・守りの段取り：{} / 攻めの段取り：{}"_fmt(s.PlanPower(0), s.PlanPower(1)) +
			    (s.Tactical() ? U"\n開戦1手目は担当者が行動済み。\n準備と初動を考えて任せる。"
			                  : U"\n支度の成果を内政・訪問へ。"))
			    .draw(RectF(Scene::Width() - 305, 214, 270, 110), ColorF(.80, .84, .70));
	}
	int speaker = SpeakerHero(stage);
	RectF portrait(285, 202, 175, 220);
	if (speaker >= 0)
		fx.Portrait(speaker, portrait);
	else
	{
		portrait.draw(ColorF(.08, .12, .10));
		FontAsset(U"storyDisplay")(String(c.speaker[stage])).drawAt(portrait.center(), ColorF(.94, .82, .56));
	}
	portrait.drawFrame(2, ColorF(.76, .63, .36));
	FontAsset(U"campaignTitle")(String(c.speaker[stage])).drawAt(372, 443, ColorF(.98, .87, .62));
	RectF(479, 202, Scene::Width() - 799, 235)
	    .draw(ColorF(.025, .05, .048, .93))
	    .drawFrame(1, ColorF(.51, .44, .28));
	FontAsset(U"campaignSmall")(s.phase == 0   ? U"出会いと決断"
	                            : s.phase == 1 ? U"仲間との準備"
	                                           : U"勝利のあとに")
	    .draw(495, 214, ColorF(.84, .72, .43));
	(void)FontAsset(U"storyDialogue")(String(c.dialogue[stage]))
	    .draw(RectF(495, 251, Scene::Width() - 835, 175), ColorF(.97, .94, .84));
	(void)FontAsset(U"campaignSmall")(U"{}\n{}"_fmt(String(c.basis), String(c.history)))
	    .draw(RectF(285, Scene::Height() - 315, Scene::Width() - 630, 70), ColorF(.65, .76, .69));
	for (int i = 0; i < 2; ++i)
	{
		Button(Choice(i), String(c.choices[stage][i]));
		FontAsset(U"campaignSmall")(
		    stage == 1
		        ? (i == 0 ? s.Tactical() ? U"糧 +{} / 耐久 +{} / 仲間の親密 +8"_fmt(25 + s.PlanPower(0) * 5,
		                                                                            2 + s.PlanPower(0) / 2)
		                                 : U"糧 +{} / 仲間の親密 +8"_fmt(25 + s.PlanPower(0) * 5)
		           : s.Tactical()   ? U"金 +25 / 決意 +6 / 攻撃 +{}"_fmt(1 + (s.PlanPower(1) == 3 ? 1 : 0))
		           : s.chapter == 1 ? U"金 +25 / 決意 +6 / 開始民心 +{}"_fmt(s.PlanPower(1) * 3)
		                            : U"金 +25 / 決意 +6 / 旅糧 +{}"_fmt(s.PlanPower(1) * 5))
		        : (i == 0 ? U"信望 +8 / 金 -10 / この章の仲間の親密 +8"
		                  : U"決意 +8 / 金 +15 / 張飛の親密 +6"))
		    .draw(Choice(i).x + 14, Choice(i).y + 48, ColorF(.66, .77, .67));
	}
}

void HeroStoryScene::drawEnding() const
{

	FontAsset(U"campaignTitle")(String(s.Ending())).draw(285, 230, ColorF(.98, .86, .54));
	(void)FontAsset(U"campaignBody")(
	    U"漢中を得て、劉備は漢中王となった。\n歩んだ道は、あなたの決断と仲間の力がつないだもの。\n益州を領す"
	    U"るまでの歳月は、この第一部では章の間に流れています。\n\n天下への道は、まだ終わらない。\n第一部・完")
	    .draw(RectF(285, 300, Scene::Width() - 635, 280), ColorF(.88, .86, .74));
	Button(B(3), U"英雄譚の選択へ");
}

void HeroStoryScene::drawFailure() const
{
	(void)FontAsset(U"campaignBody")(U"このままでは、旗を守れない。\n仲間と立て直し、この章の任務に再挑戦しよ"
	                                 U"う。\n資源を最低60まで補い、戦場を再配置します。")
	    .draw(RectF(285, 230, Scene::Width() - 630, 160), ColorF(.96, .75, .60));
	Button(B(3), U"任務をやり直す");
}
