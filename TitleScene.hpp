#pragma once
#include <Siv3D.hpp>
#include "SceneBase.hpp"
#include "GameManager.hpp"

class TitleScene : public SceneBase
{
public:
	TitleScene(GameManager* gm) : m_gameManager(gm)
	{
		if (m_gameManager->pAudio) {
			m_gameManager->pAudio->PlayBGM(AudioManager::BGMType::Title);
		}
	}

	void update() override
	{
		m_timer += Scene::DeltaTime();

		// 画面クリックかエンターキーで次へ
		if (MouseL.down() || KeyEnter.down())
		{
			if (m_gameManager->pAudio) m_gameManager->pAudio->PlaySE(AudioManager::SEType::Click);
			m_sceneEnd = true;
			m_nextScene = U"FactionSelect"; // 勢力選択へ
		}
	}

	void draw() const override
	{
		const auto& hugeFont = FontAsset(U"huge");
		const auto& menuFont = FontAsset(U"menu");

		// 背景：深みのある赤と黒のグラデーション
		Scene::Rect().draw(Arg::top = ColorF(0.2, 0.0, 0.0), Arg::bottom = Palette::Black);

		// 装飾用のパーティクル（火の粉のような演出）
		for (int i = 0; i < 20; ++i)
		{
			double t = m_timer * 0.2 + i * 0.5;
			double x = Periodic::Sine0_1(4.0, t) * Scene::Width();
			double y = Scene::Height() - Math::Fmod(t * 100.0, Scene::Height());
			Circle(x, y, 2).draw(ColorF(1.0, 0.5, 0.0, 0.3));
		}

		// メインタイトル「志在千里」
		const String title = U"志 在 千 里";
		double alpha = Min(1.0, m_timer * 0.5); // フェードイン

		// タイトルの影
		hugeFont(title).drawAt(Scene::Center().movedBy(4, 4), ColorF(0, 0, 0, alpha));
		// タイトル本体（金色）
		hugeFont(title).drawAt(Scene::Center(), ColorF(0.9, 0.7, 0.2, alpha));

		// サブタイトル
		FontAsset(U"title")(U"～三國志正伝～").drawAt(Scene::Center().movedBy(0, 80), ColorF(1.0, alpha));

		// 「PUSH START」の点滅
		if (alpha >= 1.0 && (int)(m_timer * 1.5) % 2 == 0)
		{
			menuFont(U"- Click to Start -").drawAt(Scene::Center().movedBy(0, 200), Palette::White);
		}

		// 著作権表記風
		FontAsset(U"small")(U"© 2026 Vantan Game Academy Project").drawAt(Scene::Width() / 2, Scene::Height() - 50, Palette::Gray);
	}

private:
	GameManager* m_gameManager;
	double m_timer = 0.0;
};
