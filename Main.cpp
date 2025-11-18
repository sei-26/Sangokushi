#include <Siv3D.hpp>
#include "GameSceneManager.hpp"

void Main()
{
	Window::Resize(1280, 720);
	Scene::SetBackground(ColorF{ 0.2, 0.2, 0.2 });

	// ★フォント登録を先頭に移動（絶対最初に必要）
	FontAsset::Register(U"small", 22);
	FontAsset::Register(U"title", 36);

	// ★ここからゲーム開始
	GameSceneManager manager;

	while (System::Update())
	{
		manager.update();
		manager.draw();
	}
}
