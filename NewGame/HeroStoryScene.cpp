#include "HeroStoryScene.hpp"

Rect HeroStoryScene::BattleChoice(int i) const
{
	return Rect(Scene::Center().x - 430 + i * 440, Scene::Center().y + 105, 420, 82);
}

int HeroStoryScene::SpeakerHero(int stage) const
{
	for (int i = 0; i < 6; ++i)
		if (hero::Name(i) == hero::Book()[s.chapter].speaker[stage])
			return i;
	return -1;
}

String HeroStoryScene::T(const std::u32string& v)
{
	return String(v.c_str());
}

Rect HeroStoryScene::B(int i) const
{
	return Rect(Scene::Width() - 310, 150 + i * 58, 278, 46);
}

Rect HeroStoryScene::Choice(int i) const
{
	return Rect(285, Scene::Height() - 228 + i * 82, Scene::Width() - 625, 70);
}

RectF HeroStoryScene::Map() const
{
	double z = Min((Scene::Width() - 610.0) / hero::W, (Scene::Height() - 330.0) / hero::H);
	return RectF(285, 190, z * hero::W, z * hero::H);
}

void HeroStoryScene::Button(const Rect& r, const String& text, bool enabled)
{
	r.rounded(6)
	    .draw(enabled && r.mouseOver() ? ColorF(.29, .36, .30) : ColorF(.13, .19, .18))
	    .drawFrame(1, ColorF(.55, .49, .32));
	FontAsset(U"campaignBody")(text).drawAt(r.center(), enabled ? ColorF(.94, .87, .69) : ColorF(.40));
}

void HeroStoryScene::Save()
{
	message = hero::SaveJSON(s).save(U"hero-story-save.json")
	              ? U"英雄譚を保存しました。"
	              : U"保存できませんでした。保存先を確認してください。";
}

void HeroStoryScene::DrawBackdrop(const RectF& r, double alpha) const
{
	fx.Landscape(r, alpha);
}

void HeroStoryScene::Open()
{
	fx.Stop();
	fx.LoadAudio();
	landing = true;
	back = false;
	notes = false;
	message.clear();
}

bool HeroStoryScene::Back() const
{
	return back;
}
