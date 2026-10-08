#include "CampaignScene.hpp"
#include "CampaignUI.hpp"
using namespace frontline;
using namespace campaignui;

int CampaignScene::transportTarget() const
{
	if (m_city < 0)
		return -1;
	if (m_transportTarget >= 0 && m_transportTarget < static_cast<int>(m_game.cities.size()) &&
	    m_transportTarget != m_city && m_game.cities[m_transportTarget].owner == m_game.player)
		return m_transportTarget;
	for (int c = 0; c < static_cast<int>(m_game.cities.size()); ++c)
		if (c != m_city && m_game.cities[c].owner == m_game.player)
			return c;
	return -1;
}
void CampaignScene::updateTransport()
{
	if (button(2).leftClicked())
		m_cargo = m_cargo == 5000 ? 10000 : m_cargo == 10000 ? 20000 : 5000;
	if (button(41).leftClicked())
	{
		const int target = transportTarget(), count = static_cast<int>(m_game.cities.size());
		for (int step = 1; step <= count; ++step)
		{
			const int c = (target + step + count) % count;
			if (c != m_city && m_game.cities[c].owner == m_game.player)
			{
				m_transportTarget = c;
				break;
			}
		}
	}
	if (button(42).leftClicked())
	{
		const auto available = m_game.Available(m_city);
		const int target = transportTarget();
		const int a = available.empty()
		                  ? -1
		                  : m_game.DispatchTransport(m_city, available[m_generalChoice % available.size()],
		                                             target, m_cargo);
		if (a < 0)
			m_message = U"空いた武将・護衛1000・金100・積荷と携行糧・命令1・安全な味方経路が必要です。";
		else
		{
			m_army = a;
			m_city = -1;
			m_message = U"輸送開始。戦闘部隊で街道を守り、前線の兵糧を支えましょう。";
		}
	}
}
void CampaignScene::drawTransport() const
{
	const int x = Scene::Width() - 330, target = transportTarget();
	const auto& city = m_game.cities[m_city];
	const auto available = m_game.Available(m_city);
	const bool own = city.owner == m_game.player;
	const bool orders = (m_daysLeft == 0 || m_paused) && m_game.result == 0 && own;
	const int pack = officer::SupplyPack(1000, city.logistics);

	if (m_transportEstimateFrom != m_city || m_transportEstimateTo != target ||
	    m_transportEstimateDay != m_game.day || m_transportEstimateRevision != m_game.revision)
	{
		m_transportEstimateFrom = m_city;
		m_transportEstimateTo = target;
		m_transportEstimateDay = m_game.day;
		m_transportEstimateRevision = m_game.revision;
		m_transportEstimate =
		    target < 0 ? -1 : m_game.TransportDays(city.tile, m_game.cities[target].tile, city.owner);
	}
	const int days = m_transportEstimate;

	DrawButton(button(2), U"運ぶ兵糧 {}　切替"_fmt(m_cargo), orders);
	DrawButton(button(41),
	           target < 0 ? U"配送先なし" : U"配送先：" + text(m_game.cities[target].name) + U"　切替",
	           orders && target >= 0);
	FontAsset(U"campaignSmall")(
	    days < 0 ? U"配送路なし：味方の街道をつなげよう"
	             : U"平常時の到着目安：{}日 / 先方の糧 {}"_fmt(days, m_game.cities[target].food))
	    .draw(x + 14, 418, days < 0 ? ColorF(1, .6, .4) : ColorF(.87, .8, .57));
	DrawButton(button(42), U"兵糧輸送を開始",
	           orders && !available.empty() && days >= 0 && city.troops >= 2000 && city.gold >= 100 &&
	               city.food >= m_cargo + pack && m_game.commands > 0 &&
	               m_game.ArmyCount(city.owner) < Campaign::MaxArmies);
	(void)FontAsset(U"campaignSmall")(
	    U"護衛1000 / 金100 / 命令1\n積荷 {} + 護衛の携行糧 {}\n味方の土地のみを通る輸送隊。\n襲撃・飢えで護衛を失うと積荷も減少。\n到着後、担当武将も配送先に駐在。"_fmt(
	        m_cargo, pack))
	    .draw(RectF(x + 14, 503, 280, 112), ColorF(.76, .85, .8));
}
