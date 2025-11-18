#include "WorldMapScene.hpp"
#include "CityScene.hpp"

// =======================================================
//  コンストラクタ（2 引数版）
// =======================================================
WorldMapScene::WorldMapScene(const Faction& faction, const Array<CityData>& allCities)
	: m_playerFaction(faction)
	, m_cities(allCities)      // ★ 全都市データをコピー
{
	m_selectedCity = none;
	m_hasSelection = false;
	m_hovered = -1;
}


// =======================================================
//  update()
// =======================================================
void WorldMapScene::update()
{
	m_hovered = -1;

	for (int i = 0; i < m_cities.size(); ++i)
	{
		const auto& c = m_cities[i];

		if (Circle(c.pos, 20).mouseOver())
		{
			m_hovered = i;

			// 自勢力の都市だけ選択可能
			if (MouseL.down() && c.owner == m_playerFaction.name)
			{
				m_selectedCity = c;     // Optional に保存
				m_hasSelection = true;

				m_isEnd = true;
				m_nextScene = U"City";  // CityScene へ
				return;
			}
		}
	}
}


// =======================================================
//  draw()
// =======================================================
void WorldMapScene::draw() const
{
	Scene::SetBackground(ColorF{ 0.2, 0.25, 0.2 });

	// ----------------------------
	//  ● 都市の描画
	// ----------------------------
	for (const auto& c : m_cities)
	{
		bool myCity = (c.owner == m_playerFaction.name);

		// 色：自軍は勢力色、それ以外は薄色
		ColorF dotColor = myCity
			? m_playerFaction.color
			: ColorF(c.color, 0.4);

		Circle{ c.pos, 16 }.draw(dotColor);
		FontAsset(U"small")(c.name).drawAt(c.pos.x, c.pos.y - 25);
	}

	// ----------------------------
	//  ● 都市にカーソルが乗った時の情報
	// ----------------------------
	if (m_hovered >= 0)
	{
		const auto& c = m_cities[m_hovered];

		RectF(20, 20, 350, 140).draw(ColorF(0, 0, 0, 0.6));

		FontAsset(U"small")(U"都市: ").draw(30, 30);
		FontAsset(U"small")(c.name).draw(120, 30);

		FontAsset(U"small")(U"統治者: ").draw(30, 55);
		FontAsset(U"small")(c.owner).draw(120, 55);

		FontAsset(U"small")(U"金: " + Format(c.gold)).draw(30, 80);
		FontAsset(U"small")(U"兵: " + Format(c.troops)).draw(150, 80);

		FontAsset(U"small")(U"食料: " + Format(c.food)).draw(30, 105);
		FontAsset(U"small")(U"治安: " + Format(c.order)).draw(150, 105);

		bool myCity = (c.owner == m_playerFaction.name);
		FontAsset(U"small")(
			myCity ? U"左クリックで内政画面へ" : U"他勢力の都市"
		).draw(30, 130, Palette::White);
	}
}
