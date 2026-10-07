#include "HeroStoryScene.hpp"

void HeroStoryScene::draw() const
{
	Scene::SetBackground(ColorF(.045, .073, .075));
	fx.Landscape(Scene::Rect(), landing ? .95 : .32);
	Scene::Rect().draw(ColorF(.015, .035, .035, landing ? .18 : .43));
	fx.Atmosphere();
	const auto& c = hero::Book()[s.chapter];
	FontAsset(U"campaignTitle")(U"英雄譚 ― 劉備の旗").draw(34, 26, ColorF(.95, .85, .60));
	Button(Rect(Scene::Width() - 180, 26, 148, 38), U"スタートへ");
	Button(Rect(Scene::Width() - 310, 26, 115, 38), fx.Sound() ? U"音 ON" : U"音 OFF");
	if (landing)
	{
		drawLanding();
	}
	else
	{
		RectF(24, 105, 235, Scene::Height() - 155).rounded(8).draw(ColorF(.08, .13, .13));
		if (s.phase == 2 && s.Tactical() && !notes)
			RectF(Scene::Width() - 321, 136, 301, 540)
			    .draw(ColorF(.025, .05, .045, .86))
			    .drawFrame(1, ColorF(.43, .37, .24));
		FontAsset(U"campaignSmall")(U"劉備の歩み / 第一部").draw(40, 123, ColorF(.76, .69, .47));
		if (s.phase == 2 && s.Tactical())
		{
			int row = 0;
			for (int i = 0; i < static_cast<int>(s.units.size()); ++i)
			{
				const auto& u = s.units[i];
				if (u.hero < 0)
					continue;
				Rect card(38, 164 + row * 78, 205, 70);
				card.draw(ColorF(.05, .09, .085, .95));
				fx.Portrait(u.hero, RectF(card.x, card.y, 56, 70), u.hp > 0 ? 1 : .25);
				if (selected == i)
					card.drawFrame(1.5, ColorF(.94, .77, .39));
				FontAsset(U"campaignBody")(T(hero::Name(u.hero)))
				    .draw(card.x + 67, card.y + 3, ColorF(.92, .84, .65));
				FontAsset(U"campaignSmall")(
				    u.hp <= 0 ? U"戦線離脱"
				              : U"耐久 {} / {}　{}"_fmt(u.hp, u.maxHp, u.acted ? U"行動済" : U"行動可"))
				    .draw(card.x + 67, card.y + 30, ColorF(.72, .80, .69));
				FontAsset(U"campaignSmall")((u.hero == 0
				                                 ? T(officer::RoleName(officer::RoleOf(hero::Name(u.hero))))
				                                 : T(officer::RoleName(officer::RoleOf(hero::Name(u.hero)))) +
				                                       U" 親密{}"_fmt(s.Affinity(0, u.hero))))
				    .draw(card.x + 67, card.y + 49, ColorF(.68, .66, .46));
				++row;
			}
		}
		else
			for (int i = 0; i < 6; ++i)
			{
				const auto& a = hero::Book()[i];
				ColorF color = i == s.chapter  ? ColorF(.98, .86, .53)
				               : i < s.chapter ? ColorF(.47, .68, .53)
				                               : ColorF(.43, .48, .46);
				FontAsset(U"campaignSmall")(U"第{}章  {}"_fmt(i + 1, String(a.year)))
				    .draw(40, 164 + i * 65, color);
				FontAsset(U"campaignBody")(String(a.title)).draw(40, 184 + i * 65, color);
			}
		FontAsset(U"campaignSmall")(U"{}"_fmt(String(s.Ending())))
		    .draw(40, Scene::Height() - 115, ColorF(.93, .79, .49));
		FontAsset(U"campaignTitle")(U"第{}章　{}"_fmt(s.chapter + 1, String(c.title)))
		    .draw(285, 104, ColorF(.95, .87, .68));
		FontAsset(U"campaignSmall")(U"{} / {}　信望 {}　決意 {}　糧 {}　金 {}"_fmt(
		                                String(c.year), String(c.place), s.virtue, s.resolve, s.food, s.gold))
		    .draw(285, 153, ColorF(.77, .84, .75));
		Button(B(7), notes ? U"物語に戻る" : U"史話・仲間の記録");
		Button(B(8), U"手動で保存");
		if (notes)
		{
			drawNotes();
		}
		else if (s.phase == 0 || s.phase == 1 || s.phase == 3)
		{
			drawDialogue();
		}
		else if (s.phase == 4)
		{
			drawEnding();
		}
		else if (s.failed)
		{
			drawFailure();
		}
		else if (!s.Tactical())
		{
			drawCivilActions();
		}
		else
		{
			drawBattle();
		}
		if (!notes && !s.journal.empty())
			(void)FontAsset(U"campaignSmall")(T(s.journal.back()))
			    .draw(RectF(285, Scene::Height() - 58, Scene::Width() - 620, 45), ColorF(.86, .80, .61));
	}
	if (!landing && !notes && s.phase == 2 && s.battleEvent && !s.failed)
	{
		drawBattleEvent();
	}
	fx.Overlay();
	if (!message.isEmpty())
		FontAsset(U"campaignSmall")(message).draw(30, Scene::Height() - 28, ColorF(.74, .81, .71));
}
