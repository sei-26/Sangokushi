#include "StrategistJourneyScene.hpp"
#include "CampaignUI.hpp"
#include "CampaignVisuals.hpp"

using namespace frontline;
using namespace campaignui;

void StrategistJourneyScene::DrawMap() const
{
	const auto& world = m_journey.world;
	const auto& known = m_journey.intelligence[0];
	const double cell = Cell();
	const auto viewport = MapRect();
	const auto previousScissor = Graphics2D::GetScissorRect();
	Graphics2D::SetScissorRect(Rect(static_cast<int>(viewport.x), static_cast<int>(viewport.y),
	                                static_cast<int>(viewport.w), static_cast<int>(viewport.h)));
	const ScopedRenderStates2D clipped(RasterizerState::SolidCullNoneScissor);
	MapRect().draw(ColorF(.09, .13, .12));
	for (int y = 1; y < StrategistJourney::Rows - 1; ++y)
		for (int x = 1; x < StrategistJourney::Columns - 1; ++x)
		{
			const int tile = Campaign::At(x, y);
			const auto center = Position(tile);
			const auto shape = campaignvisual::TileShape(center, cell, true);
			const auto terrain = world.tiles[tile].terrain;
			shape.draw(terrain == Terrain::River || terrain == Terrain::Bridge ? ColorF(.18, .33, .39)
			           : terrain == Terrain::Forest                            ? ColorF(.24, .32, .18)
			           : terrain == Terrain::Mountain                          ? ColorF(.37, .32, .25)
			                                                                   : ColorF(.50, .46, .31));
			if (terrain == Terrain::Forest || terrain == Terrain::Mountain)
				m_art.Relief(center, cell, terrain, tile, 2);
			if (terrain == Terrain::Bridge)
				Line(center.movedBy(-cell * .42, 0), center.movedBy(cell * .42, 0))
				    .draw(cell * .16, ColorF(.80, .68, .43));
			shape.drawFrame(.8, ColorF(.92, .83, .57, .20));
			if (!known.visible[tile])
				shape.draw(ColorF(.015, .025, .04, .62));
			if (known.visible[tile] && world.MapDistance(world.armies[m_selected].tile, tile) == 1 &&
			    !m_journey.chain.moved[m_selected])
				shape.drawFrame(1.4, ColorF(.46, .81, .90, .65));
		}
	const auto exit = Position(m_journey.goal);
	Circle(exit, cell * .52).draw(ColorF(.87, .70, .21, .22)).drawFrame(2, ColorF(.99, .84, .44));
	FontAsset(U"campaignSmall")(m_journey.Definition().kind == JourneyObjective::HoldPositions ? U"北の烽火"
	                            : m_journey.Definition().kind == JourneyObjective::Escort ? U"搬出先"
	                                                                                      : U"退路の出口")
	    .drawAt(exit.movedBy(0, -cell * .7), ColorF(.99, .89, .57));
	if (m_journey.Definition().kind == JourneyObjective::HoldPositions)
	{
		for (int tile : {m_journey.goal, m_journey.Definition().secondGoal})
		{
			const auto p = Position(tile);
			Circle(p, cell * .52)
			    .drawFrame(3,
			               m_journey.PositionSecured(tile) ? ColorF(.38, .88, .70) : ColorF(.99, .69, .35));
		}
		FontAsset(U"campaignSmall")(U"南の烽火")
		    .drawAt(Position(m_journey.Definition().secondGoal).movedBy(0, cell * .9), ColorF(.99, .89, .57));
	}
	if (m_journey.Definition().kind == JourneyObjective::Escort)
	{
		const auto& cart = world.armies[StrategistJourney::ConvoyArmy];
		for (int x = 6; x < 12; ++x)
			Line(Position(Campaign::At(x, 11)), Position(Campaign::At(x + 1, 11)))
			    .draw(3, ColorF(.96, .80, .36, .40));
		if (cart.troops > 0)
		{
			campaignvisual::ArmyIcon(Position(cart.tile), cell, cart, false, world.generals[cart.general],
			                         m_faces);
			FontAsset(U"campaignSmall")(U"糧車 / {}"_fmt(cart.cargoFood))
			    .drawAt(Position(cart.tile).movedBy(0, cell * .8), ColorF(.99, .88, .60));
		}
	}
	m_art.City(Position(world.cities[0].tile), cell * 1.6, 0, false);
	FontAsset(U"campaignSmall")(U"野営地").drawAt(Position(world.cities[0].tile).movedBy(0, cell),
	                                              ColorF(.95, .86, .64));
	const auto preview = m_journey.Preview(m_orders);
	for (int i = 0; i < static_cast<int>(preview.events.size()); ++i)
	{
		const auto& event = preview.events[i];
		if (!event.success)
			continue;
		const ColorF color = i == 0   ? ColorF(.40, .82, .97)
		                     : i == 1 ? ColorF(.78, .70, 1.)
		                              : ColorF(1., .64, .30);
		if (event.from != event.to)
			Line(Position(event.from), Position(event.to)).drawArrow(3, Vec2(9, 9), color);
		Circle(Position(event.to), cell * (.39 + i * .08)).drawFrame(2, color);
		FontAsset(U"campaignSmall")(U"{}"_fmt(i + 1))
		    .drawAt(Position(event.to).movedBy(cell * .5, -cell * .5), color);
	}
	for (int i = 0; i < 3; ++i)
	{
		const auto& army = world.armies[i];
		if (army.troops <= 0)
			continue;
		campaignvisual::ArmyIcon(Position(army.tile), cell, army, m_selected == i, world.generals[i],
		                         m_faces);
		if (m_journey.chain.concealed[i])
		{
			Circle(Position(army.tile), cell * .54).drawFrame(2, ColorF(.77, .65, 1.));
			FontAsset(U"campaignSmall")(U"伏").drawAt(Position(army.tile).movedBy(0, -cell * .58),
			                                          ColorF(.87, .78, 1.));
		}
		FontAsset(U"campaignSmall")(Text(world.generals[i].name))
		    .drawAt(Position(army.tile).movedBy(0, cell * .7), ColorF(.95, .94, .82));
	}
	for (const auto& contact : known.contacts)
	{
		const auto& army = contact.observed;
		const auto at = Position(army.tile);
		if (contact.current)
		{
			campaignvisual::ArmyIcon(at, cell, army, m_enemy == contact.id, world.generals[army.general],
			                         m_faces);
			FontAsset(U"campaignSmall")(army.general == 4 && m_journey.enemyPolicy == 2 ? U"斥候"
			                            : army.stance == battle::Stance::Guard          ? U"固守"
			                                                                            : U"追撃")
			    .drawAt(at.movedBy(0, cell * .7), ColorF(1., .78, .55));
		}
		else
		{
			Circle(at, cell * .3).draw(ColorF(.25, .28, .32)).drawFrame(1, ColorF(.70, .75, .8));
			FontAsset(U"campaignSmall")(U"?").drawAt(at, ColorF(.88, .90, .93));
		}
	}
	const int hover = Pick(Cursor::PosF());
	if (hover >= 0)
	{
		campaignvisual::TileShape(Position(hover), cell, true).drawFrame(2, ColorF(.99, .87, .54));
		const int enemy = EnemyAt(hover);
		if (enemy >= 0)
		{
			const auto* contact = known.Find(enemy);
			const auto& army = contact->observed;
			const Vec2 at(MapRect().x + 12, MapRect().y + 12);
			Panel(RectF(at, 295, 67), true);
			FontAsset(U"campaignSmall")(Text(world.generals[army.general].name) + U" / " +
			                            String(battle::Name(army.stance)))
			    .draw(at + Vec2(12, 8), ColorF(.98, .84, .62));
			FontAsset(U"campaignSmall")(U"確認中：兵{} / 士気{}"_fmt(army.troops, army.morale))
			    .draw(at + Vec2(12, 36), ColorF(.85, .89, .88));
		}
	}
	if (m_replay >= 0 && m_replay < static_cast<int>(m_journey.lastChain.events.size()))
	{
		const auto& event = m_journey.lastChain.events[m_replay];
		if (event.success)
		{
			const auto at = Position(event.to);
			const double t = std::fmod(m_replayTime, 1.);
			Circle(at, cell * (.35 + t)).drawFrame(4 * (1 - t), ColorF(1., .76, .32, 1 - t));
			if (event.damage > 0)
				FontAsset(U"storyAccent")(U"−{}"_fmt(event.damage))
				    .drawAt(at.movedBy(0, -cell - t * 18), ColorF(1., .72, .36));
		}
	}
	MapRect().drawFrame(1.5, ColorF(.71, .61, .38));
	Graphics2D::SetScissorRect(previousScissor);
}
void StrategistJourneyScene::DrawEncounter() const
{
	m_fx.Landscape(Scene::Rect(), .75);
	Scene::Rect().draw(ColorF(.025, .04, .06, .45));
	FontAsset(U"storyDisplay")(U"序章  乱世に、策を携えて").draw(48, 65, ColorF(.98, .86, .57));
	FontAsset(U"campaignBody")(U"184年 / 黄巾の乱").draw(52, 128, ColorF(.82, .88, .84));
	m_fx.Portrait(0, RectF(70, 178, Scene::Height() * .34, Scene::Height() * .48), .93);
	const String speakers[] = {U"時代の記録", U"劉備", U"主人公 / 名もなき軍師"};
	const String lines[] = {U"黄巾の乱が起こり、後漢の各地に戦火が広がった。\nあなたが出会ったのは、逃げる民"
	                        U"を守ろうとする小さな義勇軍だった。",
	                        U"私たちには、すべての敵を倒す兵などありません。\nそれでも、あの人々を見捨てるわ"
	                        U"けにはいかない。退く道を、共に開いてくれませんか。",
	                        U"敵を追わせれば、正面に隙が生まれる。森に伏せた隊が敵を崩し、突破役が道を開く。"
	                        U"\nまずは、この三人の力を一つの策につなげよう。"};
	const RectF dialog(40, Scene::Height() - 252, Scene::Width() - 80, 154);
	Panel(dialog, true);
	FontAsset(U"storyAccent")(speakers[Min(m_beat, 2)])
	    .draw(dialog.x + 22, dialog.y + 12, ColorF(.98, .86, .54));
	(void)FontAsset(U"storyDialogue")(lines[Min(m_beat, 2)])
	    .draw(RectF(dialog.x + 22, dialog.y + 57, dialog.w - 44, 89), ColorF(.90, .93, .88));
	DrawButton(Rect(Scene::Center().x - 160, Scene::Height() - 72, 320, 40),
	           m_beat < 2 ? U"話を進める / Space" : U"軍議へ / Space");
	FontAsset(U"campaignSmall")(U"史実を背景にした創作の出会い・局地戦 / 主人公の仕官先は、今後の物語で選ぶ")
	    .draw(42, Scene::Height() - 29, ColorF(.76, .80, .78));
}
void StrategistJourneyScene::DrawPreparation() const
{
	m_fx.Landscape(Scene::Rect(), .48);
	Scene::Rect().draw(ColorF(.015, .03, .05, .72));
	const Rect r(Scene::Center().x - 440, Scene::Center().y - 250, 880, 500);
	Panel(r, true);
	const auto& camp = m_journey.world.cities[0];
	FontAsset(U"campaignTitle")(U"軍議  " + String(m_journey.MissionName()))
	    .draw(r.x + 22, r.y + 17, ColorF(.98, .86, .55));
	FontAsset(U"campaignBody")(
	    U"野営地：兵{} / 兵糧{} / 軍資金{} / 信頼{}"_fmt(camp.troops, camp.food, camp.gold, m_journey.trust))
	    .draw(r.x + 22, r.y + 69, ColorF(.85, .92, .88));
	FontAsset(U"campaignSmall")(U"{} / {}指揮以内 / 主力1,800以上を温存"_fmt(
	                                String(m_journey.Definition().objective), m_journey.Definition().limit))
	    .draw(r.x + 22, r.y + 109, ColorF(.86, .84, .70));
	for (int i = 0; i < 2; ++i)
	{
		const Rect card(r.x + 20 + i * 430, r.y + 145, 410, 145);
		Panel(card, m_choice == i);
		if (m_choice == i)
			card.drawFrame(2, ColorF(.95, .78, .37));
		FontAsset(U"campaignBody")(i == 0 ? U"伏兵の段取り" : U"厚い兵站")
		    .draw(card.x + 15, card.y + 12, ColorF(.97, .87, .62));
		(void)FontAsset(U"campaignSmall")(
		    i == 0
		        ? U"兵糧3,000 / 軍資金200\n関羽隊が伏兵を準備して開戦。\nすぐ連鎖を狙えるが、携行糧は少ない。"
		        : U"兵糧4,800 / 軍資金80\n携行糧と士気に余裕を持って開戦。\n伏兵は戦場で自分で準備する。")
		    .draw(RectF(card.x + 15, card.y + 49, 380, 88), ColorF(.81, .87, .88));
	}
	FontAsset(U"campaignSmall")(U"比較する相手 / 敵の対抗手段を試せる試作設定")
	    .draw(r.x + 22, r.y + 299, ColorF(.75, .83, .88));
	for (int i = 0; i < 3; ++i)
	{
		const Rect button(r.x + 20 + i * 286, r.y + 325, 276, 42);
		DrawButton(button, String(StrategistJourney::EnemyName(i)));
		if (m_policy == i)
			button.drawFrame(2, ColorF(.95, .78, .37));
	}
	FontAsset(U"campaignSmall")(m_journey.Definition().kind == JourneyObjective::Escort
	                                ? U"主力最大4,200＋糧車護衛600。追加で糧車の糧900・積荷2,000を支払う。"
	                                : U"出陣兵は最大4,200。守備兵1,000を残す。生存兵と携行糧は戦後に戻る。")
	    .draw(r.x + 22, r.y + 375, ColorF(.78, .84, .78));
	DrawButton(Rect(r.x + 20, r.y + 403, 270, 40), U"募兵：兵+1,200 / 金250・糧500",
	           !m_journey.reinforced && camp.gold >= 250 && camp.food >= 500);
	DrawButton(Rect(r.x + 560, r.y + 403, 300, 40), U"この準備で戦場へ");
	(void)FontAsset(U"campaignSmall")(Text(m_journey.message))
	    .draw(RectF(r.x + 22, r.y + 455, 836, 33), ColorF(.92, .83, .62));
}
void StrategistJourneyScene::DrawDebrief() const
{
	const bool complete = m_journey.stage == JourneyStage::Complete;
	m_fx.Landscape(Scene::Rect(), .45);
	Scene::Rect().draw(ColorF(.01, .03, .05, .77));
	const Rect r(Scene::Center().x - 440, Scene::Center().y - 235, 880, 470);
	Panel(r, true);
	FontAsset(U"campaignTitle")(complete        ? U"この策が、次の信頼になる"
	                            : m_journey.won ? U"任された目的を果たした。次の軍議へ。"
	                                            : U"撤退。次に変える一手を考えよう。")
	    .draw(r.x + 25, r.y + 20, ColorF(.98, .85, .55));
	FontAsset(U"campaignBody")(
	    U"最大{}連鎖 / 生還{} / 損耗{} / 信頼{}"_fmt(m_journey.bestChain, m_journey.survivors,
	                                                 m_journey.casualties, m_journey.trust))
	    .draw(r.x + 25, r.y + 81, ColorF(.85, .91, .88));
	const auto& camp = m_journey.world.cities[0];
	FontAsset(U"campaignSmall")(
	    U"次の準備へ：野営地の兵{} / 兵糧{} / 軍資金{}"_fmt(camp.troops, camp.food, camp.gold))
	    .draw(r.x + 25, r.y + 119, ColorF(.80, .86, .81));
	(void)FontAsset(U"storyDialogue")(
	    complete ? U"「兵の数だけでは、道は開けなかった。あなたの策と、皆の働きがつないだのです。」\n序章の３"
	               U"戦が試作範囲。今後は出会いを重ね、仕官先を選ぶ歴史へつなぎます。"
	    : m_journey.won ? U"劉備「ありがとう。あなたが開いた道を、皆が生きて帰ってきた。\n次も、力を貸しても"
	                      U"らえますか。」"
	                    : U"劉備「兵を失ったことを、忘れないでください。\n今度は敵の構えと、退く時機を共に見"
	                      U"定めましょう。」")
	    .draw(RectF(r.x + 25, r.y + 160, 830, 97), ColorF(.93, .91, .80));
	if (!complete)
	{
		const auto& reports = m_journey.reports;
		const int first = Max(0, static_cast<int>(reports.size()) - 3);
		for (int i = first; i < static_cast<int>(reports.size()); ++i)
			(void)FontAsset(U"campaignSmall")(Text(reports[i].text))
			    .draw(RectF(r.x + 25, r.y + 275 + (i - first) * 31, 830, 29), ColorF(.78, .85, .88));
		DrawButton(Rect(Scene::Center().x - 310, Scene::Center().y + 178, 300, 40),
		           U"軍議からやり直す / 試作", bool(m_checkpoint));
		DrawButton(Rect(Scene::Center().x + 10, Scene::Center().y + 178, 300, 40),
		           m_journey.mission < 2 ? U"戦果を引き継ぎ、次の軍議へ" : U"序章の戦果を受け取る",
		           m_journey.won);
	}
	else
	{
		DrawButton(Rect(Scene::Center().x - 160, Scene::Center().y + 158, 320, 40), U"序章を初めから試す");
	}
}
void StrategistJourneyScene::Draw() const
{
	Scene::SetBackground(ColorF(.025, .045, .065));
	if (m_journey.stage == JourneyStage::Encounter)
		DrawEncounter();
	else if (m_journey.stage == JourneyStage::Preparation)
		DrawPreparation();
	else if (m_journey.stage == JourneyStage::Battle || m_replay >= 0)
	{
		const auto& world = m_journey.world;
		FontAsset(U"campaignTitle")(U"184年  黄巾の乱 / 軍師の第一歩").draw(20, 17, ColorF(.98, .86, .59));
		FontAsset(U"campaignSmall")(
		    U"{} / 指揮 {} / {} / {}"_fmt(String(m_journey.MissionName()), m_journey.turn,
		                                  m_journey.Definition().limit,
		                                  String(StrategistJourney::EnemyName(m_journey.enemyPolicy))))
		    .draw(22, 64, ColorF(.77, .85, .87));
		FontAsset(U"campaignSmall")(Text(m_journey.ObjectiveProgress())).draw(242, 99, ColorF(.96, .83, .56));
		DrawMap();
		Panel(RectF(14, 128, 214, Scene::Height() - 244));
		FontAsset(U"campaignBody")(U"策を託す三人").draw(26, 130, ColorF(.98, .86, .58));
		const char32_t* roles[] = {U"誘導 / 敵を引き出す", U"伏兵 / 隊列を崩す", U"突破 / 道を開く"};
		for (int i = 0; i < 3; ++i)
		{
			const Rect card(18, 152 + i * 96, 206, 88);
			if (m_selected == i)
				card.draw(ColorF(.13, .22, .30)).drawFrame(1.5, ColorF(.94, .76, .40));
			campaignvisual::OfficerCard(RectF(card.x + 7, card.y + 9, 42, 57), world.generals[i], m_faces);
			FontAsset(U"campaignBody")(Text(world.generals[i].name))
			    .draw(card.x + 57, card.y + 5, ColorF(.97, .91, .76));
			FontAsset(U"campaignSmall")(U"兵{} / 士気{}"_fmt(world.armies[i].troops, world.armies[i].morale))
			    .draw(card.x + 57, card.y + 32, ColorF(.79, .87, .90));
			FontAsset(U"campaignSmall")(String(roles[i]))
			    .draw(card.x + 9, card.y + 63, ColorF(.83, .78, .97));
		}
		const String hints[] = {U"誘導：隣接敵を引く\n固守する敵には無効",
		                        U"伏兵：森林で準備\n敵に接近されると露見",
		                        U"突破：崩れた隊列へ\n士気60以下でも使用可"};
		(void)FontAsset(U"campaignSmall")(
		    hints[m_selected] +
		    U"\n右クリック：通常移動\nホイール：ズーム\n中ボタン：地図移動\nHome：全体表示")
		    .draw(RectF(26, 449, 192, Max(30, Scene::Height() - 568)), ColorF(.75, .83, .87));
		const int right = Scene::Width() - 302;
		Panel(RectF(right, 128, 288, Scene::Height() - 244));
		const auto& selected = world.armies[m_selected];
		FontAsset(U"campaignBody")(U"{}の一手"_fmt(Text(world.generals[m_selected].name)))
		    .draw(right + 13, 136, ColorF(.98, .85, .55));
		(void)FontAsset(U"campaignSmall")(U"移動：{} / 戦法・攻撃：{}\n携行糧 {} / 伏兵 {}"_fmt(
		                                      m_journey.chain.moved[m_selected] ? U"済" : U"可",
		                                      m_journey.chain.spent[m_selected] ? U"済" : U"可",
		                                      selected.food,
		                                      m_journey.chain.concealed[m_selected] ? U"準備中" : U"なし"))
		    .draw(RectF(right + 13, 172, 264, 48), ColorF(.78, .87, .88));
		DrawButton(Control(1), U"選択武将の戦法を予約",
		           !m_journey.chain.spent[m_selected] && m_orders.size() < 3 && selected.troops > 0);
		DrawButton(Control(2), U"伏兵準備",
		           m_selected == 1 && !m_journey.chain.moved[1] && !m_journey.chain.spent[1]);
		DrawButton(Control(3), U"通常攻撃", !m_journey.chain.spent[m_selected] && selected.troops > 0);
		DrawButton(Control(8), U"固守 / 攻撃を使って守る（G）",
		           !m_journey.chain.spent[m_selected] && selected.troops > 0);
		FontAsset(U"campaignSmall")(U"予約した連鎖 / 最大３手").draw(right + 13, 344, ColorF(.95, .86, .67));
		const auto preview = m_journey.Preview(m_orders);
		for (int i = 0; i < 3; ++i)
		{
			const RectF card(right + 12, 373 + i * 58, 264, 52);
			card.draw(ColorF(.055, .10, .15)).drawFrame(1, ColorF(.31, .37, .40));
			if (i >= static_cast<int>(m_orders.size()))
			{
				FontAsset(U"campaignSmall")(U"{}  未予約"_fmt(i + 1))
				    .draw(card.x + 10, card.y + 14, ColorF(.49, .60, .67));
				continue;
			}
			const auto& order = m_orders[i];
			const bool reached = i < static_cast<int>(preview.events.size());
			const bool succeeds = reached && preview.events[i].success;
			FontAsset(U"campaignSmall")(U"{} {} / {}"_fmt(i + 1, Text(world.generals[order.army].name),
			                                              String(TacticChain::Name(order.tactic))))
			    .draw(card.x + 10, card.y + 3, ColorF(.96, .87, .66));
			FontAsset(U"campaignSmall")(succeeds  ? U"現在の情報では成立"
			                            : reached ? U"現在の配置では中断"
			                                      : U"前の成立を待つ")
			    .draw(card.x + 10, card.y + 27, succeeds ? ColorF(.51, .85, .76) : ColorF(.96, .61, .46));
		}
		DrawButton(Control(4), U"見本を予約");
		DrawButton(Control(5), U"予約を消す", !m_orders.empty());
		DrawButton(Control(6), U"連鎖を発動 / C", !m_orders.empty());
		DrawButton(Control(7), U"指揮を終える / Space");
		const String status = !preview.events.empty() && !preview.events.back().success
		                          ? Text(preview.events.back().reason)
		                          : Text(m_journey.message);
		(void)FontAsset(U"campaignSmall")(status).draw(
		    RectF(242, Scene::Height() - 105, Scene::Width() - 565, 70), ColorF(.89, .86, .72));
		FontAsset(U"campaignSmall")(U"予測は確認情報 / 未確認の妨害で中断あり / B：BGM / M：効果音")
		    .draw(24, Scene::Height() - 28, ColorF(.63, .74, .80));
		if (m_replay >= 0)
			m_fx.Overlay();
	}
	else
		DrawDebrief();
	DrawButton(Control(0), U"メニューへ");
}
