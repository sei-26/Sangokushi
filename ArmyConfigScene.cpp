#include "ArmyConfigScene.hpp"

ArmyConfigScene::ArmyConfigScene(const CityData& city)
	: m_city(city)
{
	m_units.resize(m_unitCount);

	// 初期値
	for (int i = 0; i < m_units.size(); i++)
	{
		m_units[i].name = U"部隊" + Format(i + 1);
		m_units[i].soldiers = 100;
		m_units[i].atk = 10;
		m_units[i].isPlayer = true;
	}
}

void ArmyConfigScene::update()
{
	// ユニット数変更
	if (KeyUp.down())   m_unitCount = Min(m_unitCount + 1, 5);
	if (KeyDown.down()) m_unitCount = Max(m_unitCount - 1, 1);

	m_units.resize(m_unitCount);

	// 各ユニット設定
	for (int i = 0; i < m_units.size(); i++)
	{
		auto& u = m_units[i];

		// 兵数調整
		if (Key1.pressed()) u.soldiers = Max(u.soldiers - 1, 10);
		if (Key2.pressed()) u.soldiers = Min(u.soldiers + 1, 200);

		// 攻撃力（仮）
		if (Key3.pressed()) u.atk = Max(u.atk - 1, 5);
		if (Key4.pressed()) u.atk = Min(u.atk + 1, 50);
	}

	// Enter → 侵攻先選択へ
	if (KeyEnter.down())
	{
		// BattleScene に渡すために m_units を保存したり
		// 次のシーンに渡す処理を書く
		m_isEnd = true;
		m_nextScene = U"TargetSelectScene";
		return;
	}
}

void ArmyConfigScene::draw() const
{
	FontAsset(U"title")(U"出陣部隊編成").drawAt(Scene::Center().x, 40);

	int y = 120;

	for (int i = 0; i < m_units.size(); i++)
	{
		const auto& u = m_units[i];

		FontAsset(U"menu")(U"ユニット " + Format(i + 1)).draw(50, y);
		FontAsset(U"menu")(U"兵数: " + Format(u.soldiers)).draw(50, y + 30);
		FontAsset(U"menu")(U"攻撃力: " + Format(u.atk)).draw(50, y + 60);

		y += 120;
	}

	FontAsset(U"menu")(U"↑↓ ユニット数調整").draw(50, 500);
	FontAsset(U"menu")(U"1/2 兵数  3/4 攻撃力").draw(50, 540);
	FontAsset(U"menu")(U"Enter: 次へ").drawAt(Scene::Center().x, 600);
}
