#pragma once
#include "StorySave.hpp"
#include "StoryPresentation.hpp"
class HeroStoryScene
{
	hero::Story m_story;
	StoryPresentation m_presentation;
	bool m_landing = true, m_backRequested = false, m_showNotes = false;
	int m_selectedUnit = 0;
	int m_notePage = 0;
	Rect battleChoiceRect(int i) const;
	int speakerHero(int stage) const;
	String m_message;
	static String text(const std::u32string& v);
	Rect buttonRect(int i) const;
	Rect choiceRect(int i) const;
	RectF mapRect() const;
	static void drawButton(const Rect& r, const String& text, bool enabled = true);
	void save();
	void drawLanding() const;
	void drawNotes() const;
	void drawDialogue() const;
	void drawEnding() const;
	void drawFailure() const;
	void drawCivilActions() const;
	void drawBattle() const;
	void drawBattleEvent() const;

public:
	void DrawBackdrop(const RectF& r, double alpha) const;
	void Open();
	bool Back() const;
	void Preview(int mode);
	void update();
	void draw() const;
};
