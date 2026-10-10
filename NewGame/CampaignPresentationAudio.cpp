#include "CampaignPresentation.hpp"
#include <cmath>
Wave CampaignPresentation::Sound(frontline::Tactic tactic)
{
	using frontline::Tactic;
	constexpr int rate = 22050;
	const int count = rate * 9 / 10;
	Wave wave(static_cast<size_t>(count), Arg::sampleRate = rate);
	uint32 seed = 11939 + static_cast<int>(tactic) * 293;
	for (int i = 0; i < count; ++i)
	{
		const double t = static_cast<double>(i) / rate;
		seed = seed * 1664525u + 1013904223u;
		const double noise = ((seed >> 8) & 65535) / 32768. - 1.;
		const double attack = Min(1., t / .015), end = Min(1., (count - i) / (rate * .07));
		const double hit = Max(0., t - .16);
		const double impact =
		    t >= .16 ? (.3 * std::sin(Math::TwoPi * 65 * hit) + .2 * noise) * std::exp(-15 * hit) : 0.;
		double sound = 0.;
		if (tactic == Tactic::Rally)
			sound = (.3 * std::sin(Math::TwoPi * 146.83 * t) + .13 * std::sin(Math::TwoPi * 220 * t) +
			         .09 * std::sin(Math::TwoPi * 293.66 * t)) *
			            std::exp(-2.5 * t) +
			        impact;
		else if (tactic == Tactic::Supply)
			sound = (.28 * std::sin(Math::TwoPi * 440 * t) + .13 * std::sin(Math::TwoPi * 660 * t)) *
			            std::exp(-6 * t) +
			        .13 * std::sin(Math::TwoPi * 880 * t) * std::exp(-5 * Max(0., t - .25));
		else if (tactic == Tactic::Fire || tactic == Tactic::Ambush)
			sound = noise * .38 * std::sin(Min(1., t / .15) * Math::HalfPi) * std::exp(-3 * t) +
			        .24 * std::sin(Math::TwoPi * (70 * t + 35 * t * t)) * std::exp(-5 * t) + impact;
		else if (tactic == Tactic::Fortify)
			sound = (.3 * std::sin(Math::TwoPi * 240 * t) + .16 * std::sin(Math::TwoPi * 361 * t) +
			         .1 * std::sin(Math::TwoPi * 513 * t)) *
			            std::exp(-7 * t) +
			        impact;
		else if (tactic == Tactic::Volley)
			sound = .28 * noise * std::exp(-4 * t) +
			        .23 * std::sin(Math::TwoPi * (1200 * t - 650 * t * t)) * std::exp(-7 * t) + impact * .5;
		else
			sound = .32 * noise * std::exp(-8 * t) +
			        .27 * std::sin(Math::TwoPi * (95 * t - 35 * t * t)) * std::exp(-5 * t) + impact;
		wave[i] = static_cast<float>(Clamp(sound * attack * end, -.9, .9));
	}
	return wave;
}
