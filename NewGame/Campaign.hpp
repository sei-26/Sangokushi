#pragma once
#include <algorithm>
#include <array>
#include <cmath>
#include <limits>
#include <queue>
#include <string>
#include <vector>
#include "CampaignTypes.hpp"
#include "OfficerCore.hpp"
#include "WorldLayout.hpp"

namespace frontline
{
	// Engine-independent simulation: one world, simultaneous daily combat,
	// continuous ownership, and supply corridors that enemies can cut.
	class Campaign
	{
	public:
		std::array<Tile, TileCount> tiles{};
		std::vector<City> cities;
		std::vector<General> generals;
		std::vector<Army> armies;
		std::vector<std::u32string> log;
		std::vector<Bond> bonds;
		std::vector<Mission> missions;
		std::vector<Chronicle> chronicle;
		std::array<std::array<int, 3>, 3> truceUntil{}, regard{};
		unsigned randomState = 89173;
		int player = 0, day = 0, commands = 3, result = 0;
		unsigned revision = 0;
		bool legacyLayout = false;
		int MapWidth() const;
		int MapHeight() const;
		static constexpr int MaxArmies = 6;
		static bool Valid(int p);
		static int At(int x, int y);
		static int Distance(int a, int b);
		static std::u32string FactionName(int f);
		static std::u32string ArmName(Arm a);
		static std::u32string DutyName(Duty d);
		static std::vector<int> Neighbors(int p);
		void Reset(int faction);
		void ResetLegacy(int faction);
		void Note(const std::u32string& text);
		int Affinity(int a, int b) const;
		void ChangeBond(int a, int b, int amount);
		bool Hostile(int a, int b) const;
		int Leader(int faction) const;
		int CityAt(int tile) const;
		bool Busy(int general) const;
		int WorkGain(int general, Duty duty, int helper = -1) const;
		bool Develop(int city, int general, Duty duty, bool ai = false, int helper = -1);
		bool CancelWork(int city);
		int ArmyCount(int faction) const;
		std::vector<int> Available(int city) const;
		int MissionChance(int general, int target, MissionKind kind, int helper = -1) const;
		bool SendMission(int city, int general, int target, MissionKind kind, int helper = -1,
		                 bool ai = false);
		unsigned Roll();
		void ResolveMissions();
		int SupportBond(int army) const;
		officer::Link Formation(int army) const;
		bool ActivateTactic(int index, bool ai = false);
		int Cost(int tile, Arm arm) const;
		std::vector<int> Route(int from, int to, Arm arm, int faction = -1) const;
		bool Order(int index, int target, bool retreat = false);
		int Deploy(int city, int general, int soldiers, Arm arm, bool ai = false);
		bool Recruit(int city, bool ai = false);
		int NearestCity(int tile, int faction, bool enemy) const;
		void BeginTurn();
		std::array<int, TileCount> Supply(int faction) const;
		void Return(Army& a, int city);
		int Fronts(int tile, int faction) const;
		void AdvanceDay();

	private:
		// Daily phases are called only by AdvanceDay(), in the order shown there.
		void AdvanceCityWork();
		void ConsumeDailySupply(const SupplyGrid& supply);
		std::vector<bool> ResolveDailyCombat(const SupplyGrid& supply);
		void MoveArmies(const std::vector<bool>& fighting);
		void FinishDay();
	};
} // namespace frontline
