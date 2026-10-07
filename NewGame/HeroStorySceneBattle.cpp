#include "HeroStoryScene.hpp"

void HeroStoryScene::drawBattle() const
{

	auto r = Map();
	fx.Board(s, selected, r);
	bool can = selected >= 0 && selected < static_cast<int>(s.units.size()) && s.units[selected].hero >= 0;
	Button(B(0), can ? String(hero::SkillName(s.units[selected].hero)) : U"武将を選択",
	       can && s.units[selected].hp > 0 && !s.units[selected].acted && !s.units[selected].skillUsed);
	Button(B(1), U"敵の手番へ進む");
	if (s.chapter == 4)
		Button(B(2), U"周瑜と火攻め / 糧15", s.turn >= 3 && !s.fireUsed && s.food >= 15);
	if (can)
		(void)FontAsset(U"campaignSmall")(String(hero::SkillHelp(s.units[selected].hero)))
		    .draw(RectF(Scene::Width() - 305, 321, 270, 54), ColorF(.79, .86, .73));
	Button(B(4), U"絆の号令 / 闘志60", s.spirit >= 60 && s.BattleReady());
	FontAsset(U"campaignSmall")(U"闘志 {} / 100"_fmt(s.spirit))
	    .draw(Scene::Width() - 305, 438, ColorF(.94, .81, .47));
	RectF(Scene::Width() - 305, 465, 268, 9).draw(ColorF(.10));
	RectF(Scene::Width() - 305, 465, 268 * s.spirit / 100., 9)
	    .draw(s.spirit >= 60 ? ColorF(.94, .71, .25) : ColorF(.42, .58, .43));
	(void)FontAsset(U"campaignSmall")(U"攻撃・連携・救出で闘志が上昇。\n号令：生存する武将が再行動、耐久+2。")
	    .draw(RectF(Scene::Width() - 305, 489, 270, 50), ColorF(.73, .81, .70));
	(void)FontAsset(U"campaignSmall")(
	    U"{}手目 / {}手\n{}\n武将を選択 → 隣の空きマスで移動 / 射程内の敵で攻撃\n各武将は1手1行動 / 戦法は各戦闘1回 / 敵の手番で民も移動\n現在の連携：攻撃 +{} / 被害 -{} / 闘志 +{} / 林は被害-1"_fmt(
	        s.turn, s.TurnLimit(),
	        s.chapter == 3 ? U"右端の金の渡し場へ民を3組護送 / 救出 {}　喪失 {}"_fmt(s.escaped, s.lost)
	                       : U"敵部隊をすべて退ける / 劉備を守る",
	        officer::StoryAttack(s.Formation(selected)), officer::StoryDefense(s.Formation(selected)),
	        s.Formation(selected).morale))
	    .draw(RectF(285, r.bottomY() + 12, Scene::Width() - 620, 114), ColorF(.72, .80, .70));
}

void HeroStoryScene::drawBattleEvent() const
{

	Scene::Rect().draw(ColorF(0, .72));
	RectF panel(Scene::Center().x - 470, Scene::Center().y - 235, 940, 440);
	fx.Landscape(panel, .28);
	panel.draw(ColorF(.025, .045, .043, .75)).drawFrame(2, ColorF(.80, .61, .29));
	fx.Portrait(s.chapter == 3 ? 2 : 0, RectF(panel.x + 24, panel.y + 68, 158, 194));
	FontAsset(U"campaignSmall")(U"戦場の決断 / 会話と効果は創作")
	    .draw(panel.x + 214, panel.y + 28, ColorF(.86, .72, .40));
	FontAsset(U"storyDisplay")(s.chapter == 3 ? U"この橋を、越えさせぬ。" : U"今、何を守るか。")
	    .draw(panel.x + 210, panel.y + 65, ColorF(.99, .89, .65));
	(void)FontAsset(U"storyDialogue")(
	    s.chapter == 3 ? U"張飛「兄者、ここは俺が止める！」\n趙雲「民が傷ついています。救護の指示を！」\n踏み"
	                     U"とどまる仲間と、救いを待つ人々。あなたは――"
	                   : U"敵と戦列がぶつかる。前へ出る仲間、傷ついた兵。\n劉備「皆で生きて、この先へ進むぞ！"
	                     U"」\n勢いをつかむか、傷ついた仲間を立て直すか。")
	    .draw(RectF(panel.x + 214, panel.y + 135, 685, 165), ColorF(.95, .92, .81));
	Button(BattleChoice(0), s.chapter == 3 ? U"張飛を信じ、橋を守らせる" : U"仲間を信じ、一斉に押す");
	Button(BattleChoice(1), s.chapter == 3 ? U"趙雲と、傷ついた民を守る" : U"仲間の傷を癒し、立て直す");
	FontAsset(U"campaignSmall")(s.chapter == 3 ? U"敵を1手足止め / 全武将再行動 / 闘志+25"
	                                           : U"全武将再行動 / 闘志+25 / 親密+8")
	    .draw(BattleChoice(0).x + 12, BattleChoice(0).y + 56, ColorF(.77, .83, .70));
	FontAsset(U"campaignSmall")(U"味方の耐久+4 / 闘志+15 / 信望+3")
	    .draw(BattleChoice(1).x + 12, BattleChoice(1).y + 56, ColorF(.77, .83, .70));
}
