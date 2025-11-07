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
		// ← ここ！ 毎フレーム左上のPrintログを全消去
		ClearPrint();

		// ESCで終了しない
		if (KeyEscape.down()) continue;

		GameSceneManager::update();
		GameSceneManager::draw();
	}
}
