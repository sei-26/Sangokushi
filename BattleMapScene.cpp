#include "BattleMapScene.hpp"

//==========================================================
// コンストラクタ：本物の CityData を参照で受け取る
//==========================================================
BattleMapScene::BattleMapScene(
	int atkIndex_,
	int defIndex_,
	Array<CityData>* all,
	const Officer& leader,
	int soldiers)
{
	atkIndex = atkIndex_;
	defIndex = defIndex_;
	allCities = all;

	atkCityRef = &((*allCities)[atkIndex]);
	defCityRef = &((*allCities)[defIndex]);

	m_leader = leader;
	m_soldiers = soldiers;

	manager.InitializeBattle(*atkCityRef, *defCityRef, leader, soldiers);
}


//==========================================================
// 更新処理
//==========================================================
void BattleMapScene::update()
{
	if (!manager.IsBattleFinished())
	{
		manager.Update();
		return;
	}

	// ★戦闘が終わったら確実に本物の CityData に結果を書き込む
	manager.ApplyBattleResult(*atkCityRef, *defCityRef);

	m_sceneEnd = true;
	m_nextScene = U"WorldMapScene";
}


//==========================================================
// 描画処理
//==========================================================
void BattleMapScene::draw() const
{
	Scene::SetBackground(ColorF(0.1, 0.1, 0.12));

	// ★戦闘終了後は描画しない（安全）
	if (!manager.IsBattleFinished())
	{
		manager.Draw();

		FontAsset(U"small")(U"[左クリック] 移動 ／ 隣接で攻撃")
			.draw(20, Scene::Height() - 35, Palette::Gray);
	}
}
