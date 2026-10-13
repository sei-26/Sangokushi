#include "MusicPlayer.hpp"
#include "CampaignUI.hpp"
void MusicPlayer::Stop()
{
	if (m_current < 0 && m_previous < 0)
		return;
	for (auto& t : m_tracks)
		if (t)
			t.stop();
	m_current = m_previous = -1;
	m_blend = 0;
}
void MusicPlayer::Toggle()
{
	m_enabled = !m_enabled;
	if (!m_enabled)
	{
		for (auto& t : m_tracks)
			if (t)
				t.stop();
	}
	else if (m_current >= 0 && m_tracks[m_current])
	{
		m_previous = -1;
		m_blend = 0;
		m_tracks[m_current].setVolume(0).play();
	}
}
void MusicPlayer::Choose(musiccues::Cue cue)
{
	const int next = static_cast<int>(cue);
	if (next == m_current)
		return;
	if (m_previous >= 0 && m_tracks[m_previous])
		m_tracks[m_previous].stop();
	m_previous = m_current;
	m_current = next;
	m_blend = 0;
	if (!m_tracks[next] && FileSystem::Exists(musiccues::File(cue)))
		m_tracks[next] = Audio(Audio::Stream, musiccues::File(cue), Loop::Yes);
	if (m_enabled && m_tracks[next])
		m_tracks[next].setVolume(0).play();
}
void MusicPlayer::Update(musiccues::Cue cue, double dt, bool duck)
{
	Choose(cue);
	if (!m_enabled)
		return;
	const double step = Max(0., dt);
	m_blend = Min(1., m_blend + step / 1.4);
	m_duck += ((duck ? .58 : 1.) - m_duck) * Min(1., step * 4);
	const double level = (m_volume + 1) * .2 * m_duck;
	if (m_current >= 0 && m_tracks[m_current])
		m_tracks[m_current].setVolume(level * std::sqrt(m_blend));
	if (m_previous >= 0 && m_tracks[m_previous])
	{
		m_tracks[m_previous].setVolume(level * std::sqrt(1 - m_blend));
		if (m_blend >= 1)
		{
			m_tracks[m_previous].stop();
			m_previous = -1;
		}
	}
}
bool MusicPlayer::Input(Vec2 p, int h)
{
	bool acted = false;
	if (KeyB.down() || ToggleRect(p, h).leftClicked())
	{
		Toggle();
		acted = true;
	}
	if (KeyN.down() || VolumeRect(p, h).leftClicked())
	{
		m_volume = (m_volume + 1) % 4;
		acted = true;
	}
	return acted;
}
void MusicPlayer::Draw(Vec2 p, int h) const
{
	const auto button = [](const Rect& r, const String& label) { campaignui::DrawButton(r, label); };
	button(ToggleRect(p, h), m_enabled ? U"BGM ON / B" : U"BGM OFF / B");
	button(VolumeRect(p, h), U"音量 {}% / N"_fmt((m_volume + 1) * 20));
}
