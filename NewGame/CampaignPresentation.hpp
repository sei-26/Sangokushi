#pragma once
#include <Siv3D.hpp>
#include <deque>
#include <functional>
#include "Campaign.hpp"

// Presentation only: it consumes successful events and never modifies the campaign.
class CampaignPresentation
{
	struct Moment
	{
		frontline::General officer;
		frontline::TacticEvent event;
	};
	std::deque<Moment> m_queue;
	double m_elapsed = 0;
	bool m_sound = true, m_loaded = false;
	std::array<Audio, 9> m_sounds;
	Audio m_impact;
	void Start();
	static Wave Sound(frontline::Tactic tactic);
	static ColorF Color(frontline::Tactic tactic);
	static String Call(frontline::Tactic tactic);

public:
	bool Busy() const
	{
		return !m_queue.empty();
	}
	bool SoundEnabled() const
	{
		return m_sound;
	}
	void ToggleSound();
	void Reset();
	void Capture(const frontline::Campaign& game);
	void Update(double dt, bool skip = false);
	void DrawMap(const std::function<Vec2(int)>& center, double cell) const;
	void DrawOverlay(const Texture& faces) const;
};
