#pragma once
#include <Siv3D.hpp>
#include "HeroStory.hpp"

// Visual and audio feedback is transient. Story rules and saves stay in HeroStory.
class StoryPresentation
{
	Texture m_land{U"StoryArt/changban.png"}, m_faces{U"StoryArt/heroes.png"};
	Audio m_hit, m_call, m_win, m_music;
	bool m_loaded = false, m_battle = false, m_sound = true;
	double m_time = 0, m_moment = 0;
	int m_hero = 0;
	String m_title, m_quote;
	struct Number
	{
		Vec2 tile;
		int amount;
		double age;
	};
	Array<Number> m_numbers;
	static Wave Tone(int kind);
	static Wave Score();

public:
	void LoadAudio();
	void Stop();
	bool Sound() const;
	void ToggleSound();
	void Tick(bool battle);
	bool Busy() const;
	void Moment(int who, const String& title, const String& quote);
	static String Quote(int who);
	void Capture(const hero::Story& before, const hero::Story& after);
	void Landscape(const RectF& r, double opacity = 1) const;
	void Portrait(int who, const RectF& r, double opacity = 1) const;
	void Atmosphere() const;
	void Board(const hero::Story& s, int selected, const RectF& r) const;
	void Overlay() const;
};
