#include "HeroStoryScene.hpp"

void HeroStoryScene::drawBattle() const
{

	auto r = mapRect();
	m_presentation.Board(m_story, m_selectedUnit, r);
	bool can = m_selectedUnit >= 0 && m_selectedUnit < static_cast<int>(m_story.units.size()) &&
	           m_story.units[m_selectedUnit].hero >= 0;
	drawButton(buttonRect(0),
	           can ? String(hero::SkillName(m_story.units[m_selectedUnit].hero)) : U"武将を選択",
	           can && m_story.units[m_selectedUnit].hp > 0 && !m_story.units[m_selectedUnit].acted &&
	               !m_story.units[m_selectedUnit].skillUsed);
	drawButton(buttonRect(1), U"敵の手番へ進む");
	if (m_story.chapter == 4)
		drawButton(buttonRect(2), U"周瑜と火攻め / 糧15",
		           m_story.turn >= 3 && !m_story.fireUsed && m_story.food >= 15);

	if (m_story.deepRules)
		drawButton(buttonRect(9), U"守備を固める / 反撃",
		           can && m_story.units[m_selectedUnit].hp > 0 && !m_story.units[m_selectedUnit].acted);

	drawButton(buttonRect(4), U"絆の号令 / 闘志60", m_story.spirit >= 60 && m_story.BattleReady());
	FontAsset(U"campaignSmall")(U"闘志 {} / 100"_fmt(m_story.spirit))
	    .draw(Scene::Width() - 305, 438, ColorF(.94, .81, .47));
	RectF(Scene::Width() - 305, 465, 268, 9).draw(ColorF(.10));
	RectF(Scene::Width() - 305, 465, 268 * m_story.spirit / 100., 9)
	    .draw(m_story.spirit >= 60 ? ColorF(.94, .71, .25) : ColorF(.42, .58, .43));

	String help =
	    can ? text(hero::SkillHelp(m_story.units[m_selectedUnit].hero)) : U"守備：被害-2 / 隣接へ反撃1";
	if (m_story.deepRules && r.mouseOver())
	{
		const auto cursor = Cursor::PosF() - r.pos;
		const int hovered = m_story.At(static_cast<int>(cursor.x / (r.w / hero::W)),
		                               static_cast<int>(cursor.y / (r.h / hero::H)));
		if (hovered >= 0 && m_story.units[hovered].enemy)
		{
			const auto intent = m_story.PlanEnemy(hovered);
			help = intent.target < 0 ? U"敵の狙い：足止め中・有効な経路なし"
			                         : U"敵の狙い：" +
			                               (m_story.units[intent.target].civilian
			                                    ? U"避難民"
			                                    : text(hero::Name(m_story.units[intent.target].hero))) +
			                               (intent.next >= 0 ? U" / 射撃位置へ移動" : U" / この位置から攻撃");
		}
	}
	(void)FontAsset(U"campaignSmall")(help).draw(RectF(Scene::Width() - 305, 489, 270, 50),
	                                             ColorF(.73, .81, .70));

	const int objective = m_story.ObjectiveTile();
	String goal = text(m_story.ObjectiveHelp());
	if (objective >= 0)
		goal += U" / 確保 {}/2手"_fmt(m_story.objectiveProgress);
	(void)FontAsset(U"campaignSmall")(
	    U"{}手目 / {}手  {}\n{}\n{}\n射線は林・水で遮断 / 赤枠は敵射程 / 守備は被害-2・反撃1\n連携：攻撃+{} / 被害-{}"_fmt(
	        m_story.turn, m_story.TurnLimit(),
	        m_story.chapter == 3 ? U"救出{} / 喪失{}"_fmt(m_story.escaped, m_story.lost)
	                             : U"敵の全滅＋劉備の生存",
	        goal,
	        m_story.deepRules ? U"移動1回＋攻撃・戦法・守備 / 拠点は敵の隣接なしで2手確保" : U"1手1行動",
	        officer::StoryAttack(m_story.Formation(m_selectedUnit)),
	        officer::StoryDefense(m_story.Formation(m_selectedUnit))))
	    .draw(RectF(285, r.bottomY() + 12, Scene::Width() - 620, 137), ColorF(.78, .84, .73));
}

void HeroStoryScene::drawBattleEvent() const
{

	Scene::Rect().draw(ColorF(0, .72));
	RectF panel(Scene::Center().x - 470, Scene::Center().y - 235, 940, 440);
	m_presentation.Landscape(panel, .28);
	panel.draw(ColorF(.025, .045, .043, .75)).drawFrame(2, ColorF(.80, .61, .29));
	m_presentation.Portrait(m_story.chapter == 3 ? 2 : 0, RectF(panel.x + 24, panel.y + 68, 158, 194));
	FontAsset(U"campaignSmall")(U"戦場の決断 / 会話と効果は創作")
	    .draw(panel.x + 214, panel.y + 28, ColorF(.86, .72, .40));
	FontAsset(U"storyDisplay")(m_story.chapter == 3 ? U"この橋を、越えさせぬ。" : U"今、何を守るか。")
	    .draw(panel.x + 210, panel.y + 65, ColorF(.99, .89, .65));
	(void)FontAsset(U"storyDialogue")(
	    m_story.chapter == 3
	        ? U"張飛「兄者、ここは俺が止める！」\n趙雲「民が傷ついています。救護の指示を！」\n踏み"
	          U"とどまる仲間と、救いを待つ人々。あなたは――"
	        : U"敵と戦列がぶつかる。前へ出る仲間、傷ついた兵。\n劉備「皆で生きて、この先へ進むぞ！"
	          U"」\n勢いをつかむか、傷ついた仲間を立て直すか。")
	    .draw(RectF(panel.x + 214, panel.y + 135, 685, 165), ColorF(.95, .92, .81));
	drawButton(battleChoiceRect(0),
	           m_story.chapter == 3 ? U"張飛を信じ、橋を守らせる" : U"仲間を信じ、一斉に押す");
	drawButton(battleChoiceRect(1),
	           m_story.chapter == 3 ? U"趙雲と、傷ついた民を守る" : U"仲間の傷を癒し、立て直す");
	FontAsset(U"campaignSmall")(m_story.chapter == 3 ? U"敵を1手足止め / 全武将再行動 / 闘志+25"
	                                                 : U"全武将再行動 / 闘志+25 / 親密+8")
	    .draw(battleChoiceRect(0).x + 12, battleChoiceRect(0).y + 56, ColorF(.77, .83, .70));
	FontAsset(U"campaignSmall")(U"味方の耐久+4 / 闘志+15 / 信望+3")
	    .draw(battleChoiceRect(1).x + 12, battleChoiceRect(1).y + 56, ColorF(.77, .83, .70));
}
