#pragma once
#include <string>

namespace frontline
{
	enum class OperationTrigger
	{
		Immediate,
		Date,
		AllyArrival,
		EnemySighted
	};
	enum class OperationState
	{
		None,
		Waiting,
		Executing,
		Holding,
		Aborted,
		Failed
	};
	struct BattleOperation
	{
		int target = -1;
		bool retreat = false, scout = false;
		OperationTrigger trigger = OperationTrigger::Immediate;
		int startDay = 1, signalArmy = -1, signalTile = -1, watchTile = -1;
		int issuedDay = 0;
		int minTroops = 0, minMorale = 0, deadline = 0, fallback = -1;
		OperationState state = OperationState::None;
		std::u32string status;
	};
	struct ArrivalSignal
	{
		int tile = -1, day = -1;
	};
} // namespace frontline
