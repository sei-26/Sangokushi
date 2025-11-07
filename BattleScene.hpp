#pragma once
#include <algorithm>
#include "SceneBase.hpp"
#include "GameSceneManager.hpp"
#include "GameManager.hpp"  // ← enum BattleResult をここから使う

class BattleScene : public SceneBase
{
private:
	GameManager game;

	BattleResult result = BattleResult::None; // ✅ OK: GameManager側のenumを使用
	bool isEnding = false;
	double endTimer = 0.0;
	Audio bgm;

	// 勝敗判定
	BattleResult calcResult() const
	{
		const auto& units = game.GetUnits();
		bool anyEnemyAlive = std::any_of(units.begin(), units.end(),
								[](const Unit& u) { return !u.isPlayer && u.alive; });
		bool anyPlayerAlive = std::any_of(units.begin(), units.end(),
								[](const Unit& u) { return  u.isPlayer && u.alive; });

		if (!anyEnemyAlive && anyPlayerAlive)  return BattleResult::Victory;
		if (!anyPlayerAlive && anyEnemyAlive)  return BattleResult::Defeat;
		if (!anyEnemyAlive && !anyPlayerAlive) return BattleResult::Defeat;
		return BattleResult::None;
	}

public:
	BattleScene()
	{
		ClearPrint();
		bgm = Audio{ U"example/bgm_battle.mp3", Loop::Yes };
		bgm.play();
	}

	void update() override
	{
		if (!isEnding)
		{
			game.Update();

			BattleResult r = calcResult();
			if (r != BattleResult::None)
			{
				result = r;
				isEnding = true;
				bgm.stop();
			}
		}
		else
		{
			endTimer += Scene::DeltaTime();
			if (endTimer >= 2.0)
			{
				GameSceneManager::SetNextScene(U"World");
			}
		}
	}

	void draw() const override
	{
		if (isEnding)
		{
			String msg;
			Color col = Palette::White;
			if (result == BattleResult::Victory)
			{
				msg = U"勝利！";
				col = Palette::Yellow;
			}
			else if (result == BattleResult::Defeat)
			{
				msg = U"敗北…";
				col = Palette::Gray;
			}

			RectF(Scene::CenterF().movedBy(-150, -60), 300, 120)
				.draw(ColorF(0, 0, 0, 0.6))
				.drawFrame(2, Palette::White);

			FontAsset(U"small")(msg).drawAt(Scene::Center(), col);
		}
	}
};
