#include "Campaign.hpp"

namespace frontline
{
	void Campaign::AdvanceDay()
	{
		if (result != 0)
			return;
		++day;
		// 1日の処理順。損害をまとめて反映してから部隊を移動させる。
		ResolveMissions();
		AdvanceAssignments();
		AdvanceCityWork();
		SupplyGrid supply;
		for (int faction = 0; faction < 3; ++faction)
			supply[faction] = Supply(faction);
		ConsumeDailySupply(supply);
		AdvanceAIArmies();
		const auto fighting = ResolveDailyCombat(supply);
		CancelInvalidAssignments();
		MoveArmies(fighting);
		FinishDay();
	}
} // namespace frontline
