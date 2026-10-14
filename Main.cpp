#include <Siv3D.hpp>
#include "NewGame/CampaignScene.hpp"
#include <cstdlib>
#include "NewGame/PreviewModes.hpp"

void Main()
{
	Window::SetTitle(U"群雄戦線 ～軍師の策で歴史を紡ぐ～");
	char* previewValue = nullptr; size_t previewLength = 0;
	_dupenv_s(&previewValue,&previewLength,"SANGOKUSHI_PREVIEW");
	const bool preview = previewValue != nullptr && previewLength > 1;
	const int previewMode = frontline::preview::FindMode(previewValue ? previewValue : "");
	std::free(previewValue);
    char* windowedValue = nullptr; size_t windowedLength = 0;
    _dupenv_s(&windowedValue, &windowedLength, "SANGOKUSHI_WINDOWED");
    const bool windowed = windowedValue && std::string_view(windowedValue) == "1";
    std::free(windowedValue);
    if (preview)
        Window::Resize(previewMode == 26 ? Size(1600, 900) :
            frontline::preview::IsCompact(previewMode) ? Size(1280, 720) : Size(1280, 800));
    else if (windowed)
    {
        Window::Resize(1280, 720);
        Scene::SetResizeMode(ResizeMode::Actual);
    }
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
