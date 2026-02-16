#include <Siv3D.hpp>
#include "GameSceneManager.hpp"
#include "CSVDataLoader.hpp"
#include "AudioManager.hpp"

void Main()
{
	Window::SetTitle(U"志在千里 ～三國志正伝～");
	Window::SetFullscreen(true);

	FontAsset::Register(U"title", 36, Typeface::Bold);
	FontAsset::Register(U"menu", 24);
	FontAsset::Register(U"small", 18);
	FontAsset::Register(U"huge", 72, Typeface::Heavy);

	AudioManager audio;
	GameManager gameManager;
	gameManager.pAudio = &audio;

	// データ読み込み
	Array<CityData> cities = CSVDataLoader::LoadCities(U"cities.csv");
	if (FileSystem::Exists(U"officers.csv")) {
		CSVDataLoader::LoadOfficers(cities, U"officers.csv");
	}

	// 初期勢力（後でFactionSelectで上書きされる）
	Faction playerFaction;

	// 管理クラス作成
	GameSceneManager sceneManager(&gameManager, playerFaction, cities, &audio);

	while (System::Update())
	{
		sceneManager.update();
		sceneManager.draw();
	}
}
