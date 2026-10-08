#include "HeroStoryScene.hpp"

Rect HeroStoryScene::battleChoiceRect(int i) const
{
	return Rect(Scene::Center().x - 430 + i * 440, Scene::Center().y + 105, 420, 82);
}

int HeroStoryScene::speakerHero(int stage) const
{
	for (int i = 0; i < 6; ++i)
		if (hero::Name(i) == hero::Book()[m_story.chapter].speaker[stage])
			return i;
	return -1;
}

String HeroStoryScene::text(const std::u32string& v)
{
	return String(v.c_str());
}

Rect HeroStoryScene::buttonRect(int i) const
{
	if (i == 9)
		return Rect(Scene::Width() - 310, 324, 278, 46);
	return Rect(Scene::Width() - 310, 150 + i * 58, 278, 46);
}

Rect HeroStoryScene::choiceRect(int i) const
{
	return Rect(285, Scene::Height() - 228 + i * 82, Scene::Width() - 625, 70);
}

RectF HeroStoryScene::mapRect() const
{
	double z = Min((Scene::Width() - 610.0) / hero::W, (Scene::Height() - 330.0) / hero::H);
	return RectF(285, 190, z * hero::W, z * hero::H);
}

void HeroStoryScene::drawButton(const Rect& r, const String& text, bool enabled)
{
	r.rounded(6)
	    .draw(enabled && r.mouseOver() ? ColorF(.29, .36, .30) : ColorF(.13, .19, .18))
	    .drawFrame(1, ColorF(.55, .49, .32));
	FontAsset(U"campaignBody")(text).drawAt(r.center(), enabled ? ColorF(.94, .87, .69) : ColorF(.40));
}

void HeroStoryScene::save()
{
	m_message = hero::SaveJSON(m_story).save(U"hero-story-save.json")
	                ? U"英雄譚を保存しました。"
	                : U"保存できませんでした。保存先を確認してください。";
}

void HeroStoryScene::DrawBackdrop(const RectF& r, double alpha) const
{
	m_presentation.Landscape(r, alpha);
}

void HeroStoryScene::Open()
{
	m_presentation.Stop();
	m_presentation.LoadAudio();
	m_landing = true;
	m_backRequested = false;
	m_showNotes = false;
	m_message.clear();
}

bool HeroStoryScene::Back() const
{
	return m_backRequested;
}
