#include "WorldMapScene.hpp"
#include "CityScene.hpp"

WorldMapScene::WorldMapScene(const Faction& faction,
							 Array<CityData>* allCities,
							 const String& startCityName)
	: m_playerFaction(faction)
	, m_allCities(allCities)
	, m_startCityName(startCityName)
	, m_gameClear(false)        // ★★ 必須 ★★
	, m_clearTimer(0.0)         // ★★ 必須 ★★
	, m_hovered(-1)
	, m_selectedIndex(-1)
{
}


void WorldMapScene::update()
{
	//-----------------------------------------
	// ★ 一都市でも占領したらゲームクリア
	//-----------------------------------------
	if (!m_gameClear)
	{
		for (const auto& c : *m_allCities)
		{
			// ★ 最初の所有者と違うなら「奪った都市」
			if (c.owner == m_playerFaction.name &&
				c.initialOwner != m_playerFaction.name)
			{
				m_gameClear = true;
				break;
			}
		}
	}


	//-----------------------------------------
	// ★ クリア時（操作ロック）
	//-----------------------------------------
	if (m_gameClear)
	{
		// 背景をうっすら暗く
		RectF(Scene::Rect()).draw(ColorF(0, 0, 0, 0.75));

		// 大きなタイトル
		FontAsset(U"huge")(U"都市を占領！")
			.drawAt(Scene::Center().movedBy(0, -40), Palette::Yellow);

		// サブテキスト
		FontAsset(U"medium")(U"ゲームクリア！！")
			.drawAt(Scene::Center().movedBy(0, 40), Palette::White);

		// フェード効果（任意）
		double a = Min(m_clearTimer / 3.0, 1.0);
		RectF(Scene::Rect())
			.draw(ColorF(0, 0, 0, 0.4 * a));

		return;
	}


	//-----------------------------------------
	// ★ 通常処理
	//-----------------------------------------
	m_hovered = -1;

	for (int i = 0; i < m_allCities->size(); ++i)
	{
		const CityData& c = (*m_allCities)[i];

		if (Circle(c.pos, 20).mouseOver())
		{
			m_hovered = i;

			if (MouseL.down() && c.owner == m_playerFaction.name)
			{
				m_selectedIndex = i;
				m_sceneEnd = true;
				m_nextScene = U"City";
				return;
			}
		}
	}
}

void WorldMapScene::draw() const
{
	Scene::SetBackground(ColorF{ 0.2, 0.25, 0.2 });

	//-----------------------------------------
	// ★ クリア表示
	//-----------------------------------------
	if (m_gameClear)
	{
		FontAsset(U"huge")(U"都市を占領！").drawAt(Scene::Center(), Palette::Yellow);
		FontAsset(U"medium")(U"ゲームクリア！！").drawAt(Scene::Center().movedBy(0, 60));
		return;
	}

	//-----------------------------------------
	// ★ 都市表示
	//-----------------------------------------
	for (const auto& c : *m_allCities)
	{
		bool myCity = (c.owner == m_playerFaction.name);

		ColorF dotColor = myCity
			? ColorF(m_playerFaction.color)
			: ColorF(c.color).withA(0.4);

		Circle{ c.pos, 16 }.draw(dotColor);
		FontAsset(U"small")(c.name).drawAt(c.pos.x, c.pos.y - 25);
	}

	//-----------------------------------------
	// ★ ホバー情報
	//-----------------------------------------
	if (m_hovered >= 0)
	{
		const CityData& c = (*m_allCities)[m_hovered];
		RectF(20, 20, 350, 140).draw(ColorF(0, 0, 0, 0.6));

		FontAsset(U"small")(U"都市: " + c.name).draw(30, 30);
		FontAsset(U"small")(U"統治者: " + c.owner).draw(30, 55);
		FontAsset(U"small")(U"金: " + Format(c.gold)).draw(30, 80);
		FontAsset(U"small")(U"兵: " + Format(c.troops)).draw(150, 80);
		FontAsset(U"small")(U"食料: " + Format(c.food)).draw(30, 105);
		FontAsset(U"small")(U"治安: " + Format(c.order)).draw(150, 105);
	}
}

Optional<std::reference_wrapper<CityData>> WorldMapScene::getSelectedCityRef()
{
	if (m_selectedIndex < 0)
		return none;

	return (*m_allCities)[m_selectedIndex];
}
