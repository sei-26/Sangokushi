#pragma once
#include "BattleIntelligence.hpp"

namespace frontline
{
	enum class ChainTactic
	{
		Lure,
		Ambush,
		Breakthrough,
		Feint,
		Disrupt,
		Oath,
		AzureDragon,
		SerpentRoar
	};
	struct ChainTacticDefinition
	{
		ChainTactic id;
		const char32_t *name, *description;
		int moraleCost;
		bool unique, targetsEnemy;
	};
	struct ChainUnitChange
	{
		int army, tile, troops, morale;
		bool concealed, disordered, pressured, inspired;
	};
	struct ChainOrder
	{
		int army = -1, enemy = -1;
		ChainTactic tactic = ChainTactic::Lure;
		int destination = -1; // Lure withdrawal tile; other tactics derive their displacement.
	};
	struct ChainEvent
	{
		int army = -1, enemy = -1, from = -1, to = -1, damage = 0;
		ChainTactic tactic = ChainTactic::Lure;
		bool success = false;
		std::u32string reason;
		std::vector<ChainUnitChange> changes; // Per-event facts for playback, never final-state guesses.
	};
	struct ChainResult
	{
		std::vector<ChainEvent> events;
		int completed = 0;
		bool prediction = false;
	};
	// Battle-local state. Preview resolves a sanitized observation copy, never the
	// real hidden enemy positions. Actual resolution checks the real occupancy.
	class TacticChain
	{
	public:
		std::vector<bool> concealed, moved, spent, disordered, pressured, inspired, signatureUsed;
		void Reset(const Campaign& world);
		void BeginTurn(const Campaign& world);
		bool ArmAmbush(Campaign& world, int army, std::u32string& reason);
		bool Move(Campaign& world, const BattleIntelligence& known, int army, int tile,
		          std::u32string& reason);
		bool Strike(Campaign& world, const BattleIntelligence& known, int army, int enemy,
		            std::u32string& reason);
		ChainResult Preview(const Campaign& world, const BattleIntelligence& known,
		                    const std::vector<ChainOrder>& orders) const;
		ChainResult Execute(Campaign& world, const BattleIntelligence& known,
		                    const std::vector<ChainOrder>& orders);
		static const char32_t* Name(ChainTactic tactic);
		static const ChainTacticDefinition& Definition(ChainTactic tactic);
		static std::vector<ChainTactic> Loadout(const Campaign& world, int army);
		static bool CanUse(const Campaign& world, int army, ChainTactic tactic);
		static bool Living(const Campaign& world, int army);
		static bool Free(const Campaign& world, int tile);
		static int PushTile(const Campaign& world, int attacker, int enemy);

	private:
		ChainResult Resolve(Campaign& world, const std::vector<ChainOrder>& orders);
		ChainEvent Apply(Campaign& world, const ChainOrder& order);
	};
} // namespace frontline
