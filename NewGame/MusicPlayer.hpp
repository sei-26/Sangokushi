#pragma once
#include <Siv3D.hpp>
#include "MusicCues.hpp"
class MusicPlayer
{
	std::array<Audio, 6> m_tracks;
	int m_current = -1, m_previous = -1, m_volume = 1;
	bool m_enabled = true;
	double m_blend = 0, m_duck = 1;
	void Choose(musiccues::Cue cue);

public:
	~MusicPlayer()
	{
		Stop();
	}
	void Stop();
	void Toggle();
	void Update(musiccues::Cue cue, double dt, bool duck = false);
	bool Input(Vec2 origin, int height = 28);
	void Draw(Vec2 origin, int height = 28) const;
	bool Enabled() const
	{
		return m_enabled;
	}
	static Rect ToggleRect(Vec2 p, int h)
	{
		return Rect(p.asPoint(), 124, h);
	}
	static Rect VolumeRect(Vec2 p, int h)
	{
		return Rect(p.movedBy(134, 0).asPoint(), 114, h);
	}
};
