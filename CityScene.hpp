#pragma once
#include "SceneBase.hpp"
#include "GameSceneManager.hpp"
#include "CityData.hpp"

class CityScene : public SceneBase
{
private:
	CityData city;
	Rect launchButton{ 70, 220, 180, 40 }; // 出撃ボタン

public:
	CityScene(const CityData& data)
		: city(data) {
	}

	void update() override
	{
		// 🟢 出撃ボタンがクリックされたら戦闘シーンへ
		if (launchButton.leftClicked())
		{
			Print << U"⚔️ 出撃！ 城: " << city.name;
			GameSceneManager::SetNextScene(U"Battle:" + city.name);
		}

		// 🟡 右クリックでワールドマップへ戻る
		if (MouseR.down())
		{
			GameSceneManager::SetNextScene(U"World");
		}

		// 💬 内政コマンド（仮）
		if (KeyF.down()) city.food += 50;
		if (KeyT.down()) city.troops += 10;
		if (KeyG.down()) city.gold += 100;
	}

	void draw() const override
	{
		Scene::SetBackground(ColorF(0.1, 0.15, 0.1));
		Rect(50, 50, 300, 250).draw(ColorF(0, 0, 0, 0.6));

		// 城情報表示
		FontAsset(U"small")(Format(U"🏰 城名: {}", city.name)).draw(70, 70, Palette::Yellow);
		FontAsset(U"small")(Format(U"城主: {}", city.ruler)).draw(70, 100, Palette::White);
		FontAsset(U"small")(Format(U"金: {}  食糧: {}  兵: {}", city.gold, city.food, city.troops)).draw(70, 130, Palette::White);

		// 🟢 出撃ボタン
		launchButton.draw(ColorF(0.3, 0.2, 0.2));
		launchButton.drawFrame(2, Palette::Yellow);
		FontAsset(U"small")(U"出撃！").drawAt(launchButton.center(), Palette::White);

		// 戻る案内
		FontAsset(U"small")(U"[右クリック] ワールドマップに戻る").draw(70, 280, Palette::Skyblue);
	}
};
