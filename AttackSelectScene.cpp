#include "AttackSelectScene.hpp"

AttackSelectScene::AttackSelectScene(int fromIndex, Array<CityData>* allCities)
	: m_fromCityIndex(fromIndex)        // ←★ 必須！
	, m_allCities(allCities)
{
	m_fromCity = &((*m_allCities)[fromIndex]);
}


void AttackSelectScene::update()
{
	// -----------------------------------
	// 侵攻先選択（コピー禁止版）
	// -----------------------------------
	for (int i = 0; i < m_allCities->size(); ++i)
	{
		CityData& city = (*m_allCities)[i];

		// 同勢力へは攻めない
		if (city.owner == m_fromCity->owner)
			continue;

		RectF button(city.pos, Vec2{ 120, 40 });

		if (button.mouseOver() && MouseL.down())
		{
			m_targetIndex = i;  // ★ コピーなし、安全
		}
	}

	// -----------------------------------
	// Enter で決定 → ArmyConfigScene
	// -----------------------------------
	if (m_targetIndex && KeyEnter.down())
	{
		m_sceneEnd = true;
		m_nextScene = U"ArmyConfigScene";
	}
}

void AttackSelectScene::draw() const
{
	Scene::SetBackground(ColorF(0.15));
	FontAsset(U"medium")(U"侵攻先を選択してください").drawAt(400, 80);

	for (int i = 0; i < m_allCities->size(); ++i)
	{
		const CityData& city = (*m_allCities)[i];

		if (city.owner == m_fromCity->owner)
			continue;

		RectF button(city.pos, Vec2{ 120,40 });
		bool hov = button.mouseOver();

		button.draw(hov ? ColorF(0.5, 0.3, 0.3) : ColorF(0.3, 0.3, 0.3));
		button.drawFrame(2);
		FontAsset(U"small")(city.name).drawAt(button.center());
	}

	if (m_targetIndex)
	{
		const CityData& t = (*m_allCities)[m_targetIndex.value()];
		FontAsset(U"medium")(U"目標: {}"_fmt(t.name))
			.draw(40, 500, Palette::Yellow);

		FontAsset(U"small")(U"Enterで決定")
			.draw(40, 540, Palette::Gray);
	}
}
