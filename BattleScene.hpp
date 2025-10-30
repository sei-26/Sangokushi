#pragma once
#include "SceneBase.hpp"
#include "GameSceneManager.hpp"
#include "GameManager.hpp"  // ← 戦闘システムを呼び出す
#include "CityData.hpp"

class BattleScene : public SceneBase
{
private:
	GameManager game;  // 🟢 実際の戦闘を管理するクラス
	bool battleEnded = false;

public:
	BattleScene()
	{
		Print << U"⚔️ 戦闘シーン開始 (GameManager使用)";
	}

	void update() override
	{
		// 🟢 GameManagerの更新（戦闘ロジック処理）
		game.Update();

		// 🟡 戦闘終了 or 戻る操作
		if (MouseR.down())
		{
			battleEnded = true;
			GameSceneManager::SetNextScene(U"World");
		}
	}

	void draw() const override
	{
		// 🟢 GameManagerが内部で描画を担当
		// （もし別UIを重ねたいならここに追加）
	}
};
