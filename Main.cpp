# include <Siv3D.hpp>
# include "GameManager.hpp"

void Main()
{
	// ウィンドウ設定
	Window::SetTitle(U"三國志風 戦略シミュレーション");
	Window::SetFullscreen(true);

	// フォント登録
	FontAsset::Register(U"small", 14);

	// ゲームマネージャ生成
	GameManager game;

	// メインループ
	while (System::Update())
	{
		// 背景クリア（毎フレーム描画）
		Scene::SetBackground(ColorF(0.15, 0.3, 0.15));

		// 1フレーム分のゲーム更新＋描画
		game.Update();
	}
}
