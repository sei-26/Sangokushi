#include <Siv3D.hpp>
#include "GameSceneManager.hpp"

void Main()
{
	Window::Resize(1920, 1080);
	Scene::SetBackground(ColorF(0.1, 0.12, 0.16));

	// ★ フォント登録（ここで行うのが正解）
	// コード内で使っているフォントアセット名を登録しておきます
	FontAsset::Register(U"title", 40);
	FontAsset::Register(U"menu", 24);
	FontAsset::Register(U"small", 26);
	FontAsset::Register(U"Default", 20); // WorldMapで使っているので追加推奨
	FontAsset::Register(U"huge", 80);    // クリア画面用
	FontAsset::Register(U"medium", 40);  // クリア画面用

	// ★ ここでマネージャーを作成
	// コンストラクタの中で自動的に WorldMapScene が作られるようにします
	GameSceneManager manager;

	while (System::Update())
	{
		manager.update();
		manager.draw();
	}
}
