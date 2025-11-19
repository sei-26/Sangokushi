#pragma once
#include <Siv3D.hpp>
#include "SceneBase.hpp"

class GameClearScene : public SceneBase
{
public:
	GameClearScene() {}

	void update() override
	{
		// Enterでタイトル（または最初の画面）に戻る
		if (KeyEnter.down())
		{
			m_sceneEnd = true;
			m_nextScene = U"FactionSelectScene";  // タイトルへ戻る
		}
	}

	void draw() const override
	{
		Scene::SetBackground(ColorF(0.1, 0.1, 0.15));

		FontAsset(U"large")(U"天下統一！ゲームクリア！")
			.drawAt(Scene::Center(), Palette::Gold);

		FontAsset(U"small")(U"Enterキーで最初に戻る")
			.drawAt(Scene::Center().movedBy(0, 60), Palette::Gray);
	}
};
