#pragma once
namespace nationalpolicy
{
	inline const char32_t* Name(int policy)
	{
		const char32_t* names[] = {U"屯田制", U"通商振興", U"仁政", U"軍需整備"};
		return names[policy];
	}
	inline const char32_t* Description(int policy)
	{
		const char32_t* descriptions[] = {
		    U"負担：金収入補正 -5%", U"負担：兵糧収入補正 -5% / 募兵 -10%",
		    U"負担：金収入補正 -5%", U"負担：金収入補正 -5%"};
		return descriptions[policy];
	}
} // namespace nationalpolicy
