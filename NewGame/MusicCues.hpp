#pragma once
namespace musiccues
{
	enum class Cue
	{
		Dawn,
		Journey,
		Storm,
		Battle,
		Victory,
		Continent
	};
	inline const char32_t* File(Cue c)
	{
		const char32_t* files[] = {
		    U"Music/banner-of-dawn.wav",       U"Music/companions-road.wav",
		    U"Music/bridge-in-the-storm.wav",  U"Music/battle-of-the-burning-river.wav",
		    U"Music/oath-carried-forward.wav", U"Music/a-continent-under-banners.wav"};
		return files[static_cast<int>(c)];
	}
	inline const char32_t* Name(Cue c)
	{
		const char32_t* names[] = {U"黎明の旗",   U"仲間と歩む道", U"嵐の橋",
		                           U"紅蓮の決戦", U"誓いを明日へ", U"大陸に旗を掲げよ"};
		return names[static_cast<int>(c)];
	}
	inline Cue Story(int chapter, int phase, bool landing, bool failed)
	{
		if (landing)
			return Cue::Dawn;
		if (failed)
			return Cue::Storm;
		if (phase >= 3)
			return Cue::Victory;
		if (phase == 2 && (chapter == 0 || chapter >= 3))
			return Cue::Battle;
		if (chapter == 3)
			return Cue::Storm;
		if (chapter == 0 || chapter == 5)
			return Cue::Dawn;
		return Cue::Journey;
	}
	inline Cue Campaign(bool started, int result, bool combat)
	{
		if (!started)
			return Cue::Dawn;
		if (result == 1)
			return Cue::Victory;
		if (result == 2)
			return Cue::Storm;
		return combat ? Cue::Battle : Cue::Continent;
	}
} // namespace musiccues
