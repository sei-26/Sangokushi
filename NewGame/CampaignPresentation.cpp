#include "CampaignPresentation.hpp"
#include <algorithm>
void CampaignPresentation::Reset()
{
	m_queue.clear();
	if (m_impact)
		m_impact.stopAllShots();
	m_elapsed = 0;
	for (const auto& sound : m_sounds)
		if (sound)
			sound.stopAllShots();
}
void CampaignPresentation::ToggleSound()
{
	m_sound = !m_sound;
	if (!m_sound && m_impact)
		m_impact.stopAllShots();
	if (!m_sound)
		for (const auto& sound : m_sounds)
			if (sound)
				sound.stopAllShots();
}
void CampaignPresentation::Start()
{
	m_elapsed = 0;
	if (!Busy() || !m_sound)
		return;
	if (!m_loaded)
	{
		for (int i = 0; i < 9; ++i)
			m_sounds[i] = Audio(Sound(static_cast<frontline::Tactic>(i)));
		m_impact = Audio(Sound(frontline::Tactic::SiegeStrike));
		m_loaded = true;
	}
	const auto tactic = m_queue.front().event.tactic;
	m_sounds[static_cast<int>(tactic)].playOneShot(.55);
}
void CampaignPresentation::Capture(const frontline::Campaign& game)
{
	const bool wasEmpty = !Busy();
	auto events = game.tacticEvents;
	std::stable_sort(events.begin(), events.end(), [&](const auto& a, const auto& b) {
		return (a.faction == game.player) > (b.faction == game.player);
	});
	for (const auto& e : events)
		if (e.general >= 0 && e.general < static_cast<int>(game.generals.size()))
			m_queue.push_back({game.generals[e.general], e});
	if (wasEmpty && Busy())
		Start();
}
void CampaignPresentation::Update(double dt, bool skip)
{
	if (!Busy())
		return;
	const double before = m_elapsed;
	m_elapsed += Max(0., dt);
	if (before < .8 && m_elapsed >= .8 && m_sound && m_loaded && !skip)
		m_impact.playOneShot(.3);
	if ((skip && m_elapsed > .2) || m_elapsed >= 1.65)
	{
		m_queue.pop_front();
		Start();
	}
}
