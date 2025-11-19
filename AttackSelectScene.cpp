#include "AttackSelectScene.hpp"

AttackSelectScene::AttackSelectScene(const CityData& from, const Array<CityData>& all)
	: m_fromCity(from)
	, m_allCities(all)
{
}

void AttackSelectScene::update()
{
	// 侵攻先を選択
	for (const auto& city : m_allCities)
	{
		// 同勢力には攻められない
		if (city.owner == m_fromCity.owner)
			continue;

		RectF button(city.pos, Vec2{ 120, 40 });

		if (button.mouseOver() && MouseL.down())
		{
			m_targetCity = city;
		}
	}

	// Enter で決定して ArmyConfigScene へ
	if (m_targetCity && KeyEnter.down())
	{
		m_sceneEnd = true;
		m_nextScene = U"ArmyConfigScene";
	}
}

void AttackSelectScene::draw() const
{
	Scene::SetBackground(ColorF(0.15));

	FontAsset(U"medium")(U"侵攻先を選択してください").drawAt(400, 80);

	// 攻撃対象候補
	for (const auto& city : m_allCities)
	{
		if (city.owner == m_fromCity.owner)
			continue;

		RectF button(city.pos, Vec2{ 120,40 });
		bool hov = button.mouseOver();

		button.draw(hov ? ColorF(0.5, 0.3, 0.3) : ColorF(0.3, 0.3, 0.3));
		button.drawFrame(2);
		FontAsset(U"small")(city.name).drawAt(button.center());
	}

	// 選択済み表示
	if (m_targetCity)
	{
		FontAsset(U"medium")(U"目標: {}"_fmt(m_targetCity->name))
			.draw(40, 500, Palette::Yellow);
		FontAsset(U"small")(U"Enterで決定").draw(40, 540, Palette::Gray);
	}
}
