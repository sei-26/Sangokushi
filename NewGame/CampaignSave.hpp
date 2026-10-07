#pragma once
#include <Siv3D.hpp>
#include "Campaign.hpp"

namespace frontline
{
	s3d::JSON SaveJSON(const Campaign& game);
	bool LoadJSON(const s3d::JSON& json, Campaign& game);
} // namespace frontline
