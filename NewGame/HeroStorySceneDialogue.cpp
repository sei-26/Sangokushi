#include "HeroStoryScene.hpp"

void HeroStoryScene::drawDialogue() const
{
	const auto& c = hero::Book()[m_story.chapter];
	int stage = m_story.phase == 3 ? 2 : m_story.phase;
	if (stage == 1)
	{
		drawButton(buttonRect(0), U"担当：{}　切替"_fmt(
		                              m_story.planner < 0 ? U"未任命" : text(hero::Name(m_story.planner))));
		if (m_story.planner >= 0)
			(void)FontAsset(U"campaignSmall")(
			    text(officer::RoleName(officer::RoleOf(hero::Name(m_story.planner)))) +
			    U"\n糧・守りの段取り：{} / 攻めの段取り：{}"_fmt(m_story.PlanPower(0), m_story.PlanPower(1)) +
			    (m_story.Tactical() ? U"\n開戦1手目は担当者が行動済み。\n準備と初動を考えて任せる。"
			                        : U"\n支度の成果を内政・訪問へ。"))
			    .draw(RectF(Scene::Width() - 305, 214, 270, 110), ColorF(.80, .84, .70));
	}
	if (stage == 2 && m_story.deepRules)
		(void)FontAsset(U"campaignSmall")(m_story.outcomes[m_story.chapter] == 2
		                                      ? U"約束を達成 / 成果が次の章を支える"
		                                      : U"戦場を勝ち抜いた / 拠点の成果なし")
		    .draw(RectF(Scene::Width() - 305, 150, 270, 80), ColorF(.93, .8, .5));
	if (stage == 0 && m_story.deepRules)
	{
		const String consequence =
		    m_story.chapter == 0   ? U"村の確保：武将を2回復\n達成後、徐州の開始民心+8"
		    : m_story.chapter == 1 ? U"復興の成果：以後の戦場で耐久+1"
		    : m_story.chapter == 2 ? U"対話の成果：以後の開始闘志+10"
		    : m_story.chapter == 3 ? U"4組救出の成果：赤壁の耐久+1"
		    : m_story.chapter == 4
		        ? U"退路を確保して火攻め：威力6\n未確保の火攻め：威力4\n達成後、漢中の耐久+1"
		        : U"高地確保後、2マス内の射撃+1\n黄忠の戦法は損害+2";
		(void)FontAsset(U"campaignSmall")(U"選んだ方針が任務を変える\n\n" + consequence +
		                                  U"\n\n達成条件・会話・効果は創作")
		    .draw(RectF(Scene::Width() - 305, 214, 270, 260), ColorF(.87, .81, .63));
	}
	int speaker = speakerHero(stage);
	RectF portrait(285, 202, 175, 220);
	if (speaker >= 0)
		m_presentation.Portrait(speaker, portrait);
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
	FontAsset(U"campaignSmall")(m_story.phase == 0   ? U"出会いと決断"
	                            : m_story.phase == 1 ? U"仲間との準備"
	                                                 : U"勝利のあとに")
	    .draw(495, 214, ColorF(.84, .72, .43));
	(void)FontAsset(U"storyDialogue")(String(c.dialogue[stage]))
	    .draw(RectF(495, 251, Scene::Width() - 835, 175), ColorF(.97, .94, .84));
	(void)FontAsset(U"campaignSmall")(U"{}\n{}"_fmt(String(c.basis), String(c.history)))
	    .draw(RectF(285, Scene::Height() - 315, Scene::Width() - 630, 70), ColorF(.65, .76, .69));
	for (int i = 0; i < 2; ++i)
	{
		drawButton(choiceRect(i), String(c.choices[stage][i]));
		FontAsset(U"campaignSmall")(
		    stage == 1
		        ? (i == 0 ? m_story.Tactical()
		                        ? U"糧 +{} / 耐久 +{} / 仲間の親密 +8"_fmt(25 + m_story.PlanPower(0) * 5,
		                                                                   2 + m_story.PlanPower(0) / 2)
		                        : U"糧 +{} / 仲間の親密 +8"_fmt(25 + m_story.PlanPower(0) * 5)
		           : m_story.Tactical()
		               ? U"金 +25 / 決意 +6 / 攻撃 +{}"_fmt(1 + (m_story.PlanPower(1) == 3 ? 1 : 0))
		           : m_story.chapter == 1 ? U"金 +25 / 決意 +6 / 開始民心 +{}"_fmt(m_story.PlanPower(1) * 3)
		                                  : U"金 +25 / 決意 +6 / 旅糧 +{}"_fmt(m_story.PlanPower(1) * 5))
		        : (stage == 0 ? text(m_story.ChoiceImpact(i))
		           : i == 0   ? U"信望 +8 / 金 -10 / この章の仲間の親密 +8"
		                      : U"決意 +8 / 金 +15 / 張飛の親密 +6"))
		    .draw(choiceRect(i).x + 14, choiceRect(i).y + 48, ColorF(.66, .77, .67));
	}
}

void HeroStoryScene::drawEnding() const
{

	FontAsset(U"campaignTitle")(String(m_story.Ending())).draw(285, 230, ColorF(.98, .86, .54));
	(void)FontAsset(U"campaignBody")(
	    U"漢中を得て、劉備は漢中王となった。\n歩んだ道は、あなたの決断と仲間の力がつないだもの。\n益州を領す"
	    U"るまでの歳月は、この第一部では章の間に流れています。\n\n天下への道は、まだ終わらない。\n第一部・完")
	    .draw(RectF(285, 300, Scene::Width() - 635, 280), ColorF(.88, .86, .74));
	drawButton(buttonRect(3), U"英雄譚の選択へ");
}

void HeroStoryScene::drawFailure() const
{
	(void)FontAsset(U"campaignBody")(U"このままでは、旗を守れない。\n仲間と立て直し、この章の任務に再挑戦しよ"
	                                 U"う。\n資源を最低60まで補い、戦場を再配置します。")
	    .draw(RectF(285, 230, Scene::Width() - 630, 160), ColorF(.96, .75, .60));
	drawButton(buttonRect(3), U"任務をやり直す");
}
