#pragma once
#include "TacticChain.hpp"
#include "JourneyMission.hpp"
#include "JourneyEnemyPlan.hpp"
#include "JourneyOpening.hpp"
#include <optional>
#include "JourneyStory.hpp"
#include "JourneyAssignment.hpp"
#include "JourneyDifficulty.hpp"

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
	struct JourneySupplies
	{
		int food, gold, cargo, escort;
		int TotalFood() const { return food + cargo; }
		int MinimumTroops() const { return 2800 + escort; }
	};
	enum class JourneyPromise { OpenRoad, EscortRefugees };
	struct JourneyResult
	{
		bool completed = false, won = false, withdrawn = false, rescued = false;
		int survivors = 0, casualties = 0;
        JourneyBattleReview review;
        JourneyConvoyRoute route=JourneyConvoyRoute::Road;
        JourneyBattleStory story;
        JourneyDifficulty difficulty=JourneyDifficulty::Standard;
        bool campaignFlow=false;
        std::array<JourneyAssignment,3> assignments{};
	};
	// One persistent campaign for the first playable historical-background slice.
	// Character conversations and these three local engagements are original fiction.
	class StrategistJourney
	{
	public:
		static constexpr int Columns = 16, Rows = 14, TurnLimit = 8;
		Campaign world;
		TacticChain chain;
		std::array<BattleIntelligence, 2> intelligence;
		JourneyStage stage = JourneyStage::Encounter;
		int mission = 0, turn = 1, enemyPolicy = 0, trust = 0, bestChain = 0;
        std::array<int,3> officerTrust{}; // Each officer's trust in the protagonist, separate from pair affinity.
		int preparation = 0, deployed = 0, casualties = 0, survivors = 0;
        int reserveFood=0; // Paid, unissued supply. Never reconstructed for a loaded battle.
		int goal = Campaign::At(12, 10);
		static constexpr int ConvoyArmy = 5;
		int heldTurns = 0, convoyStep = 0, convoyEscort = 0, recoveredCargo = 0;
		std::array<bool, 3> guarding{};
		bool won = false, reinforced = false;
		JourneyPromise promise = JourneyPromise::OpenRoad;
        JourneyConvoyRoute convoyRoute=JourneyConvoyRoute::Road;
        JourneyBattleReview battleReview;
        std::optional<JourneyReliefRecord> retryRelief; // Non-canonical lesson, never a settled battle or reward.
        JourneyDifficulty difficulty=JourneyDifficulty::Standard;
        bool campaignFlow=false;
        bool SelectDifficulty(JourneyDifficulty);
        bool ApplyPlanExample();
        bool RetryAtCouncil(const StrategistJourney& beforeBattle);
        std::array<JourneyAssignment,3> assignments{};
        std::u32string AssignmentProblem(int officer,int tile,JourneyDuty) const;
        bool AssignDuty(int officer,int tile,JourneyDuty);
        std::u32string AssignmentStatus(int officer) const;
		bool withdrawn = false;
		std::array<JourneyResult, 3> history{};
		ChainResult lastChain;
		std::vector<Chronicle> reports;
		std::u32string message;
		void Reset();
        void ResetCampaign();
        bool UpgradeCampaignAtCouncil();
		void Meet();
		bool Reinforce();
		JourneySupplies PreparationCosts(int choice) const;
        std::array<int,3> PreparationPositions(int choice) const;
        int PreparationMorale(int choice,int officer) const;
        bool CanResupply() const;
        bool Resupply();
		bool CanPrepare(int choice) const;
		bool SelectPromise(JourneyPromise value);
        bool SelectConvoyRoute(JourneyConvoyRoute);
        const std::vector<int>& ConvoyPath() const;
        const char32_t* ResolutionName() const;
        std::u32string BattleReviewLesson() const;
		bool Withdraw();
		bool IsRescue() const { return mission == 0 && promise == JourneyPromise::EscortRefugees; }
		bool HasConvoy() const { return IsRescue() || Definition().kind == JourneyObjective::Escort; }
        int TurnBudget() const { return (IsRescue() ? 10 : Definition().limit)+CombatSettings(difficulty).extraCycles+
            (campaignFlow && mission==1 && history[0].rescued ? 2:0); }
        int RequiredHoldTurns() const
        { const int mask=history[1].review.coordinatedOfficers;return campaignFlow && mission==2 && history[1].won && (mask&(mask-1)) ? 2:Definition().requiredProgress; }
        std::u32string ObjectiveBrief() const;
		bool Prepare(int choice, int policy);
		bool Move(int army, int tile);
		bool Ambush(int army);
		bool Strike(int army, int enemy);
		bool Hold(int army);
		const JourneyMission& Definition() const
		{
			return campaignFlow ? CampaignMissionDefinition(mission,CurrentPledge()==JourneyPledge::Preserve):MissionDefinition(mission);
		}
		bool PositionSecured(int tile) const;
		std::u32string ObjectiveProgress() const;
		ChainResult Preview(const std::vector<ChainOrder>& orders) const;
		ChainResult Execute(const std::vector<ChainOrder>& orders);
		bool EndTurn();
		bool Continue();
        bool CanContinue() const;
        bool NeedsStoryReply() const;
        bool ChooseStoryReply(JourneyPledge);
        JourneyPledge CurrentPledge() const;
        std::u32string PledgeProgress() const;
        int StoryMoraleBonus(int officer) const;
		void RefreshIntelligence();
        void RecordReliefOrder(int outgoing,int incoming,int source,int destination,bool realtime);
        void ValidateReliefOrders(const class JourneyRealtime&);
		// Estimate from current player contacts, never private enemy orders.
		std::vector<JourneyEnemyPlan> ReadEnemyPlans() const;
		static bool OnMap(int tile);
		static const char32_t* EnemyName(int policy);
		const char32_t* MissionName() const;

	private:
        friend class JourneyRealtime;
        std::vector<JourneyEnemyPlan> PlanEnemyTurn() const;
        void ApplyEnemyPlans(const std::vector<JourneyEnemyPlan>& plans);
        bool CompleteCommandCycle(std::size_t previousReports);
		void EnemyTurn();
		void AdvanceObjective();
		void SetupBattlefield();
		void Evaluate();
		void Finish(bool success, const char32_t* reason);
        void CompleteBattleReview(bool success);
        void CompleteBattleStory(bool success);
        void RecordCooperation(const ChainResult&);
        void RecordAssignment(int officer);
        void RecordReliefMove(int army,int from,int to);
        void CheckReliefOutcome();
        void FinalizeRelief();
		void Report(const std::u32string& text);
	};
} // namespace frontline
