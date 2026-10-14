#include "StrategistWarScene.hpp"
#include "CampaignUI.hpp"

using namespace frontline;
using namespace campaignui;

Rect StrategistWarScene::OperationControl(int id) const
{
	const int x = Scene::Center().x - 460, y = Scene::Center().y - 267;
	if (id < 12)
		return Rect(x + 24 + (id % 3) * 290, y + 98 + (id / 3) * 53, 276, 36);
	return Rect(x + (id == 12 ? 24 : 604), y + 475, 276, 38);
}
void StrategistWarScene::OpenOperation()
{
	const auto& existing = m_battle.operations[m_selected];
	m_draft = existing.state == OperationState::None ? BattleOperation{} : existing;
	if (existing.state == OperationState::None)
	{
		m_draft.target = m_battle.world.armies[m_selected].target;
		m_draft.startDay = m_battle.world.day + 1;
		m_draft.signalArmy = m_selected == 0 ? 1 : 0;
		m_draft.signalTile = m_battle.world.armies[m_draft.signalArmy].target;
		m_draft.watchTile = m_draft.target;
		m_draft.minTroops = m_selected == StrategistBattle::Convoy ? 0 : 1500;
		m_draft.minMorale = 35;
		m_draft.fallback = m_battle.world.cities[0].tile;
	}
	m_taskKind = m_draft.retreat ? 2 : m_draft.scout ? 1 : 0;
	if (m_draft.signalArmy < 0 || m_draft.signalArmy > StrategistBattle::Convoy ||
	    m_draft.signalArmy == m_selected)
		m_draft.signalArmy = m_selected == 0 ? 1 : 0;
	if (m_draft.signalTile < 0)
		m_draft.signalTile = m_battle.world.armies[m_draft.signalArmy].target;
	if (m_draft.watchTile < 0)
		m_draft.watchTile = m_draft.target;
	m_operationOpen = true;
	m_pickPurpose = 0;
}
void StrategistWarScene::UpdateOperation()
{
	if (OperationControl(0).leftClicked() && m_selected < StrategistBattle::Convoy)
	{
		m_taskKind = (m_taskKind + 1) % 3;
		m_draft.scout = m_taskKind == 1;
		m_draft.retreat = m_taskKind == 2;
	}
	if (OperationControl(2).leftClicked())
	{
		m_draft.trigger = static_cast<OperationTrigger>((static_cast<int>(m_draft.trigger) + 1) % 4);
		if (m_draft.trigger == OperationTrigger::Date) m_draft.startDay = Max(m_draft.startDay, m_battle.world.day + 1);
	}
	if (OperationControl(3).leftClicked() && m_draft.trigger == OperationTrigger::Date)
	{
		m_draft.startDay += 5;
		if (m_draft.startDay > StrategistBattle::Deadline)
			m_draft.startDay = m_battle.world.day + 1;
	}
	if (OperationControl(4).leftClicked() && m_draft.trigger == OperationTrigger::AllyArrival)
	{
		m_draft.signalArmy = (m_draft.signalArmy + 1) % 5;
		if (m_draft.signalArmy == m_selected)
			m_draft.signalArmy = (m_draft.signalArmy + 1) % 5;
		const auto& source = m_battle.operations[m_draft.signalArmy];
		m_draft.signalTile = source.state == OperationState::None
		                         ? m_battle.world.armies[m_draft.signalArmy].target
		                         : source.target;
	}
	if (OperationControl(6).leftClicked())
		m_draft.minTroops = m_draft.minTroops == 0 ? 1500 : m_draft.minTroops == 1500 ? 2000 : 0;
	if (OperationControl(7).leftClicked())
		m_draft.minMorale = m_draft.minMorale == 0 ? 30 : m_draft.minMorale == 30 ? 40 : 0;
	if (OperationControl(8).leftClicked())
	{
		m_draft.deadline = m_draft.deadline == 0    ? 10
		                   : m_draft.deadline == 10 ? 20
		                   : m_draft.deadline == 20 ? 30
		                   : m_draft.deadline == 30 ? 45
		                                            : 0;
	}
	if (OperationControl(10).leftClicked())
		m_draft.fallback = m_draft.fallback == -1 ? m_battle.world.cities[0].tile : -1;
	int purpose = 0;
	if (OperationControl(1).leftClicked())
		purpose = 1;
	if (OperationControl(9).leftClicked() && m_draft.fallback >= 0)
		purpose = 2;
	if (OperationControl(5).leftClicked() && m_draft.trigger == OperationTrigger::EnemySighted)
		purpose = 3;
	if (OperationControl(11).leftClicked() && m_draft.trigger == OperationTrigger::AllyArrival)
		purpose = 4;
	if (purpose)
	{
		m_pickPurpose = purpose;
		m_operationOpen = false;
		m_message = U"地図を左クリックして地点を指定。右クリックで選択を取り消す。";
	}
	if (OperationControl(12).leftClicked())
	{
		m_operationOpen = false;
		m_pickPurpose = 0;
	}
	if (OperationControl(13).leftClicked())
	{
		if (m_battle.QueueOperation(m_selected, m_draft))
		{
			m_operationOpen = false;
			m_message = U"作戦を予約。開始条件と中止条件は、日ごとに同じ情報から評価する。";
		}
		else
			m_message = U"予約できません。目的地、合図元、循環した合図や後退先を確認してください。";
	}
}
void StrategistWarScene::DrawOperation() const
{
	Scene::Rect().draw(ColorF(.015, .035, .06, .88));
	const RectF panel(Scene::Center().x - 460, Scene::Center().y - 267, 920, 535);
	Panel(panel, true);
	FontAsset(U"campaignTitle")(U"作戦設定 / {}"_fmt(Text(m_battle.world.generals[m_selected].name)))
	    .draw(panel.x + 24, panel.y + 18, ColorF(.97, .85, .54));
	FontAsset(U"campaignSmall")(U"地点は地図で指定。開始条件が成立するまで、この隊は現在地で待機する。")
	    .draw(panel.x + 24, panel.y + 66, ColorF(.77, .84, .86));
	const auto point = [](int tile) {
		return tile < 0 ? U"なし" : U"({},{})"_fmt(tile % Width, tile / Width);
	};
	const String task = m_taskKind == 1 ? U"偵察" : m_taskKind == 2 ? U"後退" : U"移動・守備";
	DrawButton(OperationControl(0), U"任務：{}"_fmt(task), m_selected < StrategistBattle::Convoy);
	DrawButton(OperationControl(1), U"目的地 {}"_fmt(point(m_draft.target)));
	DrawButton(OperationControl(2), U"開始：{}"_fmt(String(StrategistBattle::TriggerName(m_draft.trigger))));
	DrawButton(OperationControl(3), U"指定日：{}日 / +5"_fmt(m_draft.startDay),
	           m_draft.trigger == OperationTrigger::Date);
	DrawButton(OperationControl(4), U"合図元：{}"_fmt(Text(m_battle.world.generals[m_draft.signalArmy].name)),
	           m_draft.trigger == OperationTrigger::AllyArrival);
	DrawButton(OperationControl(5), U"監視地点 {}"_fmt(point(m_draft.watchTile)),
	           m_draft.trigger == OperationTrigger::EnemySighted);
	DrawButton(OperationControl(6),
	           m_draft.minTroops == 0 ? U"兵数の中止条件：なし" : U"中止：兵 {}以下"_fmt(m_draft.minTroops));
	DrawButton(OperationControl(7), m_draft.minMorale == 0 ? U"士気の中止条件：なし"
	                                                       : U"中止：士気 {}以下"_fmt(m_draft.minMorale));
	DrawButton(OperationControl(8),
	           m_draft.deadline == 0 ? U"任務期限：なし" : U"任務期限：{}日"_fmt(m_draft.deadline));
	DrawButton(OperationControl(9), U"後退先 {}"_fmt(point(m_draft.fallback)), m_draft.fallback >= 0);
	DrawButton(OperationControl(10),
	           m_draft.fallback < 0 ? U"中止時：現在地で待機" : U"中止時：後退先へ撤退");
	DrawButton(OperationControl(11), U"合図地点 {}"_fmt(point(m_draft.signalTile)),
	           m_draft.trigger == OperationTrigger::AllyArrival);
	(void)FontAsset(U"campaignBody")(
	    U"合図は指定した地点への到着後、翌日の判断から有効。\n敵の確認条件は、現在目撃している敵だけで判定す"
	    U"る。\n偵察は確認範囲が3から5マスへ広がる。固守で行軍し、攻撃力は半減。")
	    .draw(RectF(panel.x + 24, panel.y + 320, 872, 102), ColorF(.85, .89, .85));
	(void)FontAsset(U"campaignSmall")(m_message).draw(RectF(panel.x + 24, panel.y + 426, 872, 37),
	                                                  ColorF(.92, .77, .48));
	DrawButton(OperationControl(12), U"取り消す / Esc");
	DrawButton(OperationControl(13), U"この作戦を予約する");
}
