#include <Siv3D.hpp>
#include "GameSceneManager.hpp"

void Main()
{
	Window::Resize(1920, 1080);
	Scene::SetBackground(ColorF(0.1, 0.12, 0.16));

	// ★ フォント登録（ここで行うのが正解）
	// コード内で使っているフォントアセット名を登録しておきます
	FontAsset::Register(U"Default", 20);
	FontAsset::Register(U"title", 40);   // ← これが必要です！
	FontAsset::Register(U"menu", 20);
	FontAsset::Register(U"huge", 80);
	FontAsset::Register(U"small", 14);
	FontAsset::Register(U"medium", 30);

	// ★ ここでマネージャーを作成
	// コンストラクタの中で自動的に WorldMapScene が作られるようにします
	GameSceneManager manager;

	while (System::Update())
	{
		manager.update();
		manager.draw();
	}
}
