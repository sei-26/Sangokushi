#pragma once
#include <algorithm>
#include "CampaignTypes.hpp"

namespace frontline
{
    inline constexpr int YellowTurbanMission=3;
    inline constexpr int CoalitionMission=4;
    enum class CoalitionPlan { None, Advance, Protect };
    inline bool CanOccupyObjective(const Army& unit)
    { return unit.troops>=150 && unit.morale>=25; }
    inline const char32_t* CoalitionPlanName(CoalitionPlan plan)
    {
        if(plan==CoalitionPlan::Advance) return U"曹操の先行隊に協力";
        if(plan==CoalitionPlan::Protect) return U"張邈の街道を守る";
        return U"協力先を選ぶ";
    }
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
    inline const JourneyMission& CampaignMissionDefinition(int index,bool northApproach=false,CoalitionPlan plan=CoalitionPlan::None,bool legacyMap=false)
    {
        static const JourneyMission port{U"第二策 / 河港の指揮所を奪え",
            U"川を渡り、敵の指揮所を安全に占領する",
            U"北橋で回り込むか南橋から突破。敵を退け、指揮所へ一隊を入れて確保すれば決着。護送と残敵掃討は不要。",
            JourneyObjective::CaptureCamp,10,12+11*Width,-1,0,
            {{9+10*Width,8+9*Width,8+10*Width,12+11*Width,12+7*Width}}};
        static const JourneyMission north=[] { auto m=port;m.positions[0]=9+8*Width;m.positions[1]=8+8*Width;m.positions[2]=9+9*Width;return m; }();
        static const JourneyMission guangzong{U"広宗の戦い ／ 黄巾の前陣を奪え",
            U"黄巾の前陣に一隊を入れ、周囲の敵を退けて確保する",
            U"正面の守備隊と南からの反撃に注意。林で敵を引きつけ、別の隊を前陣へ進めよう。",
            JourneyObjective::CaptureCamp,11,12+8*Width,-1,0,
            {{8+9*Width,8+8*Width,9+10*Width,12+8*Width,12+11*Width}}};
        if(index==YellowTurbanMission) return guangzong;
        if(index==CoalitionMission)
        {
            static const JourneyMission advance{U"酸棗の先陣 ／ 董卓軍の前哨を奪え",
                U"渡河して敵の前哨を占領する",
                U"北橋の先にある敵の前哨へ一隊を入れる。南橋へ別の隊を回せば、反撃を引き受けられる。",
                JourneyObjective::CaptureCamp,11,12+6*Width,-1,0,
                {{9+6*Width,8+6*Width,9+9*Width,12+6*Width,12+10*Width}}};
            static const JourneyMission protect{U"酸棗の街道 ／ 二つの烽火を守れ",
                U"街道の二つの烽火を同時に２指揮守る",
                U"街道の南北に守備を置き、二つの烽火を同時に２指揮維持する。東の林から来る敵の足を止めよう。",
                JourneyObjective::HoldPositions,10,9+6*Width,9+10*Width,2,
                {{8+6*Width,8+8*Width,8+10*Width,12+10*Width,12+6*Width}}};
            static const JourneyMission oldProtect=[] { auto m=protect;
                m.briefing=U"街道の南北に守備を置き、二つの烽火を同時に２指揮維持する。渡河してくる敵の足を止めよう。";return m; }();
            if(plan==CoalitionPlan::Protect) return legacyMap ? oldProtect:protect;
            return advance;
        }
        return index==1 ? (northApproach ? north:port):MissionDefinition(index);
    }
} // namespace frontline
