#pragma once
#include <algorithm>
#include "CampaignTypes.hpp"
namespace citygrowth
{
	inline const char32_t* Name(int level)
	{
		const char32_t* names[] = {U"小城", U"発展都市", U"大都市", U"都府"};
		return names[level];
	}
	inline int Required(int level)
	{
		return level == 1 ? 55 : level == 2 ? 70 : 85;
	}
	inline int Eligible(const frontline::City& c)
	{
		for (int level = 3; level > 0; --level)
			if (std::min({c.farming, c.commerce, c.order, c.logistics}) >= Required(level))
				return level;
		return 0;
	}
} // namespace citygrowth
