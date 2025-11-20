#include "ArmyConfigScene.hpp"

ArmyConfigScene::ArmyConfigScene(int fromIndex, int targetIndex, Array<CityData>* allCities)
	: m_fromIndex(fromIndex)
	, m_targetIndex(targetIndex)
	, m_allCities(allCities)
{
	m_fromCity = &((*allCities)[fromIndex]);
	m_targetCity = &((*allCities)[targetIndex]);
}

void ArmyConfigScene::update()
{
	// -------- 武将選択 --------
	if (!m_fromCity->officers.isEmpty())
	{
		if (KeyRight.down())
			m_selectedOfficerIndex = (m_selectedOfficerIndex + 1) % m_fromCity->officers.size();

		if (KeyLeft.down())
			m_selectedOfficerIndex = (m_selectedOfficerIndex + m_fromCity->officers.size() - 1)
			% m_fromCity->officers.size();
	}

	// -------- 兵数調整 --------
	if (KeyUp.pressed())   m_soldierAllocation += 50;
	if (KeyDown.pressed()) m_soldierAllocation -= 50;

	m_soldierAllocation = Clamp(m_soldierAllocation, 100, m_fromCity->troops);

	// -------- 出陣決定 --------
	if (KeyEnter.down())
	{
		m_sceneEnd = true;
		m_nextScene = U"BattleMapScene";
	}
}

void ArmyConfigScene::draw() const
{
	Scene::SetBackground(ColorF(0.1, 0.1, 0.15));

	FontAsset(U"medium")(U"部隊編成").drawAt(400, 40);

	FontAsset(U"small")(U"出陣元: {} (兵:{})"_fmt(m_fromCity->name, m_fromCity->troops))
		.draw(20, 100);
	FontAsset(U"small")(U"目標: {}"_fmt(m_targetCity->name))
		.draw(20, 130);

	// 武将表示
	if (m_fromCity->officers.isEmpty())
	{
		FontAsset(U"small")(U"この都市には武将がいません").draw(20, 200, Palette::Red);
	}
	else
	{
		const Officer& off = m_fromCity->officers[m_selectedOfficerIndex];
		FontAsset(U"medium")(U"主将: {}"_fmt(off.name))
			.draw(20, 200, Palette::Yellow);
		FontAsset(U"small")(U"武力:{}  統率:{}"_fmt(off.war, off.leadership))
			.draw(20, 240);
	}

	// 兵数
	FontAsset(U"small")(U"出陣兵数: {}"_fmt(m_soldierAllocation)).draw(20, 300);

	RectF bar(20, 330, 300, 20);
	bar.draw(ColorF(0.2));
	RectF(20, 330, 300.0 * (double(m_soldierAllocation) / m_fromCity->troops), 20)
		.draw(ColorF(0.8, 0.2, 0.2));

	FontAsset(U"small")(U"↑↓: 兵数変更   ←→: 武将変更   Enter: 出陣")
		.draw(20, 380, Palette::Gray);
}

Officer ArmyConfigScene::getSelectedOfficer() const
{
	if (m_fromCity->officers.isEmpty())
	{
		Officer d;
		d.name = U"名無し";
		return d;
	}
	return m_fromCity->officers[m_selectedOfficerIndex];
}

int ArmyConfigScene::getSoldierAllocation() const
{
	return m_soldierAllocation;
}
