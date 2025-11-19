#include "AttackSelectScene.hpp"

AttackSelectScene::AttackSelectScene(const CityData& from, const Array<CityData>& allCities)
	: m_fromCity(from)
	, m_allCities(allCities)
{
	// 隣接都市（距離 ≦ 220）を抽出
	for (int i = 0; i < m_allCities.size(); i++)
	{
		if (m_allCities[i].name == m_fromCity.name) continue;

		double dist = m_fromCity.pos.distanceFrom(m_allCities[i].pos);

		if (dist <= 220)
		{
			m_neighbors << i;
		}
	}
}

void AttackSelectScene::update()
{
	int y = 150;

	for (int idx : m_neighbors)
	{
		RectF btn{ 400, y, 300, 40 };

		if (btn.leftClicked())
		{
			// 侵攻先確定 → 戦闘へ
			m_isEnd = true;
			m_nextScene = U"BattleScene";
			return;
		}

		y += 60;
	}

	if (KeyEscape.down())
	{
		m_isEnd = true;
		m_nextScene = U"City";
		return;
	}
}

void AttackSelectScene::draw() const
{
	FontAsset(U"title")(m_fromCity.name + U" からの侵攻先選択").drawAt(Scene::Center().x, 50);

	int y = 150;

	for (int idx : m_neighbors)
	{
		const auto& c = m_allCities[idx];
		RectF btn{ 400, y, 300, 40 };

		btn.draw(ColorF(0.2, 0.2, 0.4));
		FontAsset(U"menu")(c.name).drawAt(btn.center(), Palette::White);

		y += 60;
	}

	FontAsset(U"menu")(U"Escで戻る").drawAt(Scene::Center().x, 600);
}
