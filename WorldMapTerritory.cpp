#include "WorldMapScene.hpp"

namespace
{
	int PanelY() { return Min(470, Scene::Height() - 230); }
	Rect TerritoryButton(int row) { return Rect(Scene::Width() - 370, PanelY() + 80 + row * 45, 320, 36); }
}

void WorldMapScene::UpdateTerritory()
{
	if (!m_allCities) return;
	m_gameManager->EnsureTerritory(*m_allCities);
	if (m_cutInTimer > 0) return;
	const RectF mapArea(20, 20, Scene::Width() - 440, Scene::Height() - 40);
	const double scale = Min(mapArea.w / 1520.0, mapArea.h / 820.0);
	const Vec2 offset(mapArea.x + (mapArea.w - 1520.0 * scale) / 2, mapArea.y + (mapArea.h - 820.0 * scale) / 2);
	if (MouseR.down() && mapArea.mouseOver() && scale > 0)
	{
		const Vec2 world = (Cursor::PosF() - offset) / scale;
		m_selectedTerritory = TerritoryMap::Index(world.x, world.y);
		m_territoryMessage = U"開拓は隣接する未支配領地から可能";
	}
	for (int action = 0; action < 2; ++action)
	{
		if (!TerritoryButton(action).leftClicked()) continue;
		if (!TerritoryMap::Valid(m_selectedTerritory)) { m_territoryMessage = U"先に領地を右クリックしてください"; return; }
		auto& map = m_gameManager->territory;
		const auto& cell = map.cells[m_selectedTerritory];
		if (cell.city < 0 || cell.city >= static_cast<int>(m_allCities->size())) { m_territoryMessage = U"この場所は開拓対象外です"; return; }
		auto& city = (*m_allCities)[cell.city];
		if (city.owner != m_gameManager->playerFactionName || !m_gameManager->career.CanManageCity(city.name))
		{ m_territoryMessage = U"君主または担当都市の太守が操作できます"; return; }
		if (action == 0 ? !map.CanClaim(m_selectedTerritory) : !map.CanDevelop(m_selectedTerritory))
		{ m_territoryMessage = action == 0 ? U"自都市の支配領地に隣接する必要があります" : U"支配済み・開発Lv3未満の領地を選択"; return; }
		const int cost = action == 0 ? 100 : 150;
		if (city.gold < cost) { m_territoryMessage = U"担当都市の金が足りません"; return; }
		if (!m_gameManager->turnManager.ExecuteCommand()) { m_territoryMessage = U"今月のコマンドを使い切りました"; return; }
		if (action == 0) map.Claim(m_selectedTerritory); else map.Develop(m_selectedTerritory);
		city.gold -= cost;
		m_territoryMessage = U"領地を整備しました。翌月から収入が増えます";
	}
}

void WorldMapScene::DrawTerritoryPanel() const
{
	const int x = Scene::Width() - 370, y = PanelY();
	Rect(x - 10, y - 5, 350, 240).draw(ColorF(0.12, 0.10, 0.08));
	FontAsset(U"small")(U"領地：右クリックで選択").draw(x, y, Palette::Gold);
	if (TerritoryMap::Valid(m_selectedTerritory) && m_allCities)
	{
		const auto& cell = m_gameManager->territory.cells[m_selectedTerritory];
		if (cell.city >= 0 && cell.city < static_cast<int>(m_allCities->size()))
		{
			const auto& city = (*m_allCities)[cell.city];
			FontAsset(U"small")(city.name + U" / " + (TerritoryMap::Farmland(m_selectedTerritory) ? U"農村" : U"集落") + U" Lv" + Format(cell.level)).draw(x, y + 25);
			FontAsset(U"small")(U"領地月収 金{}・糧{}"_fmt(m_gameManager->territory.CityGold(cell.city), m_gameManager->territory.CityFood(cell.city))).draw(x, y + 50);
		}
	}
	const Array<String> labels{U"開拓：金100・1コマンド", U"開発：金150・1コマンド"};
	for (int i = 0; i < 2; ++i)
	{
		TerritoryButton(i).draw(TerritoryButton(i).mouseOver() ? ColorF(0.35, 0.4, 0.3) : ColorF(0.2, 0.25, 0.2));
		FontAsset(U"small")(labels[i]).drawAt(TerritoryButton(i).center());
	}
	FontAsset(U"small")(U"残り{}コマンド"_fmt(m_gameManager->turnManager.GetRemainingCommands())).draw(x, y + 165, Palette::Gold);
	(void)FontAsset(U"small")(m_territoryMessage).draw(RectF(x, y + 190, 330, 50));
}
