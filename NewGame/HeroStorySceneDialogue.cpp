#include "HeroStoryScene.hpp"
#include "StoryDrama.hpp"
void HeroStoryScene::drawDialogue() const
{
	const auto& c = hero::Book()[m_story.chapter];
	const int stage = m_story.phase == 3 ? 2 : m_story.phase;
	const auto beat = m_dialogueBeat < 2 ? hero::drama::Conversation(m_story.chapter, stage, m_dialogueBeat)
	                                     : hero::drama::Beat{c.speaker[stage], c.dialogue[stage]};
	const double w = Scene::Width(), h = Scene::Height();
	m_presentation.Landscape(Scene::Rect(), 1);
	Scene::Rect().draw(ColorF(.015, .027, .03, .24));
	m_presentation.Atmosphere();
	RectF(0, 0, w, 130).draw(ColorF(.01, .025, .025, .83));
	FontAsset(U"campaignSmall")(U"{} / {} / 会話は創作"_fmt(String(c.year), String(c.place)))
	    .draw(64, 25, ColorF(.85, .76, .53));
	FontAsset(U"storyDisplay")(U"第{}章  {}"_fmt(m_story.chapter + 1, String(c.title)))
	    .draw(36, 64, 58, ColorF(.99, .9, .70));
	const RectF portrait(64, h - 450, 230, 300);
	m_presentation.PortraitName(String(beat.speaker), portrait);
	portrait.drawFrame(2, ColorF(.87, .70, .36));
	RectF panel(326, h - 344, w - 390, 200);
	panel.draw(ColorF(.025, .045, .048, .94)).drawFrame(2, ColorF(.64, .51, .29));
	FontAsset(U"campaignTitle")(String(beat.speaker)).draw(346, h - 326, ColorF(.98, .84, .49));
	const String line(beat.line);
	const size_t count = Min(line.size(), static_cast<size_t>(Max(0., m_dialogueAge) * 36));
	(void)FontAsset(U"storyDialogue")(line.substr(0, count))
	    .draw(RectF(346, h - 278, w - 430, 120), ColorF(.98, .96, .88));
	FontAsset(U"campaignSmall")(U"{} / 3　{}"_fmt(m_dialogueBeat + 1, m_dialogueBeat < 2
	                                                                      ? U"クリック・Spaceで次の会話"
	                                                                      : U"あなたの決断を選ぶ"))
	    .draw(346, h - 176, ColorF(.75, .82, .76));
	if (stage == 2)
	{
		const String outcome = m_story.outcomes[m_story.chapter] == 2
		                           ? U"約束を達成 / この章の成果が次の歩みを支える"
		                           : U"任務を完遂 / 勝利を、この先へつなぐ";
		RectF(64, 146, w - 128, 40).draw(ColorF(.04, .10, .08, .9)).drawFrame(1, ColorF(.69, .57, .30));
		FontAsset(U"campaignBody")(outcome).draw(80, 153, ColorF(.96, .87, .60));
	}
	if (stage == 1)
	{
		drawButton(buttonRect(0), U"準備を任せる：{} / 切替"_fmt(
		                              m_story.planner < 0 ? U"未任命" : text(hero::Name(m_story.planner))));
		const String plan =
		    U"守りの段取り {} / 攻めの段取り {}"_fmt(m_story.PlanPower(0), m_story.PlanPower(1)) +
		    (m_story.Tactical() ? U" / 担当者は初手を準備に使用" : U" / 支度の成果を内政・訪問へ");
		(void)FontAsset(U"campaignSmall")(plan).draw(RectF(w - 305, 211, 275, 60), ColorF(.96, .91, .76));
	}
	for (int i = 0; i < 2; ++i)
	{
		const auto r = choiceRect(i);
		drawButton(r, String(c.choices[stage][i]), m_dialogueBeat == 2);
		const String impact =
		    stage == 0 ? text(m_story.ChoiceImpact(i))
		    : stage == 1
		        ? (i == 0 ? (m_story.Tactical()
		                         ? U"糧 +{} / 耐久 +{} / 親密 +8"_fmt(25 + m_story.PlanPower(0) * 5,
		                                                              2 + m_story.PlanPower(0) / 2)
		                         : U"糧 +{} / 親密 +8"_fmt(25 + m_story.PlanPower(0) * 5))
		           : m_story.Tactical()
		               ? U"金 +25 / 決意 +6 / 攻撃 +{}"_fmt(1 + (m_story.PlanPower(1) == 3 ? 1 : 0))
		           : m_story.chapter == 1 ? U"金 +25 / 決意 +6 / 開始民心 +{}"_fmt(m_story.PlanPower(1) * 3)
		                                  : U"金 +25 / 決意 +6 / 旅糧 +{}"_fmt(m_story.PlanPower(1) * 5))
		    : i == 0 ? U"信望 +8 / 金 -10 / 仲間の親密 +8"
		             : U"決意 +8 / 金 +15 / 張飛の親密 +6";
		(void)FontAsset(U"campaignSmall")(impact).draw(RectF(r.x + 16, r.y + 48, r.w - 32, 24),
		                                               ColorF(.75, .83, .72));
	}
	FontAsset(U"campaignSmall")(U"信望 {}　決意 {}　糧 {}　金 {}　/　史実の背景は「史話・仲間の記録」へ"_fmt(
	                                m_story.virtue, m_story.resolve, m_story.food, m_story.gold))
	    .draw(64, h - 26, ColorF(.89, .86, .74));
}

