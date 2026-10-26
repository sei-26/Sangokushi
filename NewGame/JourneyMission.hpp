#pragma once
#include <algorithm>
#include "CampaignTypes.hpp"

namespace frontline
{
	enum class JourneyObjective
	{
		Breakout,
		Escort,
		HoldPositions
	};
	struct JourneyMission
	{
		const char32_t* title;
		const char32_t* objective;
		const char32_t* briefing;
		JourneyObjective kind;
		int limit, goal, secondGoal, requiredProgress;
		std::array<int, 5> positions;
	};
	// Scenario data is deliberately independent of rendering and engine APIs.
	inline const JourneyMission& MissionDefinition(int index)
	{
		static const std::array<JourneyMission, 3> missions{
		    {{U"第一策 / 民の退路を開け",
		      U"先鋒を崩す、または安全な退路の出口を確保",
		      U"誘導・伏兵・突破を学ぶ戦い。先鋒の兵450以下か士気30以下でも退路を開ける。",
		      JourneyObjective::Breakout,
		      8,
		      12 + 10 * Width,
		      -1,
		      0,
		      {{9 + 10 * Width, 8 + 9 * Width, 8 + 10 * Width, 10 + 10 * Width, 14 + 11 * Width}}},
		     {U"第二策 / 糧車を守り抜け",
		      U"糧車を東端へ護送し、積荷1,200以上を残す",
		      U"先鋒を倒しても護送は終わらない。糧車の道を空け、敵の別働隊から守ろう。",
		      JourneyObjective::Escort,
		      10,
		      12 + 11 * Width,
		      -1,
		      1200,
		      {{9 + 10 * Width, 8 + 9 * Width, 8 + 10 * Width, 10 + 10 * Width, 14 + 11 * Width}}},
		     {U"第三策 / 二つの烽火を守れ",
		      U"二つの烽火を同時に確保し、３指揮を耐える",
		      U"両拠点を３指揮維持。固守（兵500以上・士気40以上）なら隣接敵１隊まで維持。２隊に囲まれると中断"
		      U"。",
		      JourneyObjective::HoldPositions,
		      12,
		      7 + 9 * Width,
		      8 + 11 * Width,
		      3,
		      {{7 + 10 * Width, 7 + 8 * Width, 8 + 11 * Width, 11 + 10 * Width, 12 + 7 * Width}}}}};
		return missions[std::clamp(index, 0, 2)];
	}
} // namespace frontline
