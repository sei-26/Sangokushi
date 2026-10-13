#pragma once
#include <Siv3D.hpp>
#include "HeroStory.hpp"

// Visual and audio feedback is transient. Story rules and saves stay in HeroStory.
class StoryPresentation
{
	Texture m_land{U"StoryArt/changban.png"}, m_faces{U"StoryArt/heroes.png"};
	Audio m_hit, m_call, m_win;
	bool m_loaded = false, m_battle = false, m_sound = true;
	double m_time = 0, m_moment = 0;
	int m_hero = 0;
	String m_title, m_quote;
	struct Number
	{
		Vec2 tile;
		int amount;
		double age;
		Vec2 source{};
		bool hasSource = false, ranged = false, defeated = false;
	};
	Array<Number> m_numbers;
	struct Motion
	{
		int index;
		Vec2 from, to;
		double age = 0;
	};
	Array<Motion> m_motion;
	void TerrainTile(const hero::Story& s, int tile, const RectF& cell) const;
	void UnitFigure(const hero::Story& s, int index, int selected, const RectF& board) const;
	void ActionTrails(const RectF& board) const;
	static Wave Tone(int kind);
	bool m_celebration = false;
	void BattleSpectacle(const hero::Story& s, const RectF& r) const;

public:
	static constexpr double MomentSeconds = 2.7;
	void LoadAudio();
	void Stop();
	bool Sound() const;
	void ToggleSound();
	void Tick(bool battle, double dt = -1);
	int m_chapter = 3;
	Texture m_chapterArt{U"StoryArt/chapter-scenes-v1.png", TextureDesc::Mipped};
	void Chapter(int chapter)
	{
		m_chapter = Clamp(chapter, 0, 5);
	}
	void Skip()
	{
		m_moment = 0;
		m_motion.clear();
		for (auto& n : m_numbers)
			n.age = Max(.7, n.age);
	}
	bool Busy() const;
	void Moment(int who, const String& title, const String& quote);
	static String Quote(int who);
	void Capture(const hero::Story& before, const hero::Story& after, int actor = -1);
	void Landscape(const RectF& r, double opacity = 1) const;
	void PortraitName(const String& name, const RectF& r, double opacity = 1) const;
	void Portrait(int who, const RectF& r, double opacity = 1) const;
	void Atmosphere() const;
	void Board(const hero::Story& s, int selected, const RectF& r) const;
	void Overlay() const;
};
