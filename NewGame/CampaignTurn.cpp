#include "Campaign.hpp"

namespace frontline
{
	void Campaign::AdvanceDay()
	{
		AdvanceDaily(false);
	}
	void Campaign::AdvanceBattleDay()
	{
		AdvanceDaily(true);
	}
	void Campaign::AdvanceDaily(bool battlefieldOnly)
	{
		tacticEvents.clear();
		returnEvents.clear();
		if (result != 0)
			return;
		++day;
		// 1日の処理順。損害をまとめて反映してから部隊を移動させる。
		ResolveMissions();
		AdvanceAssignments();
		AdvanceTraining();
		AdvanceCityWork();
		AdvanceCityGrowth();
		SupplyGrid supply;
		for (int faction = 0; faction < 3; ++faction)
			supply[faction] = Supply(faction);
		ConsumeDailySupply(supply);
		if (!battlefieldOnly) AdvanceAIArmies();
		const auto assembly = battlefieldOnly ? std::vector<bool>(armies.size()) : AdvanceAIOperations();
		auto fighting = ResolveDailyCombat(supply, !battlefieldOnly);
		for (size_t i = 0; i < fighting.size(); ++i)
			fighting[i] = fighting[i] || assembly[i];
		CancelInvalidAssignments();
		MoveArmies(fighting, !battlefieldOnly);
		AdvanceRegions();
		if (!battlefieldOnly) FinishDay();
	}
} // namespace frontline
