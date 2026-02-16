#include <Siv3D.hpp>
#include "GameSceneManager.hpp"
#include "CSVDataLoader.hpp"
#include "LoyaltyManager.hpp"
#include "AudioManager.hpp"

void Main()
{
	Window::SetTitle(U"三国志風SLG - 官渡の戦い");
	Window::SetFullscreen(true);

	FontAsset::Register(U"title", 36, Typeface::Bold);
	FontAsset::Register(U"menu", 24);
	FontAsset::Register(U"small", 18);
	FontAsset::Register(U"huge", 56, Typeface::Bold);

	// ★ AudioManager追加
	AudioManager audio;

	// テスト用：起動時に効果音
	audio.PlaySE(AudioManager::SEType::Click);

	Array<CityData> cities = CSVDataLoader::LoadCities(U"cities.csv");

	if (FileSystem::Exists(U"officers.csv"))
	{
		CSVDataLoader::LoadOfficers(cities, U"officers.csv");
	}
	else
	{
		Print << U"[WARNING] officers.csv が見つかりません。";

		for (auto& city : cities)
		{
			Officer defaultOfficer;
			defaultOfficer.name = city.owner;
			defaultOfficer.leadership = 70;
			defaultOfficer.power = 70;
			defaultOfficer.war = 70;
			defaultOfficer.intelligence = 70;
			defaultOfficer.politics = 70;
			defaultOfficer.loyalty = 100;
			city.officers.push_back(defaultOfficer);
		}
	}

	Faction playerFaction;
	playerFaction.name = U"曹操";
	playerFaction.color = Color(100, 150, 255);

	GameManager gameManager;
	gameManager.year = 200;
	gameManager.month = 1;

	// ★ AudioManagerをGameManagerに接続（ポインタで）
	// 注：GameManager.hppに AudioManager* audio; を追加する必要あり
	// 今回は外部で管理

	GameSceneManager sceneManager(&gameManager, playerFaction, cities, &audio);

	// BGMは自動で開始（sceneManager内で）

	while (System::Update())
	{
		// マウスクリック時に効果音
		if (MouseL.down())
		{
			audio.PlaySE(AudioManager::SEType::Click);
		}

		sceneManager.update();
		sceneManager.draw();
	}
}
