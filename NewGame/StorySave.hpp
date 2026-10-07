#pragma once
#include <Siv3D.hpp>
#include "HeroStory.hpp"

namespace hero
{
	s3d::JSON SaveJSON(const Story& s);
	bool LoadJSON(const s3d::JSON& json, Story& game);
} // namespace hero
