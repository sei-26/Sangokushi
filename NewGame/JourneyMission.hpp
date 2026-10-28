#pragma once
#include <algorithm>
#include "CampaignTypes.hpp"

namespace frontline
{
	enum class JourneyObjective
	{
		Breakout,
		Escort,
		HoldPositions,
        CaptureCamp
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
		      U"川は橋でのみ渡れる。北橋で回り込み、南橋の橋頭堡を崩して糧車の道を空けよう。",
		      JourneyObjective::Escort,
		      12,
		      12 + 11 * Width,
		      -1,
		      1200,
		      {{9 + 10 * Width, 8 + 9 * Width, 8 + 10 * Width, 12 + 11 * Width, 12 + 7 * Width}}},
		     {U"第三策 / 二つの烽火を守れ",
		      U"二つの烽火を同時に確保し、３指揮を耐える",
		      U"川の南北にある二つの烽火を３指揮維持。橋を使って救援。固守（兵500以上・士気40以上）なら隣接敵１隊まで維持"
		      U"。",
		      JourneyObjective::HoldPositions,
		      12,
		      7 + 6 * Width,
		      8 + 9 * Width,
		      3,
		      {{7 + 7 * Width, 7 + 5 * Width, 8 + 9 * Width, 11 + 9 * Width, 10 + 5 * Width}}}}};
		return missions[std::clamp(index, 0, 2)];
	}
    inline const JourneyMission& CampaignMissionDefinition(int index,bool northApproach=false)
    {
        static const JourneyMission port{U"第二策 / 河港の指揮所を奪え",
            U"川を渡り、敵の指揮所を安全に占領する",
            U"北橋で回り込むか南橋から突破。敵を退け、指揮所へ一隊を入れて確保すれば決着。護送と残敵掃討は不要。",
            JourneyObjective::CaptureCamp,10,12+11*Width,-1,0,
            {{9+10*Width,8+9*Width,8+10*Width,12+11*Width,12+7*Width}}};
        static const JourneyMission north=[] { auto m=port;m.positions[0]=9+8*Width;m.positions[1]=8+8*Width;m.positions[2]=9+9*Width;return m; }();
        return index==1 ? (northApproach ? north:port):MissionDefinition(index);
    }
} // namespace frontline
