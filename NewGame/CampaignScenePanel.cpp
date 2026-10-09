#include "CampaignScene.hpp"
#include "CampaignSave.hpp"
#include "CampaignUI.hpp"
#include "CampaignVisuals.hpp"

using namespace frontline;
using namespace campaignui;

void CampaignScene::drawPanel() const
{
	const int x = Scene::Width() - 330;
	Rect(x, 86, 306, Scene::Height() - 106)
	    .rounded(3)
	    .draw(ColorF(.075, .12, .10))
	    .drawFrame(1, ColorF(.57, .49, .31));
	const auto line = [&](const String& label, int y, ColorF color = ColorF(0.87, 0.90, 0.86)) {
		FontAsset(U"campaignBody")(label).draw(x + 14, y, color);
	};
	const bool orders = (m_daysLeft == 0 || (!m_game.hexMap && m_paused)) && m_game.result == 0;
	if (m_army >= 0 && m_army < static_cast<int>(m_game.armies.size()) && m_game.armies[m_army].troops > 0)
	{
		const auto& army = m_game.armies[m_army];
		campaignvisual::OfficerCard(RectF(x + 14, 102, 56, 66), m_game.generals[army.general],
		                            m_campaignFaces);
		(void)FontAsset(U"campaignBody")(text(m_game.generals[army.general].name) + U"隊 / " +
		                                 text(Campaign::ArmName(army.arm)))
		    .draw(RectF(x + 82, 105, 210, 30), ColorF(.94, .86, .65));
		FontAsset(U"campaignSmall")(U"兵力 {}　士気 {}"_fmt(army.troops, army.morale))
		    .draw(x + 82, 145, ColorF(.8, .86, .75));
		line(army.arm == Arm::Transport ? U"積荷 {} / 携行糧 {}"_fmt(army.cargoFood, army.food)
		                                : U"携行兵糧 {}"_fmt(army.food),
		     178);
		line(army.supplied ? U"補給：都市と接続" : U"補給：途絶（携行糧を消費）", 211,
		     army.supplied ? ColorF(0.5, 0.88, 0.65) : ColorF(1.0, 0.62, 0.36));
		FontAsset(U"campaignSmall")(army.aiAssemblyDays > 0
		                                ? U"作戦：集結待ち {}/20日"_fmt(army.aiAssemblyDays)
		                                : U"進路：残り{}マス"_fmt(army.path.size()))
		    .draw(x + 14, 239, ColorF(.87, .8, .57));
		const bool own = army.faction == m_game.player;
		const int home = returnTarget();
		DrawButton(button(8),
		           home < 0 ? U"帰還先なし" : U"帰還先：" + text(m_game.cities[home].name) + U" / 切替",
		           own && orders && home >= 0);
		DrawButton(button(44),
		           home >= 0 && army.tile == m_game.cities[home].tile ? U"入城する" : U"帰還を開始",
		           own && orders && home >= 0);
		DrawButton(button(9), army.arm == Arm::Transport ? U"輸送停止" : U"現在地を守備", own && orders);
		if (army.arm == Arm::Transport)
		{
			const int target = m_game.CityAt(army.target);
			String detail = target < 0 ? U"輸送停止中" : U"配送先：" + text(m_game.cities[target].name);
			detail += U"\n護衛が損耗すると積荷も失います。\n味方の土地を通り、都市へ届けます。\n戦闘部隊を近"
			          U"くに置いて護衛を。\n到着すると護衛と残りの糧も入城。\n右クリック：味方都市へ配送先変"
			          U"更\n経路が切れたら停止・5日ごと再探索。";
			(void)FontAsset(U"campaignSmall")(detail).draw(RectF(x + 14, 344, 280, 250),
			                                               ColorF(.87, .8, .57));
		}
		else
		{
			const auto& g = m_game.generals[army.general];
			DrawButton(button(39),
			           army.tacticQueued ? U"戦法予約済み"
			                             : text(TacticName(g.tactic)) +
			                                   U" / 待ち{}日"_fmt(Max(0, army.tacticReadyDay - m_game.day)),
			           own && orders && !army.tacticQueued && army.tacticReadyDay <= m_game.day &&
			               army.morale >= 30);

			DrawButton(button(43), U"構え：{} / 切替・命令1"_fmt(String(battle::Name(army.stance))),
			           own && orders && m_game.commands > 0);
			const auto link = m_game.Formation(m_army);
			(void)FontAsset(U"campaignSmall")(
			    U"構え：攻撃 {}% / 被害 {}%\n射程 {} / 林と山の背後には射撃不可\n包囲 {}方向 / 被害+{}%・士気-{}/日\n連携：攻撃+{}% / 被害-{}%\n兵糧消費-{}% / 士気+{}/日\n役割：{} / 個性：{}\n戦法：士気15消費・再使用30日"_fmt(
			        battle::AttackPercent(army.stance), battle::DamagePercent(army.stance),
			        m_game.AttackRange(m_army), m_game.PressureDirections(m_army),
			        m_game.PressureDamagePercent(m_army), m_game.PressureMoraleLoss(m_army), link.attack,
			        link.defense, link.supply, link.morale / 2,
			        text(officer::RoleName(officer::RoleOf(g.name))), text(TraitName(g.trait))))
			    .draw(RectF(x + 14, 434, 280, 174), ColorF(.74, .81, .76));
		}
	}
	else if (m_region >= 0 && m_region < static_cast<int>(m_game.regions.size()))
	{
		drawRegionPanel();
	}
	else if (m_city >= 0)
	{
		const auto& city = m_game.cities[m_city];
		const bool own = city.owner == m_game.player;
		line(text(city.name) + U" / " + text(Campaign::FactionName(city.owner)), 105,
		     FactionColor(city.owner));
		line(U"守備兵 {}"_fmt(city.troops), 145);
		line(U"金 {}"_fmt(city.gold), 180);
		line(U"兵糧 {}"_fmt(city.food), 215);
		DrawButton(button(13), m_tab == 0 ? U"● 軍務" : U"軍務");
		DrawButton(button(14), m_tab == 1 ? U"● 内政" : U"内政");
		DrawButton(button(40), m_tab == 2 ? U"● 輸送" : U"輸送");
		const auto available = m_game.Available(m_city);
		const String name = available.empty()
		                        ? U"出陣可能な武将なし"
		                        : text(m_game.generals[available[m_generalChoice % available.size()]].name);
		DrawButton(button(1), name + U"　切替", own && orders && !available.empty());
		if (m_tab == 2)
			drawTransport();
		else if (m_tab == 1)
		{
			FontAsset(U"campaignSmall")(U"農政 {} / 商業 {} / 治安 {} / 兵站 {}"_fmt(
			                                city.farming, city.commerce, city.order, city.logistics))
			    .draw(x + 14, 324, ColorF(0.87, 0.90, 0.86));
			FontAsset(U"campaignSmall")(
			    U"月収：金 {} / 糧 {}"_fmt((100 + city.commerce * 15) * city.order / 100,
			                               (800 + city.farming * 80) * city.order / 100))
			    .draw(x + 14, 346, ColorF(0.93, 0.83, 0.57));
			for (int d = 0; d < 4; ++d)
			{
				const auto duty = static_cast<Duty>(d);
				const int gain = available.empty()
				                     ? 0
				                     : m_game.WorkGain(available[m_generalChoice % available.size()], duty);
				DrawButton(button(3 + d), text(Campaign::DutyName(duty)) + U" +{}"_fmt(gain),
				           own && orders && city.worker < 0 && !available.empty());
			}
			DrawButton(button(7), city.worker >= 0 ? U"事業を中止（返金なし）" : U"金500 / 30日 / 命令1",
			           own && orders && city.worker >= 0);
			String detail;
			if (!available.empty())
			{
				const int g = available[m_generalChoice % available.size()];
				const auto& officer = m_game.generals[g];
				detail =
				    U"統率 {} / 政治 {} / 得意：{}\n得意な仕事は開発量 +8。兵站型は\n出陣時にも兵糧の消費を抑えます。"_fmt(
				        officer.leadership, officer.politics, text(Campaign::DutyName(officer.specialty)));
			}
			if (city.worker >= 0)
				detail += U"\n{}：{} / 残り{}日"_fmt(text(m_game.generals[city.worker].name),
				                                     text(Campaign::DutyName(static_cast<Duty>(city.work))),
				                                     city.workLeft);
			(void)FontAsset(U"campaignSmall")(detail).draw(RectF(x + 14, 503, 280, 100),
			                                               ColorF(0.76, 0.85, 0.80));
		}
		else
		{
			DrawButton(button(2), U"出陣兵数 {}　切替"_fmt(m_soldiers), own && orders);
			DrawButton(button(3), U"槍兵で出陣", own && orders);
			DrawButton(button(4), U"弓兵で出陣", own && orders);
			DrawButton(button(5), U"攻城隊で出陣", own && orders);
			DrawButton(button(6), U"騎兵で出陣", own && orders);
			DrawButton(button(7), U"募兵 +2000", own && orders);
			FontAsset(U"campaignSmall")(
			    U"出陣：金 {} / 携行糧 {}\n出陣士気 {}（治安で変化）\n兵站が高いと多くの糧を携行。\n携行糧は都市から支払います。\n募兵は治安を10消費。"_fmt(
			        m_soldiers / 10, officer::SupplyPack(m_soldiers, city.logistics),
			        officer::StartingMorale(city.order)))
			    .draw(x + 14, 503, ColorF(0.68, 0.77, 0.74));
		}
	}
	else
	{
		line(U"戦況を見て、作戦を立てる", 107, ColorF(0.93, 0.83, 0.57));
		(void)FontAsset(U"campaignBody")(
		    U"都市を選択 → 出陣\n部隊選択 → 右クリックで進路\nEnter → "
		    U"全勢力の10日間を進行\n\nShift+"
		    U"左：都市を選択\n同じマスはクリックで部隊切替\nTab：補給可能な土地を表示\n\n槍兵：野戦の基本\n弓"
		    U"兵：2マス先へ射撃\n騎兵：平地で速く強い\n攻城隊：城に強い、野戦に弱い")
		    .draw(RectF(x + 14, 154, 280, Max(330, Scene::Height() - 350)), ColorF(0.75, 0.82, 0.77));
	}
	DrawButton(button(10), U"保存", m_daysLeft == 0);
	DrawButton(button(11), U"読込", m_daysLeft == 0);
	DrawButton(button(12), U"勢力選択へ", m_daysLeft == 0);
	(void)FontAsset(U"campaignSmall")(m_message).draw(RectF(x + 14, Scene::Height() - 84, 280, 60),
	                                                  ColorF(0.94, 0.83, 0.55));
}
