#include "CampaignScene.hpp"
#include "CampaignUI.hpp"
#include "CampaignVisuals.hpp"
using namespace frontline;
using namespace campaignui;
namespace
{
	int Rows()
	{
		return Max(1, Min(8, (Scene::Height() - 390) / 42));
	}
	Rect Row(int row)
	{
		return Rect(80, 238 + row * 42, 380, 38);
	}
	Rect Slot(int which)
	{
		return Rect(Scene::Width() / 2 + 24 + which * 230, 186, 220, 40);
	}
	Rect Action(bool cancel)
	{
		return Rect(Scene::Width() / 2 + 24 + (cancel ? 230 : 0), Scene::Height() - 142, 220, 38);
	}
	Rect Close()
	{
		return Rect(Scene::Width() - 250, 116, 170, 32);
	}
	Rect Page(bool next)
	{
		return Rect(next ? 280 : 80, 238 + Rows() * 42 + 12, 180, 30);
	}
} // namespace
void CampaignScene::updateTraining()
{
	if (m_governCity < 0 || m_governCity >= static_cast<int>(m_game.cities.size()))
	{
		m_councilMode = 0;
		return;
	}
	if (Close().leftClicked() || KeyEscape.down())
	{
		m_councilMode = 4;
		return;
	}
	const auto staff = m_game.Available(m_governCity);
	const int last = Max(0, (static_cast<int>(staff.size()) - 1) / Rows());
	m_trainingPage = Clamp(m_trainingPage, 0, last);
	if (Page(false).leftClicked())
		m_trainingPage = Max(0, m_trainingPage - 1);
	if (Page(true).leftClicked())
		m_trainingPage = Min(last, m_trainingPage + 1);
	for (int slot = 0; slot < 2; ++slot)
		if (Slot(slot).leftClicked())
			m_trainingSlot = slot;
	const int task = m_game.TrainingAt(m_governCity);
	if (task < 0)
		for (int row = 0; row < Rows(); ++row)
		{
			const int n = m_trainingPage * Rows() + row;
			if (n >= static_cast<int>(staff.size()))
				break;
			if (Row(row).leftClicked())
			{
				const int id = staff[n];
				if ((m_trainingSlot == 0 && id == m_trainingSecond) ||
				    (m_trainingSlot == 1 && id == m_trainingFirst))
				{
					m_message = U"別々の武将を選んでください。";
					return;
				}
				(m_trainingSlot == 0 ? m_trainingFirst : m_trainingSecond) = id;
				m_trainingSlot = 1 - m_trainingSlot;
				return;
			}
		}
	if (m_daysLeft != 0 || m_game.cities[m_governCity].owner != m_game.player)
		return;
	if (Action(false).leftClicked() && task < 0)
	{
		m_message = m_game.TrainTogether(m_governCity, m_trainingFirst, m_trainingSecond)
		                ? U"共同訓練を開始。10日後に親密度+12。"
		                : U"待機中の武将2人・金300・糧500・命令1が必要です。";
	}
	if (Action(true).leftClicked() && task >= 0)
	{
		m_game.CancelTraining(m_governCity);
		m_trainingFirst = m_trainingSecond = -1;
		m_message = U"訓練を中止。返金と親密度の成果はありません。";
	}
}
void CampaignScene::drawTraining() const
{
	if (m_governCity < 0 || m_governCity >= static_cast<int>(m_game.cities.size()))
		return;
	const auto& c = m_game.cities[m_governCity];
	const int d = Scene::Width() / 2 + 24, w = Scene::Width() - d - 88,
	          task = m_game.TrainingAt(m_governCity);
	const int first = task >= 0 ? m_game.training[task].first : m_trainingFirst,
	          second = task >= 0 ? m_game.training[task].second : m_trainingSecond;
	Scene::Rect().draw(ColorF(.015, .035, .04, .88));
	Rect(60, 102, Scene::Width() - 120, Scene::Height() - 160)
	    .rounded(8)
	    .draw(ColorF(.07, .13, .14))
	    .drawFrame(2, ColorF(.67, .60, .36));
	FontAsset(U"campaignTitle")(text(c.name) + U" / 共同訓練").draw(80, 116, ColorF(.97, .86, .61));
	DrawButton(Close(), U"都市運営へ戻る");
	FontAsset(U"campaignBody")(U"待機中の武将から2人を選択").draw(80, 190, ColorF(.9, .89, .75));
	const auto staff = m_game.Available(m_governCity);
	for (int row = 0; row < Rows(); ++row)
	{
		const int n = m_trainingPage * Rows() + row;
		if (n >= static_cast<int>(staff.size()))
			break;
		const int id = staff[n];
		const auto& g = m_game.generals[id];
		const auto r = Row(row);
		DrawButton(r, U"    {} / {}"_fmt(text(g.name), text(officer::RoleName(officer::RoleOf(g.name)))),
		           task < 0);
		campaignvisual::OfficerCard(RectF(r.x + 4, r.y + 3, 28, 32), g, m_campaignFaces);
		if (id == first || id == second)
			r.drawFrame(2, ColorF(.96, .77, .36));
	}
	if (staff.empty())
		FontAsset(U"campaignSmall")(U"待機中の武将がいません。").draw(80, 245, ColorF(.80, .85, .76));
	DrawButton(Page(false), U"前の候補", m_trainingPage > 0);
	DrawButton(Page(true), U"次の候補",
	           m_trainingPage < Max(0, (static_cast<int>(staff.size()) - 1) / Rows()));
	for (int i = 0; i < 2; ++i)
	{
		const int id = i == 0 ? first : second;
		DrawButton(Slot(i), id >= 0 ? text(m_game.generals[id].name) : U"武将{}を選択"_fmt(i + 1), task < 0);
		if (task < 0 && m_trainingSlot == i)
			Slot(i).drawFrame(2, ColorF(.97, .81, .38));
		if (id >= 0)
		{
			campaignvisual::OfficerCard(RectF(d + i * 230, 242, 100, 126), m_game.generals[id],
			                            m_campaignFaces);
			(void)FontAsset(U"campaignSmall")(
			    text(officer::RoleName(officer::RoleOf(m_game.generals[id].name))))
			    .draw(RectF(d + 110 + i * 230, 258, 108, 66), ColorF(.86, .89, .75));
		}
	}
	if (first >= 0 && second >= 0)
	{
		const int affinity = m_game.Affinity(first, second), after = Min(100, affinity + 12);
		FontAsset(U"campaignTitle")(U"親密 {} → {}"_fmt(affinity, after)).draw(d, 398, ColorF(.98, .84, .50));
		FontAsset(U"campaignBody")(
		    U"連携 Lv{} → Lv{}"_fmt(officer::BondTier(affinity), officer::BondTier(after)))
		    .draw(d, 445, ColorF(.70, .92, .80));
	}
	if (task >= 0)
		FontAsset(U"campaignBody")(U"訓練中 / あと{}日"_fmt(m_game.training[task].left))
		    .draw(d, 488, ColorF(.99, .77, .44));
	else
		FontAsset(U"campaignBody")(U"10日間 / 金300・糧500・命令1").draw(d, 488, ColorF(.92, .88, .68));
	const String help = Scene::Height() < 800
	                        ? UR"(訓練中は他の任務に任用不可。親密度の成果は10日後。
中止・城の陥落では成果・返金なし。)"
	                        : UR"(訓練中の2人は、出陣・内政・使者・異動に任用不可。
親密度は連携、共同内政、外交・謀略の協力を支える。
戦場の連携には距離・役割・士気・補給の条件も必要。
中止・城の陥落では成果なし。支払いは戻らない。)";
	(void)FontAsset(U"campaignSmall")(help).draw(
	    RectF(d, Scene::Height() < 800 ? 518 : 528, w, Scene::Height() < 800 ? 50 : 96),
	    ColorF(.79, .86, .77));
	const bool orders = m_daysLeft == 0 && c.owner == m_game.player && m_game.result == 0;
	DrawButton(Action(false), U"共同訓練を開始",
	           orders && task < 0 && first >= 0 && second >= 0 && first != second && m_game.commands > 0 &&
	               c.gold >= 300 && c.food >= 500 && m_game.Affinity(first, second) < 100);
	DrawButton(Action(true), U"訓練を中止", orders && task >= 0);
	(void)FontAsset(U"campaignSmall")(m_message).draw(
	    RectF(80, Scene::Height() - 94, Scene::Width() - 160, 40), ColorF(.95, .84, .56));
}
