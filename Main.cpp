#include <Siv3D.hpp>
#include "GameSceneManager.hpp"

void Main()
{
	Window::Resize(1280, 720);
	Scene::SetBackground(ColorF{ 0.1, 0.1, 0.15 });
	FontAsset::Register(U"small", 24, Typeface::Medium);

	GameSceneManager game;

	while (System::Update())
	{
		game.update();
		game.draw();
	}
}
