#include "CampaignScene.hpp"
using namespace frontline;

int CampaignScene::returnTarget() const
{
	if (m_army < 0 || m_army >= static_cast<int>(m_game.armies.size()))
		return -1;
	const auto& a = m_game.armies[m_army];
	if (m_returnArmy == m_army && m_returnCity >= 0 &&
	    m_returnCity < static_cast<int>(m_game.cities.size()) &&
	    m_game.cities[m_returnCity].owner == a.faction)
		return m_returnCity;
	return m_game.NearestCity(a.tile, a.faction, false);
}
void CampaignScene::updateReturnOrders()
{
	if (button(8).leftClicked())
	{
		const int from = returnTarget(), count = static_cast<int>(m_game.cities.size());
		for (int step = 1; step <= count; ++step)
		{
			const int c = (from + step + count) % count;
			if (m_game.cities[c].owner == m_game.player)
			{
				m_returnCity = c;
				m_returnArmy = m_army;
				break;
			}
		}
	}
	if (button(44).leftClicked())
	{
		const int city = returnTarget();
		if (city < 0 || !m_game.ReturnToCity(m_army, city))
			m_message = U"帰還先までの経路がありません。自勢力の都市を選んでください。";
		else if (m_game.armies[m_army].troops <= 0)
		{
			m_city = city;
			m_army = -1;
			m_message = U"入城しました。兵・残りの兵糧・武将を都市へ戻しました。";
		}
		else
			m_message = U"帰還命令。敵との交戦で足を止めず、選んだ都市へ撤退します。";
	}
}
