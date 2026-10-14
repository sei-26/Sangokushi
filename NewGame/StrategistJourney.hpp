#pragma once
#include "TacticChain.hpp"

namespace frontline
{
	enum class JourneyStage
	{
		Encounter,
		Preparation,
		Battle,
		Debrief,
		Complete
	};
	// One persistent campaign for the first playable historical-background slice.
	// Character conversations and these two local engagements are original fiction.
	class StrategistJourney
	{
	public:
		static constexpr int Columns = 16, Rows = 14, TurnLimit = 8;
		Campaign world;
		TacticChain chain;
		std::array<BattleIntelligence, 2> intelligence;
		JourneyStage stage = JourneyStage::Encounter;
		int mission = 0, turn = 1, enemyPolicy = 0, trust = 0, bestChain = 0;
		int preparation = 0, deployed = 0, casualties = 0, survivors = 0;
		int goal = Campaign::At(12, 10);
		bool won = false, reinforced = false;
		ChainResult lastChain;
		std::vector<Chronicle> reports;
		std::u32string message;
		void Reset();
		void Meet();
		bool Reinforce();
		bool Prepare(int choice, int policy);
		bool Move(int army, int tile);
		bool Ambush(int army);
		bool Strike(int army, int enemy);
		ChainResult Preview(const std::vector<ChainOrder>& orders) const;
		ChainResult Execute(const std::vector<ChainOrder>& orders);
		bool EndTurn();
		bool Continue();
		void RefreshIntelligence();
		static bool OnMap(int tile);
		static const char32_t* EnemyName(int policy);
		const char32_t* MissionName() const;

	private:
		void EnemyTurn();
		void Evaluate();
		void Finish(bool success, const char32_t* reason);
		void Report(const std::u32string& text);
	};
} // namespace frontline
