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
		w[i] = static_cast<float>(v * .6);
	}
	return w;
}

Wave StoryPresentation::Score()
{
	constexpr int rate = 22050;
	Wave w(static_cast<size_t>(rate * 16), Arg::sampleRate = rate);
	const double notes[]{146.83, 174.61, 196.00, 220.00, 196.00, 174.61, 130.81, 146.83};
	uint32 seed = 71;
	for (size_t i = 0; i < w.size(); ++i)
	{
		double t = static_cast<double>(i) / rate, beat = std::fmod(t, .5), bar = std::fmod(t, 2.0),
		       noteTime = std::fmod(t, 1.0);
		seed = seed * 1664525u + 1013904223u;
		double noise = static_cast<double>((seed >> 8) & 65535) / 32768 - 1;
		double drum = std::sin(6.283185 * (70 * beat - 45 * beat * beat)) * std::exp(-18 * beat) * .28 +
		              noise * std::exp(-65 * beat) * .06;
		double gong = std::sin(6.283185 * 98 * t) * std::exp(-3 * bar) * .12;
		double n = notes[static_cast<int>(t) % 8],
		       melody = (std::sin(6.283185 * n * t) + .20 * std::sin(6.283185 * n * 2 * t)) *
		                std::sin(3.14159 * noteTime) * .065;
		double fade = Min(1.0, Min(t * 4, (16 - t) * 4));
		w[i] = static_cast<float>((drum + gong + melody) * fade);
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
	m_music = Audio(Score(), Loop::Yes);
}

void StoryPresentation::Stop()
{
	m_music.stop();
	m_hit.stopAllShots();
	m_call.stopAllShots();
	m_win.stopAllShots();
	m_battle = false;
	m_moment = 0;
	m_numbers.clear();
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
		m_music.stop();
		m_hit.stopAllShots();
		m_call.stopAllShots();
		m_win.stopAllShots();
	}
	else if (m_battle)
		m_music.setVolume(.32).play();
}

void StoryPresentation::Tick(bool battle)
{
	m_time += Scene::DeltaTime();
	m_moment = Max(0.0, m_moment - Scene::DeltaTime());
	for (auto& n : m_numbers)
		n.age += Scene::DeltaTime();
	m_numbers.remove_if([](const Number& n) { return n.age > 1.1; });
	if (m_battle != battle)
	{
		m_battle = battle;
		if (m_sound && m_loaded)
		{
			if (battle)
				m_music.setVolume(.32).play();
			else
				m_music.stop();
		}
	}
}
