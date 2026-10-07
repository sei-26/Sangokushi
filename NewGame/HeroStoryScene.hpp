#pragma once
#include "StorySave.hpp"
#include "StoryPresentation.hpp"
class HeroStoryScene
{
	hero::Story s;
	StoryPresentation fx;
	bool landing = true, back = false, notes = false;
	int selected = 0;
	int notePage = 0;
	Rect BattleChoice(int i) const;
	int SpeakerHero(int stage) const;
	String message;
	static String T(const std::u32string& v);
	Rect B(int i) const;
	Rect Choice(int i) const;
	RectF Map() const;
	static void Button(const Rect& r, const String& text, bool enabled = true);
	void Save();
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
