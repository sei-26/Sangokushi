#include <Siv3D.hpp>
#include "NewGame/CampaignScene.hpp"
#include <cstdlib>
#include <cstring>

void Main()
{
	Window::SetTitle(U"群雄戦線 ～領土と兵站の三国志～");
	char* previewValue = nullptr; size_t previewLength = 0;
	_dupenv_s(&previewValue,&previewLength,"SANGOKUSHI_PREVIEW");
	const bool preview = previewValue != nullptr && previewLength > 1;
	const int previewMode = std::strcmp(previewValue ? previewValue : "", "world-wide")==0 ? 26 : std::strcmp(previewValue ? previewValue : "", "regions")==0 ? 24 : std::strcmp(previewValue ? previewValue : "", "battlefield")==0 ? 23 : std::strcmp(previewValue ? previewValue : "", "ai-assembly")==0 ? 22 : std::strcmp(previewValue ? previewValue : "", "story-depth")==0 ? 19 : std::strcmp(previewValue ? previewValue : "", "battle-depth")==0 ? 20 : std::strcmp(previewValue ? previewValue : "", "story-vow")==0 ? 21 : std::strcmp(previewValue ? previewValue : "", "convoy")==0 ? 17 : std::strcmp(previewValue ? previewValue : "", "convoy-plan")==0 ? 18 : std::strcmp(previewValue ? previewValue : "", "assignments")==0 ? 16 : !preview ? 1 : std::strcmp(previewValue,"world")==0 ? 14 : std::strcmp(previewValue,"world-front")==0 ? 15 : std::strcmp(previewValue,"story-prepare")==0 ? 12 : std::strcmp(previewValue,"story-links")==0 ? 13 : std::strcmp(previewValue,"start")==0 ? 11 : std::strcmp(previewValue,"story-event")==0 ? 9 : std::strcmp(previewValue,"story-cutin")==0 ? 10 : std::strcmp(previewValue,"story-menu")==0 ? 4 : std::strcmp(previewValue,"story")==0 ? 5 : std::strcmp(previewValue,"story-battle")==0 ? 6 : std::strcmp(previewValue,"story-civil")==0 ? 7 : std::strcmp(previewValue,"story-ending")==0 ? 8 : std::strcmp(previewValue,"history")==0 ? 2 : std::strcmp(previewValue,"tactics")==0 ? 3 : 1;
	std::free(previewValue);
	if (preview) Window::Resize(previewMode==26 ? Size(1600,900) : Size(1280,800));
	else { Window::SetFullscreen(true); Scene::SetResizeMode(ResizeMode::Actual); }

	FontAsset::Register(U"campaignTitle",32,Typeface::Bold);
	FontAsset::Register(U"campaignBody",19,Typeface::CJK_Regular_JP);
	FontAsset::Register(U"campaignSmall",16,Typeface::CJK_Regular_JP);
	FontAsset::Register(U"storyDisplay",44,Typeface::CJK_Regular_JP);
	FontAsset::Register(U"storyDialogue",22,Typeface::CJK_Regular_JP);
	FontAsset::Register(U"storyAccent",28,Typeface::CJK_Regular_JP);

	CampaignScene campaign;
	if (preview) campaign.preview(previewMode);
	int previewFrames = 0;

	while (System::Update())
	{
		if (!preview) campaign.update();
		campaign.draw();
		if (preview && ++previewFrames == 12) { ScreenCapture::SetScreenshotDirectory(U"../Intermediate/"); ScreenCapture::SaveCurrentFrame(U"preview-{}.png"_fmt(previewMode)); }
		if (preview && previewFrames >= 20) System::Exit();
	}
}
