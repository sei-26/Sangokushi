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
#include "CityIdentity.hpp"
#include "WorldLayout.hpp"
#include "HexGrid.hpp"

namespace frontline
{
	// Engine-independent simulation: one world, simultaneous daily combat,
	// continuous ownership, and supply corridors that enemies can cut.
	class Campaign
	{
	public:
		std::array<Tile, TileCount> tiles{};
		std::vector<City> cities;
		std::vector<Region> regions;
		std::array<int, TileCount> tileRegion{};
		std::vector<General> generals;
		std::vector<Army> armies;
		std::vector<TacticEvent> tacticEvents;
		std::vector<std::u32string> log;
		std::vector<Bond> bonds;
		std::vector<Mission> missions;
		std::vector<Assignment> assignments;
		std::vector<Chronicle> chronicle;
		std::array<std::array<int, 3>, 3> truceUntil{}, regard{};
		unsigned randomState = 89173;
		int player = 0, day = 0, commands = 3, result = 0;
		unsigned revision = 0;
		bool legacyLayout = false, hexMap = false;
		std::array<int, 3> aiPlannedDay{{-1, -1, -1}}, aiCommands{{3, 3, 3}};
		int MapWidth() const;
		int MapHeight() const;
		static constexpr int MaxArmies = 6;
		static bool Valid(int p);
		static int At(int x, int y);
		static int Distance(int a, int b);
		int MapDistance(int a, int b) const;
		std::vector<int> MapNeighbors(int tile) const;
		bool ClearShot(int from, int to) const;
		static std::u32string FactionName(int f);
		static std::u32string ArmName(Arm a);
		static std::u32string DutyName(Duty d);
		static std::vector<int> Neighbors(int p);
		void BuildRegions();
		int RegionAt(int tile) const;
		int RegionCoverage(int region) const;
		bool RegionConnected(int region) const;
		void Reset(int faction);
		void AddExpandedRoster();
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
		cityidentity::Kind CityKind(int city) const;
		int Governor(int city) const;
		bool AppointGovernor(int city, int general, bool ai = false);
		int CityIncome(int city, bool food, int candidate = -2) const;
		int CityLogistics(int city, int candidate = -2) const;
		int CityDamagePercent(int city, int candidate = -2) const;
		int Recruitment(int city) const;
		int CityWorkGain(int city) const;
		int GovernorWorkBonus(int city, int general) const;
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
		bool SetStance(int index, battle::Stance stance, bool ai = false);
		int AttackRange(int index) const;
		int OfficerAttackPercent(int army, bool siege = false) const;
		int OfficerDamagePercent(int army) const;
		bool CanStrike(int index, int tile) const;
		int Cost(int tile, Arm arm) const;
		std::vector<int> Route(int from, int to, Arm arm, int faction = -1) const;
		bool Order(int index, int target, bool retreat = false);
		int Deploy(int city, int general, int soldiers, Arm arm, bool ai = false);
		int DispatchTransport(int city, int general, int targetCity, int cargo, bool ai = false);
		int TransportDays(int from, int to, int faction) const;
		bool Recruit(int city, bool ai = false);
		int NearestCity(int tile, int faction, bool enemy) const;
		void BeginTurn();
		std::array<int, TileCount> Supply(int faction) const;
		std::array<bool, TileCount> SupplyBlockade(int faction) const;
		int PressureDirections(int army) const;
		int PressureDamagePercent(int army) const;
		int PressureMoraleLoss(int army) const;
		void Return(Army& a, int city);
		bool ReturnToCity(int army, int city);
		int Fronts(int tile, int faction) const;
		void AdvanceDay();
		int AssignmentDays(int from, int to) const;
		bool AssignOfficer(int general, int target, bool ai = false);

	private:
		enum class AIKind
		{
			Govern,
			Develop,
			Recruit,
			Deploy,
			Transport,
			Mission,
			Transfer,
			Stance
		};
		struct AIOrder
		{
			AIKind kind;
			int priority = 0, city = -1, general = -1, target = -1, amount = 0;
			Arm arm = Arm::Spear;
			Duty duty = Duty::Farming;
			MissionKind mission = MissionKind::Sabotage;
			const char32_t* reason = U"";
		};
		int AIThreat(int city) const;
		int AIEnemyCity(int from, int faction, Arm arm, int troops) const;
		std::vector<AIOrder> AIPlan(int faction) const;
		bool ExecuteAI(int faction, const AIOrder& order);
		void AdvanceAIArmies();
		std::vector<bool> AdvanceAIOperations();
		int AITargetArmy(int index) const;
		bool AIUseTactic(int index, const SupplyGrid& supply) const;
		// Daily phases are called only by AdvanceDay(), in the order shown there.
		void AdvanceCityWork();
		void AdvanceAssignments();
		void RedistributeOfficers(int faction);
		void CancelInvalidAssignments();
		void ConsumeDailySupply(const SupplyGrid& supply);
		std::vector<bool> ResolveDailyCombat(const SupplyGrid& supply);
		void MoveArmies(const std::vector<bool>& fighting);
		void AdvanceRegions();
		void RegionIncome();
		void FinishDay();
	};
} // namespace frontline
