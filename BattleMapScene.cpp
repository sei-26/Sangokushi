#include "BattleMapScene.hpp"

//==========================================================
// コンストラクタ：戦場を初期化
//==========================================================
BattleMapScene::BattleMapScene(
	const CityData& fromCity,
	const CityData& targetCity,
	const Officer& selectedLeader,
	int selectedSoldiers)
{
	atkCityRef = const_cast<CityData*>(&fromCity);
	defCityRef = const_cast<CityData*>(&targetCity);

	manager.InitializeBattle(fromCity, targetCity, selectedLeader, selectedSoldiers);
}


//==========================================================
// 更新処理
//==========================================================
void BattleMapScene::update()
{
	manager.Update();

	if (manager.IsBattleFinished())
	{
		manager.ApplyBattleResult(*atkCityRef, *defCityRef);

		// ★占領処理はここで完了

		m_sceneEnd = true;
		m_nextScene = U"WorldMapScene";
	}
}



//==========================================================
// 描画処理
//==========================================================
void BattleMapScene::draw() const
{
	Scene::SetBackground(ColorF(0.1, 0.1, 0.12));

	manager.Draw();

	// 操作説明
	FontAsset(U"small")(U"[左クリック] 移動 ／ 隣接で攻撃")
		.draw(20, Scene::Height() - 35, Palette::Gray);
}
