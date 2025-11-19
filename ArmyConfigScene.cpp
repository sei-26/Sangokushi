#include "ArmyConfigScene.hpp"

ArmyConfigScene::ArmyConfigScene(const CityData& from, const CityData& target)
	: m_fromCity(from)
	, m_targetCity(target)
{
}

void ArmyConfigScene::update()
{
	// ★ 武将選択（左右キー）
	if (!m_fromCity.officers.isEmpty())
	{
		if (KeyRight.down())
		{
			m_selectedOfficerIndex =
				(m_selectedOfficerIndex + 1) % m_fromCity.officers.size();
		}
		if (KeyLeft.down())
		{
			m_selectedOfficerIndex =
				(m_selectedOfficerIndex + m_fromCity.officers.size() - 1)
				% m_fromCity.officers.size();
		}
	}

	// ★ 兵数スライダー（上下キー）
	if (KeyUp.pressed())   m_soldierAllocation += 50;
	if (KeyDown.pressed()) m_soldierAllocation -= 50;

	m_soldierAllocation = Clamp(m_soldierAllocation, 100, m_fromCity.troops);

	// ★ Enterで戦闘へ
	if (KeyEnter.down())
	{
		m_sceneEnd = true;
		m_nextScene = U"BattleMapScene";  // ← これに変更
	}

}

void ArmyConfigScene::draw() const
{
	Scene::SetBackground(ColorF(0.1, 0.1, 0.15));

	FontAsset(U"medium")(U"部隊編成").drawAt(400, 40);

	// 都市情報
	FontAsset(U"small")(U"出陣元: {}  (兵:{} )"_fmt(m_fromCity.name, m_fromCity.troops))
		.draw(20, 100, Palette::White);
	FontAsset(U"small")(U"目標: {}"_fmt(m_targetCity.name))
		.draw(20, 130, Palette::White);

	// 武将選択
	if (m_fromCity.officers.isEmpty())
	{
		FontAsset(U"small")(U"この都市には武将がいません").draw(20, 200, Palette::Red);
	}
	else
	{
		const Officer& off = m_fromCity.officers[m_selectedOfficerIndex];
		FontAsset(U"medium")(U"主将: {}"_fmt(off.name))
			.draw(20, 200, Palette::Yellow);

		FontAsset(U"small")(U"武力:{}  統率:{}"_fmt(off.war, off.leadership))
			.draw(20, 240, Palette::White);
	}

	// 兵数スライダー
	FontAsset(U"small")(U"出陣兵数: {}"_fmt(m_soldierAllocation))
		.draw(20, 300, Palette::White);

	// バー描画
	RectF bar(20, 330, 300, 20);
	bar.draw(ColorF(0.2));
	RectF(20, 330, 300.0 * (double(m_soldierAllocation) / m_fromCity.troops), 20)
		.draw(ColorF(0.8, 0.2, 0.2));

	FontAsset(U"small")(U"↑↓: 兵数変更   ←→: 武将変更   Enter: 出陣")
		.draw(20, 380, Palette::Gray);
}

Officer ArmyConfigScene::getSelectedOfficer() const
{
	if (m_fromCity.officers.isEmpty())
	{
		Officer dummy;
		dummy.name = U"名無し";
		return dummy;
	}
	return m_fromCity.officers[m_selectedOfficerIndex];
}


int ArmyConfigScene::getSoldierAllocation() const
{
	return m_soldierAllocation;
}
