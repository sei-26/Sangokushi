# include <Siv3D.hpp>
# include "GameSceneManager.hpp"
# include "WorldMapScene.hpp"
# include "CityScene.hpp"
# include "BattleScene.hpp"

void Main()
{
	Window::SetFullscreen(true);
	FontAsset::Register(U"small", 14);

	GameSceneManager::setScene(std::make_unique<WorldMapScene>());

	while (System::Update())
	{
		// ESCで終了しないようにする
		if (KeyEscape.down()) continue;

		GameSceneManager::update();
		GameSceneManager::draw();
	}
}
