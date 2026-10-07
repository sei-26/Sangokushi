#include <Siv3D.hpp>
#include "GameSceneManager.hpp"
#include "CSVDataLoader.hpp"
#include "AudioManager.hpp"
#include "NewGame/CampaignScene.hpp"
#include <cstdlib>
#include <memory>
#include <cstring>

void Main()
{
	Window::SetTitle(U"群雄戦線 ～領土と兵站の三国志～");
	char* previewValue = nullptr; size_t previewLength = 0;
	_dupenv_s(&previewValue,&previewLength,"SANGOKUSHI_PREVIEW");
	const bool preview = previewValue != nullptr && previewLength > 1;
	const int previewMode = preview && std::strcmp(previewValue,"history") == 0 ? 2 : preview && std::strcmp(previewValue,"tactics") == 0 ? 3 : 1;
	std::free(previewValue);
	if (preview) Window::Resize(1280,800);
	else { Window::SetFullscreen(true); Scene::SetResizeMode(ResizeMode::Keep); Scene::Resize(1600,900); }

	FontAsset::Register(U"title", 36, Typeface::Bold);
	FontAsset::Register(U"menu", 24);
	FontAsset::Register(U"small", 18);
	FontAsset::Register(U"huge", 72, Typeface::Heavy);
	FontAsset::Register(U"campaignTitle",32,Typeface::Bold);
	FontAsset::Register(U"campaignBody",19,Typeface::CJK_Regular_JP);
	FontAsset::Register(U"campaignSmall",14,Typeface::CJK_Regular_JP);

	CampaignScene campaign;
	if (preview) campaign.preview(previewMode);
	std::unique_ptr<AudioManager> audio;
	std::unique_ptr<GameManager> gameManager;
	std::unique_ptr<GameSceneManager> sceneManager;
	int previewFrames = 0;

	while (System::Update())
	{
		if (campaign.legacyRequested() && !sceneManager)
		{
			audio = std::make_unique<AudioManager>(); gameManager = std::make_unique<GameManager>(); gameManager->pAudio = audio.get();
			auto cities = CSVDataLoader::LoadCities(U"cities.csv");
			if (FileSystem::Exists(U"officers.csv")) CSVDataLoader::LoadOfficers(cities,U"officers.csv");
			sceneManager = std::make_unique<GameSceneManager>(gameManager.get(),Faction{},cities,audio.get());
			Window::SetTitle(U"志在千里 ～三國志正伝～");
		}
		if (sceneManager) { sceneManager->update(); sceneManager->draw(); }
		else { if (!preview) campaign.update(); campaign.draw(); }
		if (preview && ++previewFrames == 12) { ScreenCapture::SetScreenshotDirectory(U"../Intermediate/"); ScreenCapture::SaveCurrentFrame(previewMode == 2 ? U"new-campaign-history.png" : previewMode == 3 ? U"new-campaign-tactics.png" : U"new-campaign-preview.png"); }
		if (preview && previewFrames >= 20) System::Exit();
	}
}
