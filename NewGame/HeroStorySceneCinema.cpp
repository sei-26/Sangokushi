#include "HeroStoryScene.hpp"
#include "StoryDrama.hpp"
void HeroStoryScene::syncNarrative()
{
	m_presentation.Chapter(m_story.chapter);
	if (m_seenChapter != m_story.chapter || m_seenPhase != m_story.phase)
	{
		const bool chapterChanged = m_seenChapter != m_story.chapter;
		m_seenChapter = m_story.chapter;
		m_seenPhase = m_story.phase;
		m_dialogueBeat = 0;
		m_dialogueAge = 0;
		if (chapterChanged && m_story.phase == 0)
			m_chapterIntro = 3.6;
	}
	m_dialogueAge += Scene::DeltaTime();
}
bool HeroStoryScene::advanceDialogue()
{
	if (m_story.phase != 0 && m_story.phase != 1 && m_story.phase != 3)
		return false;
	const int stage = m_story.phase == 3 ? 2 : m_story.phase;
	const String line = m_dialogueBeat < 2
	                        ? String(hero::drama::Conversation(m_story.chapter, stage, m_dialogueBeat).line)
	                        : String(hero::Book()[m_story.chapter].dialogue[stage]);
	if (m_dialogueAge * 36 < line.size())
	{
		m_dialogueAge = line.size() / 36. + 1;
		return true;
	}
	if (m_dialogueBeat < 2)
	{
		++m_dialogueBeat;
		m_dialogueAge = 0;
		return true;
	}
	return false;
}
void HeroStoryScene::drawChapterIntro() const
{
	if (m_chapterIntro <= 0)
		return;
	const auto& c = hero::Book()[m_story.chapter];
	const double elapsed = 3.6 - m_chapterIntro, enter = Clamp(elapsed / .4, 0., 1.),
	             alpha = Min(enter, Clamp(m_chapterIntro / .4, 0., 1.));
	m_presentation.Landscape(Scene::Rect(), 1);
	Scene::Rect().draw(ColorF(.01, .025, .027, .36));
	m_presentation.Atmosphere();
	Rect(0, 0, Scene::Width(), 64).draw(ColorF(.005, .01, .012));
	Rect(0, Scene::Height() - 64, Scene::Width(), 64).draw(ColorF(.005, .01, .012));
	for (int i = 0; i < m_story.Unlocked(); ++i)
	{
		const RectF card(80 + i * 82, Scene::Height() - 165, 68, 80);
		m_presentation.Portrait(i, card, alpha);
		FontAsset(U"campaignSmall")(text(hero::Name(i)))
		    .drawAt(card.center().x, Scene::Height() - 74, ColorF(.97, .87, .66, alpha));
	}
	const double x = 80 + (1 - enter) * 70, y = Scene::Height() * .32;
	FontAsset(U"campaignBody")(U"{} / {}"_fmt(String(c.year), String(c.place)))
	    .draw(x, y - 48, ColorF(.95, .83, .56, alpha));
	FontAsset(U"storyDisplay")(U"第{}章"_fmt(m_story.chapter + 1))
	    .draw(32, x, y, ColorF(.97, .80, .39, alpha));
	FontAsset(U"storyDisplay")(String(c.title)).draw(64, x, y + 54, ColorF(1., .94, .79, alpha));
	Line(x, y + 144, Scene::Width() - 100, y + 144).draw(2, ColorF(.90, .69, .30, alpha));
	FontAsset(U"storyDialogue")(String(hero::drama::Theme(m_story.chapter)))
	    .draw(x, y + 170, ColorF(.95, .94, .84, alpha));
	FontAsset(U"campaignSmall")(U"クリック・Space・Enterで物語へ")
	    .drawAt(Scene::Center().x, Scene::Height() - 32, ColorF(.81, .84, .76));
}
