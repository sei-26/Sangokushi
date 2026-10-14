#include "StrategistWarScene.hpp"
#include "CampaignUI.hpp"
#include "CampaignVisuals.hpp"

using namespace frontline;
using namespace campaignui;

void StrategistWarScene::DrawMap() const
{
	const auto& world = m_battle.world;
	const auto& known = m_battle.intelligence[0];
	const auto rect = MapRect();
	const double cell = Cell();
	rect.draw(ColorF(.19, .22, .17));
	for (int y = 0; y < StrategistBattle::MapRows; ++y)
		for (int x = 0; x < StrategistBattle::MapColumns; ++x)
		{
			const int tile = Campaign::At(x, y);
			const auto terrain = world.tiles[tile].terrain;
			const Vec2 center = Position(tile);
			const auto shape = campaignvisual::TileShape(center, cell, true, 1.);
			shape.draw(terrain == Terrain::Sea      ? ColorF(.14, .21, .25)
			           : terrain == Terrain::River  ? ColorF(.21, .36, .40)
			           : terrain == Terrain::Forest ? ColorF(.27, .34, .22)
			                                        : ColorF(.48, .47, .33));
			if (terrain == Terrain::Forest)
				m_art.Relief(center, cell, terrain, tile, 2);
			if (terrain == Terrain::Bridge)
				Line(center.movedBy(-cell * .43, 0), center.movedBy(cell * .43, 0))
				    .draw(cell * .17, ColorF(.77, .67, .43));
			if (terrain != Terrain::Sea)
			{
				shape.draw(known.owners[tile] == 0 ? ColorF(.16, .5, .73, .12) : ColorF(.72, .2, .15, .10));
				shape.drawFrame(.7, ColorF(.85, .84, .67, .22));
				if (!known.visible[tile])
					shape.draw(ColorF(.015, .04, .065, .52));
			}
		}
	for (int i = 0; i < 5; ++i)
	{
		const auto& army = world.armies[i];
		if (army.troops <= 0)
			continue;
		Vec2 last = Position(army.tile);
		for (int tile : army.path)
		{
			const Vec2 next = Position(tile);
			Line(last, next)
			    .draw(i == m_selected ? 2.5 : 1., ColorF(.98, .81, .37, i == m_selected ? .95 : .25));
			last = next;
		}
		if (i == m_selected)
			Circle(Position(army.target), cell * .32).drawFrame(2, ColorF(.99, .85, .45));
		const auto& operation = m_battle.operations[i];
		if (operation.state == OperationState::Waiting)
		{
			const Vec2 start = Position(army.tile), end = Position(operation.target);
			for (int piece = 0; piece < 12; ++piece)
				Line(start.lerp(end, piece / 12.), start.lerp(end, (piece + .45) / 12.))
				    .draw(i == m_selected ? 2.5 : 1., ColorF(.36, .79, .94, .78));
			Circle(end, cell * .34).drawFrame(2, ColorF(.36, .79, .94));
		}
	}
	for (int i = 0; i < static_cast<int>(world.cities.size()); ++i)
	{
		const auto& city = world.cities[i];
		const auto center = Position(city.tile);
		m_art.City(center, cell * 1.5, known.owners[city.tile], false);
		FontAsset(U"campaignSmall")(Text(city.name))
		    .drawAt(center.movedBy(-cell * .5, cell * 1.1), ColorF(.98, .89, .68));
	}
	struct DisplayUnit
	{
		int id;
		Army army;
		bool current;
		int seenDay;
	};
	std::vector<DisplayUnit> units;
	for (int i = 0; i <= StrategistBattle::Convoy; ++i)
		if (world.armies[i].troops > 0)
			units.push_back({i, world.armies[i], true, world.day});
	for (const auto& contact : known.contacts)
		units.push_back({contact.id, contact.observed, contact.current, contact.seenDay});
	for (const auto& display : units)
	{
		const int i = display.id;
		const auto& army = display.army;
		int count = 0, order = 0;
		for (const auto& other : units)
			if (other.army.tile == army.tile)
			{
				++count;
				if (other.id < i)
					++order;
			}
		const auto center = Position(army.tile).movedBy((order - (count - 1) * .5) * cell * .45, 0);
		if (!display.current)
		{
			Circle(center, cell * .3).draw(ColorF(.18, .26, .33, .8)).drawFrame(1.5, ColorF(.66, .74, .81));
			FontAsset(U"campaignSmall")(U"?").drawAt(center, ColorF(.83, .89, .94));
			continue;
		}
		campaignvisual::ArmyIcon(center, cell * .8, army, i == m_selected, world.generals[army.general],
		                         m_faces);
		Circle(center.movedBy(-cell * .28, -cell * .30), 8)
		    .draw(i < 5 ? ColorF(.12, .38, .61) : ColorF(.64, .17, .14));
		FontAsset(U"campaignSmall")(i < 5 ? U"{}"_fmt(i + 1) : U"敵")
		    .drawAt(center.movedBy(-cell * .28, -cell * .30), Palette::White);
	}
	const int hover = Pick(Cursor::PosF());
	if (hover >= 0)
		campaignvisual::TileShape(Position(hover), cell, true).drawFrame(2, ColorF(.98, .89, .62));
	for (const auto& display : units)
		if (display.army.tile == hover)
		{
			const auto& army = display.army;
			const Vec2 at(Min(Cursor::PosF().x + 18, rect.rightX() - 244),
			              Min(Cursor::PosF().y + 18, rect.bottomY() - 65));
			Panel(RectF(at, 236, 60), true);
			FontAsset(U"campaignSmall")(
			    Text(world.generals[army.general].name) +
			    (display.current ? U" / 確認中" : U" / {}日前"_fmt(world.day - display.seenDay)))
			    .draw(at + Vec2(10, 5), ColorF(.96, .84, .59));
			FontAsset(U"campaignSmall")(
			    display.current
			        ? U"兵{} / 士気{} / {}"_fmt(army.troops, army.morale, String(battle::Name(army.stance)))
			        : U"目撃時の兵 {} / 現在は不明"_fmt(army.troops))
			    .draw(at + Vec2(10, 30), ColorF(.85, .89, .90));
			break;
		}
	rect.drawFrame(1.5, ColorF(.63, .56, .38));
}
void StrategistWarScene::DrawCouncil() const
{
	Scene::Rect().draw(ColorF(.015, .035, .06, .86));
	const RectF panel(Scene::Center().x - 465, Scene::Center().y - 255, 930, 510);
	Panel(panel, true);
	FontAsset(U"campaignTitle")(U"軍議 / 渡河・兵糧護送")
	    .draw(panel.x + 30, panel.y + 20, ColorF(.97, .85, .54));
	FontAsset(U"campaignBody")(U"従軍軍師として、護送方面の指揮を任される。")
	    .draw(panel.x + 30, panel.y + 72, ColorF(.90, .91, .86));
	(void)FontAsset(U"campaignBody")(
	    U"前線は兵糧を待っている。45日以内に3,500以上を届け、戦闘兵7,500以上を残せ。\n"
	    U"輸送隊をいつ出すか、渡河点を誰が守るか。敵を倒すことだけが目的ではない。")
	    .draw(RectF(panel.x + 30, panel.y + 109, 865, 77), ColorF(.79, .85, .86));
	FontAsset(U"campaignSmall")(U"敵は10日間の集結後に進軍。渡河点と護衛の配置を整える猶予がある。")
	    .draw(panel.x + 30, panel.y + 184, ColorF(.78, .84, .73));
	for (int i = 0; i < 3; ++i)
	{
		const Rect card(Scene::Center().x - 435 + i * 295, Scene::Center().y - 45, 280, 155);
		Panel(card, m_battle.plan == i);
		if (m_battle.plan == i)
			card.drawFrame(2, ColorF(.96, .78, .38));
		FontAsset(U"campaignBody")(String(StrategistBattle::PlanName(i)))
		    .draw(card.x + 14, card.y + 14, ColorF(.97, .87, .61));
		(void)FontAsset(U"campaignSmall")(String(StrategistBattle::PlanDescription(i)))
		    .draw(RectF(card.x + 14, card.y + 48, card.w - 28, 77), ColorF(.82, .88, .90));
		FontAsset(U"campaignSmall")(m_battle.plan == i ? U"採用する方針" : U"クリックで選択")
		    .draw(card.x + 14, card.y + 126, ColorF(.84, .72, .46));
	}
	DrawButton(Rect(Scene::Center().x - 310, Scene::Center().y + 123, 620, 34),
	           U"検証する敵方針：{}"_fmt(String(StrategistBattle::EnemyPolicyName(m_battle.enemyPolicy))));
	DrawButton(Rect(Scene::Center().x - 310, Scene::Center().y + 164, 620, 28),
	           m_battle.intelligenceEnabled ? U"観測ルール：偵察情報 / クリックで比較用の全情報公開へ"
	                                        : U"観測ルール：全情報公開 / クリックで偵察情報へ");
	DrawButton(Rect(Scene::Center().x - 160, Scene::Center().y + 201, 320, 42), U"方針を採用して作戦準備へ");
}
void StrategistWarScene::DrawResult() const
{
	Scene::Rect().draw(ColorF(.015, .03, .05, .88));
	Panel(RectF(Scene::Center().x - 420, Scene::Center().y - 225, 840, 450), true);
	FontAsset(U"campaignTitle")(m_battle.outcome == 1 ? U"護送作戦、完遂" : U"戦果を振り返ろう")
	    .drawAt(Scene::Center().movedBy(0, -170), ColorF(.97, .85, .55));
	FontAsset(U"campaignBody")(U"経過 {}日 / 届けた兵糧 {} / 温存した戦闘兵 {}"_fmt(
	                               m_battle.world.day, m_battle.deliveredCargo, m_battle.RemainingSoldiers()))
	    .drawAt(Scene::Center().movedBy(0, -121), ColorF(.87, .92, .89));
	(void)FontAsset(U"campaignBody")(Text(m_battle.reason))
	    .draw(RectF(Scene::Center().x - 370, Scene::Center().y - 81, 740, 68), ColorF(.96, .85, .66));
	const auto& records = m_battle.reports;
	const int first = Max(0, static_cast<int>(records.size()) - 4);
	for (int i = first; i < static_cast<int>(records.size()); ++i)
		(void)FontAsset(U"campaignSmall")(U"{}日：{}"_fmt(records[i].day, Text(records[i].text)))
		    .draw(RectF(Scene::Center().x - 370, Scene::Center().y + 7 + (i - first) * 32, 740, 30),
		          ColorF(.77, .85, .87));
	DrawButton(Rect(Scene::Center().x - 160, Scene::Center().y + 170, 320, 38),
	           U"軍議へ戻って別の作戦を試す");
}
void StrategistWarScene::Draw() const
{
	Scene::SetBackground(ColorF(.035, .055, .08));
	FontAsset(U"campaignTitle")(U"軍師戦役 / 渡河・兵糧護送").draw(20, 18, ColorF(.97, .87, .64));
	FontAsset(U"campaignSmall")(m_battle.intelligenceEnabled
	                                ? U"従軍軍師として護送方面を指揮 / 敵情は偵察と報告から判断"
	                                : U"従軍軍師として護送方面を指揮 / 比較用・全情報公開")
	    .draw(22, 65, ColorF(.73, .82, .86));
	FontAsset(U"campaignSmall")(
	    U"{}日 / 期限45日　　{}"_fmt(m_battle.world.day,
	                                 m_daysLeft > 0
	                                     ? (m_paused ? U"進行停止・閲覧中" : U"進行中・命令変更は次の再軍議")
	                                     : U"命令期・作戦を編集"))
	    .draw(302, 89, ColorF(.95, .81, .51));
	FontAsset(U"campaignSmall")(
	    m_battle.world.day < StrategistBattle::EnemyAssemblyDays
	        ? U"敵集結の目安：あと{}日"_fmt(StrategistBattle::EnemyAssemblyDays - m_battle.world.day)
	        : U"偵察で敵の動きを確認")
	    .draw(Scene::Width() - 264, 89, ColorF(.89, .67, .49));
	DrawButton(Control(4), U"メニューへ");
	DrawMap();
	Panel(RectF(16, 118, 270, Scene::Height() - 230));
	FontAsset(U"campaignBody")(U"担当する部隊").draw(28, 126, ColorF(.95, .86, .65));
	const auto& world = m_battle.world;
	for (int i = 0; i < 5; ++i)
	{
		const auto& army = world.armies[i];
		const Rect card(20, 159 + i * 72, 262, 66);
		if (i == m_selected)
			card.draw(ColorF(.16, .25, .33)).drawFrame(1.5, ColorF(.86, .71, .43));
		campaignvisual::OfficerCard(RectF(card.x + 6, card.y + 5, 36, 44), world.generals[army.general],
		                            m_faces);
		FontAsset(U"campaignSmall")(U"{} {} / {}"_fmt(i + 1, Text(world.generals[army.general].name),
		                                              String(StrategistBattle::RoleName(i))))
		    .draw(card.x + 50, card.y + 5, ColorF(.95, .90, .73));
		FontAsset(U"campaignSmall")(army.troops > 0 ? U"兵{}  士気{}"_fmt(army.troops, army.morale)
		                                            : (m_battle.disposition[i] == 1 ? U"入城" : U"敗走"))
		    .draw(card.x + 50, card.y + 25, ColorF(.73, .84, .88));
		FontAsset(U"campaignSmall")(String(StrategistBattle::StateName(m_battle.operations[i].state)) +
		                            (army.scouting ? U" / 偵察中" : U""))
		    .draw(card.x + 50, card.y + 45, ColorF(.58, .82, .91));
	}
	const auto& selected = m_battle.operations[m_selected];
	(void)FontAsset(U"campaignSmall")(selected.state == OperationState::None
	                                      ? U"右クリック：進路 / P：作戦\n点線：条件待ちの予約"
	                                      : Text(selected.status))
	    .draw(RectF(28, 529, 245, 69), ColorF(.73, .81, .85));
	const int right = Scene::Width() - 264;
	Panel(RectF(right - 4, 118, 252, Scene::Height() - 230));
	FontAsset(U"campaignBody")(U"任された目的").draw(right + 10, 132, ColorF(.97, .86, .58));
	FontAsset(U"campaignSmall")(U"兵糧 3,500以上を前線へ").draw(right + 10, 177, ColorF(.83, .89, .87));
	FontAsset(U"campaignBody")(U"{} / 3,500"_fmt(m_battle.Cargo()))
	    .draw(right + 10, 202, ColorF(.98, .86, .50));
	FontAsset(U"campaignSmall")(m_battle.deliveredCargo > 0 ? U"配送済み" : U"積荷・まだ配送していない")
	    .draw(right + 10, 233, ColorF(.69, .80, .84));
	FontAsset(U"campaignSmall")(U"戦闘兵 7,500以上を温存").draw(right + 10, 282, ColorF(.83, .89, .87));
	FontAsset(U"campaignBody")(U"{} / 7,500"_fmt(m_battle.RemainingSoldiers()))
	    .draw(right + 10, 307, ColorF(.88, .94, .82));
	(void)FontAsset(U"campaignSmall")(U"後方へ入城した戦闘兵も含む。\n輸送護衛の兵は別に扱う。")
	    .draw(RectF(right + 10, 345, 224, 67), ColorF(.70, .81, .85));
	DrawButton(Control(1), U"輸送隊に出発命令", m_daysLeft == 0 && world.armies[4].troops > 0);
	DrawButton(Control(2), U"選択隊を待機", m_daysLeft == 0);
	DrawButton(Control(3), U"後方へ撤退", m_daysLeft == 0);
	DrawButton(Control(5),
	           world.armies[m_selected].scouting
	               ? U"偵察中：固守に固定"
	               : U"構え：{} / 変更"_fmt(String(battle::Name(world.armies[m_selected].stance))),
	           m_daysLeft == 0 && m_selected < 4 && world.armies[m_selected].troops > 0 &&
	               !world.armies[m_selected].scouting);
	DrawButton(Control(6), U"作戦設定 / 合図・後退", m_daysLeft == 0 && world.armies[m_selected].troops > 0);
	DrawButton(Control(0), m_daysLeft == 0 ? U"10日進行 / Space"
	                       : m_paused      ? U"進行を再開 / Space"
	                                       : U"一時停止 / Space");
	(void)FontAsset(U"campaignSmall")(m_message).draw(
	    RectF(302, Scene::Height() - 98, Scene::Width() - 590, 38), ColorF(.84, .88, .86));
	if (!m_battle.reports.empty())
	{
		const auto& report = m_battle.reports.back();
		(void)FontAsset(U"campaignSmall")(U"報告 {}日：{}"_fmt(report.day, Text(report.text)))
		    .draw(RectF(302, Scene::Height() - 60, Scene::Width() - 590, 26), ColorF(.93, .81, .56));
	}
	FontAsset(U"campaignSmall")(U"暗い土地：監視外 / ?：過去の目撃 / 点線：予約　　P：作戦設定 / B：BGM")
	    .draw(22, Scene::Height() - 34, ColorF(.61, .73, .79));
	if (m_council)
		DrawCouncil();
	if (m_battle.outcome)
		DrawResult();
	if (m_operationOpen)
		DrawOperation();
	if (m_council || m_battle.outcome || m_operationOpen)
		DrawButton(Control(4), U"メニューへ");
}
