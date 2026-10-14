#pragma once
#include "Campaign.hpp"
#include "BattleIntelligence.hpp"
#include "BattleOperation.hpp"

namespace frontline
{
	// The existing battlefield rules resolve reality; intelligence and operations
	// define what each faction knows and which orders it can issue.
	class StrategistBattle
	{
	public:
		static constexpr int MapColumns = 24, MapRows = 18;
		static constexpr int Deadline = 45, RequiredCargo = 3500, RequiredSoldiers = 7500;
		static constexpr int EnemyAssemblyDays = 10;
		static constexpr int Convoy = 4, Destination = 1;
		Campaign world;
		int plan = 0, enemyPolicy = 0, deliveredCargo = 0, evacuatedSoldiers = 0;
		int outcome = 0; // 0 ongoing, 1 success, 2 failure
		bool prepared = false;
		bool intelligenceEnabled = true;
		std::array<BattleIntelligence, 2> intelligence;
		std::array<BattleOperation, 5> operations;
		std::array<ArrivalSignal, 5> arrivals;
		std::vector<Chronicle> reports;
		std::array<int, 5> disposition{}; // 0 field, 1 returned, 2 routed
		std::u32string reason;
		void Reset();
		bool SetPlan(int choice);
		bool Order(int army, int tile, bool retreat = false);
		bool SetStance(int army, battle::Stance stance);
		bool QueueOperation(int army, BattleOperation operation);
		void RefreshIntelligence();
		std::vector<int> KnownRoute(int army, int target) const;
		int EnemyTarget(int army) const;
		static const char32_t* TriggerName(OperationTrigger trigger);
		static const char32_t* StateName(OperationState state);
		void AdvanceDay();
		int RemainingSoldiers() const;
		int Cargo() const;
		static bool OnMap(int tile);
		static const char32_t* PlanName(int choice);
		static const char32_t* PlanDescription(int choice);
		static const char32_t* RoleName(int army);
		static const char32_t* EnemyPolicyName(int choice);

	private:
		void EnemyOrders();
		void Evaluate();
		void ResolveOperations();
		void RecordArrivals();
		void Report(const std::u32string& text);
	};
} // namespace frontline
