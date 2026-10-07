#include "Campaign.hpp"

namespace frontline
{
	void Campaign::AdvanceDay()
	{
		if (result != 0)
			return;
		++day;
		// The phase order is part of the rules. Resolve combat before moving armies.
		ResolveMissions();
		AdvanceCityWork();
		SupplyGrid supply;
		for (int faction = 0; faction < 3; ++faction)
			supply[faction] = Supply(faction);
		ConsumeDailySupply(supply);
		const auto fighting = ResolveDailyCombat(supply);
		MoveArmies(fighting);
		FinishDay();
	}
} // namespace frontline