void HeroStoryScene::drawEnding() const
{
	const double w = Scene::Width(), h = Scene::Height();
	m_presentation.Landscape(Scene::Rect(), 1);
	Scene::Rect().draw(ColorF(.015, .027, .028, .50));
	FontAsset(U"campaignSmall")(U"英雄譚 / 劉備の旗 / 第一部・完").draw(64, 40, ColorF(.95, .79, .45));
	FontAsset(U"storyDisplay")(String(m_story.Ending())).draw(56, 64, 120, ColorF(1., .93, .74));
	(void)FontAsset(U"storyDialogue")(
	    U"旗を掲げた日の誓いを、ここまで運んだのは、一人の英雄ではない。\n待つ者、支える者、救う者、最後の一"
	    U"射を放つ者。\nあなたの決断と、仲間たちの働きが、この道をつないだ。")
	    .draw(RectF(64, 220, w - 128, 110), ColorF(.97, .95, .86));
	FontAsset(U"campaignBody")(
	    U"信望 {} / 決意 {} / 決断 {}回 / 戦場の選択 {}回"_fmt(
	        m_story.virtue, m_story.resolve, m_story.decisions.size(), m_story.battleDecisions.size()))
	    .draw(64, 350, ColorF(.95, .85, .59));
	const int cw = (Scene::Width() - 128) / 3;
	for (int i = 0; i < 6; ++i)
	{
		const RectF card(64 + (i % 3) * cw, h - 304 + (i / 3) * 86, cw - 12, 76);
		card.draw(ColorF(.02, .05, .053, .91)).drawFrame(1, ColorF(.66, .53, .30));
		FontAsset(U"campaignSmall")(U"第{}章 / {}"_fmt(i + 1, String(hero::Book()[i].year)))
		    .draw(card.x + 14, card.y + 8, ColorF(.80, .76, .57));
		FontAsset(U"campaignBody")(String(hero::Book()[i].title))
		    .draw(card.x + 14, card.y + 29, ColorF(.97, .89, .67));
		FontAsset(U"campaignSmall")(m_story.outcomes[i] == 2 ? U"約束を達成 / 次の章へ成果を継承"
		                                                     : U"任務を完遂")
		    .draw(card.x + 14, card.y + 54, ColorF(.64, .86, .70));
	}
	FontAsset(U"campaignBody")(U"天下への道は、まだ終わらない。").draw(64, h - 99, ColorF(.97, .89, .70));
	drawButton(buttonRect(3), U"英雄譚の選択へ戻る");
}

void HeroStoryScene::drawFailure() const
{
	(void)FontAsset(U"campaignBody")(U"このままでは、旗を守れない。\n仲間と立て直し、この章の任務に再挑戦しよ"
	                                 U"う。\n資源を最低60まで補い、戦場を再配置します。")
	    .draw(RectF(285, 230, Scene::Width() - 630, 160), ColorF(.96, .75, .60));
	drawButton(buttonRect(3), U"任務をやり直す");
}
