#include "HeroStoryScene.hpp"

void HeroStoryScene::draw() const
{
	Scene::SetBackground(ColorF(.045, .073, .075));
	m_presentation.Landscape(Scene::Rect(), m_landing ? .95 : .32);
	Scene::Rect().draw(ColorF(.015, .035, .035, m_landing ? .18 : .43));
	m_presentation.Atmosphere();
	const auto& c = hero::Book()[m_story.chapter];
	FontAsset(U"campaignTitle")(U"英雄譚 ― 劉備の旗").draw(34, 26, ColorF(.95, .85, .60));
	drawButton(Rect(Scene::Width() - 180, 26, 148, 38), U"スタートへ");
	drawButton(Rect(Scene::Width() - 310, 26, 115, 38), m_presentation.Sound() ? U"音 ON" : U"音 OFF");
	if (m_landing)
	{
		drawLanding();
	}
	else
	{
		RectF(24, 105, 235, Scene::Height() - 155).rounded(8).draw(ColorF(.08, .13, .13));
		if (m_story.phase == 2 && m_story.Tactical() && !m_showNotes)
			RectF(Scene::Width() - 321, 136, 301, 540)
			    .draw(ColorF(.025, .05, .045, .86))
			    .drawFrame(1, ColorF(.43, .37, .24));
		FontAsset(U"campaignSmall")(U"劉備の歩み / 第一部").draw(40, 123, ColorF(.76, .69, .47));
		if (m_story.phase == 2 && m_story.Tactical())
		{
			int row = 0;
			for (int i = 0; i < static_cast<int>(m_story.units.size()); ++i)
			{
				const auto& u = m_story.units[i];
				if (u.hero < 0)
					continue;
				Rect card(38, 164 + row * 78, 205, 70);
				card.draw(ColorF(.05, .09, .085, .95));
				m_presentation.Portrait(u.hero, RectF(card.x, card.y, 56, 70), u.hp > 0 ? 1 : .25);
				if (m_selectedUnit == i)
					card.drawFrame(1.5, ColorF(.94, .77, .39));
				FontAsset(U"campaignBody")(text(hero::Name(u.hero)))
				    .draw(card.x + 67, card.y + 3, ColorF(.92, .84, .65));
				FontAsset(U"campaignSmall")(
				    u.hp <= 0 ? U"戦線離脱"
				              : U"耐久 {} / {}　{}"_fmt(u.hp, u.maxHp, u.acted ? U"行動済" : U"行動可"))
				    .draw(card.x + 67, card.y + 30, ColorF(.72, .80, .69));
				FontAsset(U"campaignSmall")(
				    (u.hero == 0 ? text(officer::RoleName(officer::RoleOf(hero::Name(u.hero))))
				                 : text(officer::RoleName(officer::RoleOf(hero::Name(u.hero)))) +
				                       U" 親密{}"_fmt(m_story.Affinity(0, u.hero))))
				    .draw(card.x + 67, card.y + 49, ColorF(.68, .66, .46));
				++row;
			}
		}
		else
			for (int i = 0; i < 6; ++i)
			{
				const auto& a = hero::Book()[i];
				ColorF color = i == m_story.chapter  ? ColorF(.98, .86, .53)
				               : i < m_story.chapter ? ColorF(.47, .68, .53)
				                                     : ColorF(.43, .48, .46);
				FontAsset(U"campaignSmall")(U"第{}章  {}"_fmt(i + 1, String(a.year)))
				    .draw(40, 164 + i * 65, color);
				FontAsset(U"campaignBody")(String(a.title)).draw(40, 184 + i * 65, color);
			}
		FontAsset(U"campaignSmall")(U"{}"_fmt(String(m_story.Ending())))
		    .draw(40, Scene::Height() - 115, ColorF(.93, .79, .49));
		FontAsset(U"campaignTitle")(U"第{}章　{}"_fmt(m_story.chapter + 1, String(c.title)))
		    .draw(285, 104, ColorF(.95, .87, .68));
		FontAsset(U"campaignSmall")(
		    U"{} / {}　信望 {}　決意 {}　糧 {}　金 {}"_fmt(String(c.year), String(c.place), m_story.virtue,
		                                                   m_story.resolve, m_story.food, m_story.gold))
		    .draw(285, 153, ColorF(.77, .84, .75));
		drawButton(buttonRect(7), m_showNotes ? U"物語に戻る" : U"史話・仲間の記録");
		drawButton(buttonRect(8), U"手動で保存");
		if (m_showNotes)
		{
			drawNotes();
		}
		else if (m_story.phase == 0 || m_story.phase == 1 || m_story.phase == 3)
		{
			drawDialogue();
		}
		else if (m_story.phase == 4)
		{
			drawEnding();
		}
		else if (m_story.failed)
		{
			drawFailure();
		}
		else if (!m_story.Tactical())
		{
			drawCivilActions();
		}
		else
		{
			drawBattle();
		}
		if (!m_showNotes && !m_story.journal.empty())
			(void)FontAsset(U"campaignSmall")(text(m_story.journal.back()))
			    .draw(RectF(285, Scene::Height() - 58, Scene::Width() - 620, 45), ColorF(.86, .80, .61));
	}
	if (!m_landing && !m_showNotes && m_story.phase == 2 && m_story.battleEvent && !m_story.failed)
	{
		drawBattleEvent();
	}
	m_presentation.Overlay();
	if (!m_message.isEmpty())
		FontAsset(U"campaignSmall")(m_message).draw(30, Scene::Height() - 28, ColorF(.74, .81, .71));
}
