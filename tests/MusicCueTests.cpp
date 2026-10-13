#include "../NewGame/MusicCues.hpp"
#include <cassert>
#include <iostream>

int main()
{
	using namespace musiccues;
	assert(Campaign(false, 0, true) == Cue::Dawn);
	assert(Campaign(true, 0, false) == Cue::Continent);
	assert(Campaign(true, 0, true) == Cue::Battle);
	assert(Campaign(true, 1, true) == Cue::Victory);
	assert(Campaign(true, 2, true) == Cue::Storm);
	assert(Story(4, 2, true, false) == Cue::Dawn);
	assert(Story(4, 2, false, false) == Cue::Battle);
	assert(Story(3, 1, false, false) == Cue::Storm);
	assert(Story(1, 2, false, false) == Cue::Journey);
	assert(Story(4, 2, false, true) == Cue::Storm);
	for (int chapter = 0; chapter < 6; ++chapter)
	{
		assert(Story(chapter, 3, false, false) == Cue::Victory);
		assert(Story(chapter, 4, false, false) == Cue::Victory);
	}
	std::cout << "Music cue tests passed.\n";
}
