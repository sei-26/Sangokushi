#pragma once
#include "StoryBook.hpp"
#include "OfficerCore.hpp"
#include <algorithm>
#include <queue>
#include <vector>
#include <cmath>

namespace hero
{
	constexpr int W = 11, H = 7;
	struct Unit
	{
		int hero = -1, x = 0, y = 0, hp = 8, maxHp = 8, attack = 2, range = 1;
		bool enemy = false, civilian = false, acted = false, skillUsed = false;
		int stunned = 0;
	};
	class Story
	{
	public:
		int chapter = 0, phase = 0, virtue = 50, resolve = 50, food = 120, gold = 100, reputation = 0;
		int turn = 1, tasks = 0, order = 40, visits = 0, escaped = 0, lost = 0, preparation = 0;
		bool failed = false, fireUsed = false;
		int spirit = 20, battleEvent = 0, eventMask = 0, rallies = 0;
		int planner = 1;
		std::vector<officer::Bond> relationships{{1, 2, 40}};
		std::vector<int> battleDecisions;
		std::array<int, 6> bonds{{50, 40, 40, 30, 20, 20}};
		std::vector<int> decisions;
		std::vector<std::u32string> journal;
		std::array<int, W * H> terrain{};
		std::vector<Unit> units;
		void Reset();
		bool Tactical() const;
		int TurnLimit() const;
		static bool Valid(int x, int y);
		static int Distance(const Unit& a, const Unit& b);
		void Record(const std::u32string& text);
		int At(int x, int y) const;
		int Companion() const;
		int Unlocked() const;
		int Affinity(int a, int b) const;
		void ChangeBond(int a, int b, int amount);
		bool AssignPlanner(int who);
		int PlanPower(int plan) const;
		officer::Link Formation(int index) const;
		void Clamp();
		void GainSpirit(int amount);
		bool BattleReady() const;
		bool Rally();
		bool ResolveBattleEvent(int choice);
		bool Choose(int choice);
		void StartMission();
		void Finish(bool success);
		void Retry();
		bool CivilAction(int action);
		int StepToward(int index, int targetX, int targetY) const;
		void CheckBattle();
		bool Act(int index, int x, int y);
		bool Skill(int index);
		bool FireSignal();
		void EndTurn();
		std::u32string Ending() const;
	};
} // namespace hero
