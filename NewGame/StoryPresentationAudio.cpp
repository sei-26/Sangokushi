#include "StoryPresentation.hpp"

Wave StoryPresentation::Tone(int kind)
{
	const int rate = 22050, count = kind == 2 ? rate : rate / 2;
	Wave w(static_cast<size_t>(count), Arg::sampleRate = rate);
	uint32 seed = 137;
	for (int i = 0; i < count; ++i)
	{
		double t = static_cast<double>(i) / rate;
		seed = seed * 1664525u + 1013904223u;
		double noise = (static_cast<double>((seed >> 8) & 65535) / 32768 - 1);
		double v = kind == 0   ? (.45 * noise * std::exp(-25 * t) +
		                          .5 * std::sin(6.283185 * (92 * t - 38 * t * t)) * std::exp(-12 * t))
		           : kind == 1 ? (.4 * std::sin(6.283185 * 110 * t) + .18 * std::sin(6.283185 * 165 * t)) *
		                             std::exp(-6 * t)
		                       : (.23 * std::sin(6.283185 * 220 * t) + .18 * std::sin(6.283185 * 330 * t) +
		                          .12 * std::sin(6.283185 * 440 * t)) *
		                             std::sin(Min(t / .02, 1.0) * 1.5708) * std::exp(-3 * t);
		if (kind == 0)
			v += .28 * std::sin(Math::TwoPi * 55 * t) * std::exp(-9 * t) +
			     .12 * std::sin(Math::TwoPi * 870 * t) * std::exp(-18 * t);
		if (kind == 1)
			v += .22 * std::sin(Math::TwoPi * (160 * t + 380 * t * t)) * std::exp(-4 * t) +
			     .10 * noise * std::exp(-12 * t);
		w[i] = static_cast<float>(Clamp(v * .65, -.92, .92));
	}
	return w;
}

void StoryPresentation::LoadAudio()
{
	if (m_loaded)
		return;
	m_loaded = true;
	m_hit = Audio(Tone(0));
	m_call = Audio(Tone(1));
	m_win = Audio(Tone(2));
}

void StoryPresentation::Stop()
{
	m_hit.stopAllShots();
	m_call.stopAllShots();
	m_win.stopAllShots();
	m_battle = false;
	m_moment = 0;
	m_numbers.clear();
	m_motion.clear();
}

bool StoryPresentation::Sound() const
{
	return m_sound;
}

void StoryPresentation::ToggleSound()
{
	m_sound = !m_sound;
	if (!m_sound)
	{
		m_hit.stopAllShots();
		m_call.stopAllShots();
		m_win.stopAllShots();
	}
}

void StoryPresentation::Tick(bool battle, double dt)
{
	const double step = dt < 0 ? Scene::DeltaTime() : Max(0., dt);
	m_time += step;
	m_moment = Max(0.0, m_moment - step);
	for (auto& motion : m_motion)
		motion.age += step;
	m_motion.remove_if([](const Motion& motion) { return motion.age >= .38; });
	for (auto& n : m_numbers)
		if (m_moment < .85)
			n.age += step;
	m_numbers.remove_if([](const Number& n) { return n.age > 1.1; });
	m_battle = battle;
}
