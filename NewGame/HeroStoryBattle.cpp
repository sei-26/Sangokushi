#include "HeroStory.hpp"

namespace hero
{
	bool Story::BattleReady() const
	{
		return phase == 2 && Tactical() && !failed && battleEvent == 0;
	}

	int Story::StepToward(int index, int targetX, int targetY) const
	{
		std::array<int, W * H> parent;
		parent.fill(-1);
		std::queue<int> q;
		const int start = units[index].y * W + units[index].x, goal = targetY * W + targetX;
		parent[start] = start;
		q.push(start);
		while (!q.empty())
		{
			const int p = q.front();
			q.pop();
			if (p == goal)
				break;
			for (const auto& delta : {std::pair<int, int>{1, 0}, {0, 1}, {0, -1}, {-1, 0}})
			{
				const int x = p % W + delta.first, y = p / W + delta.second;
				if (!Valid(x, y))
					continue;
				const int n = y * W + x;
				if (parent[n] >= 0 || terrain[n] == 2 || (At(x, y) >= 0 && n != goal))
					continue;
				parent[n] = p;
				q.push(n);
			}
		}
		if (parent[goal] < 0 || goal == start)
			return -1;
		int p = goal;
		while (parent[p] != start)
			p = parent[p];
		return p;
	}

	void Story::CheckBattle()
	{
		if (phase != 2 || failed)
			return;
		for (const auto& u : units)
			if (u.hero == 0 && u.hp <= 0)
			{
				Finish(false);
				return;
			}
		if (chapter == 3)
		{
			if (escaped >= (VowRequired() ? 4 : 3))
				Finish(true);
			else if (lost >= (VowRequired() ? 2 : 3))
				Finish(false);
		}
		else if (std::none_of(units.begin(), units.end(), [](const Unit& u) { return u.enemy && u.hp > 0; }))
			if (!VowRequired() || ObjectiveTile() < 0 || ObjectiveMet())
				Finish(true);
	}

	bool Story::Act(int index, int x, int y)
	{
		if (!BattleReady() || index < 0 || index >= static_cast<int>(units.size()) || !Valid(x, y))
			return false;
		auto& u = units[index];
		if (u.hp <= 0 || u.enemy || u.civilian || u.acted)
			return false;
		const int target = At(x, y), dist = std::abs(x - u.x) + std::abs(y - u.y);
		if (target >= 0 && units[target].enemy && CanAttack(index, target))
		{
			const auto link = Formation(index);
			const int bonus = officer::StoryAttack(link) + (deepRules && chapter == 5 &&
			                                                        objectiveProgress >= 2 && u.range > 1 &&
			                                                        std::abs(u.x - 6) + std::abs(u.y - 3) <= 2
			                                                    ? 1
			                                                    : 0);
			units[target].hp = std::max(
			    0, units[target].hp - std::max(1, u.attack + bonus - (terrain[y * W + x] == 1 ? 1 : 0)));
			GainSpirit(8 + bonus * 4 + link.morale + (units[target].hp == 0 ? 10 : 0));
			if (units[target].hp == 0)
				Record(Name(u.hero) + U"が敵隊を撃破！ 戦列に道が開いた。");
			else if (bonus > 0)
				Record(Name(u.hero) + U"「仲間となら、押し切れる！」 信頼による連携攻撃。");
		}
		else if (target < 0 && dist == 1 && terrain[y * W + x] != 2 && (!deepRules || !u.moved))
		{
			u.x = x;
			u.y = y;
			if (deepRules)
			{
				u.moved = true;
				return true;
			}
		}
		else
			return false;
		u.acted = true;
		CheckBattle();
		return true;
	}

	void Story::EndTurn()
	{
		if (!BattleReady())
			return;
		for (int i = 0; i < static_cast<int>(units.size()); ++i)
		{
			auto& e = units[i];
			if (!e.enemy || e.hp <= 0)
				continue;
			if (e.stunned > 0)
			{
				--e.stunned;
				continue;
			}
			int target = -1, distance = 100;
			for (int j = 0; j < static_cast<int>(units.size()); ++j)
				if (!units[j].enemy && units[j].hp > 0 && Distance(e, units[j]) < distance)
				{
					target = j;
					distance = Distance(e, units[j]);
				}
			EnemyIntent intent;
			if (deepRules)
			{
				intent = PlanEnemy(i);
				target = intent.target;
			}
			if (target < 0)
				continue;
			if (CanAttack(i, target))
			{
				auto& victim = units[target];
				victim.hp = std::max(
				    0, victim.hp - std::max(1, e.attack - (terrain[victim.y * W + victim.x] == 1 ? 1 : 0) -
				                                   officer::StoryDefense(Formation(target)) -
				                                   (victim.guarding ? 2 : 0)));
				if (victim.guarding && victim.hp > 0 && Distance(e, victim) == 1)
					e.hp = std::max(0, e.hp - 1);
				if (victim.hp == 0 && victim.civilian)
					++lost;
				GainSpirit(4);
				if (victim.hero >= 0 && victim.hp > 0 && victim.hp <= victim.maxHp / 3)
					Record(Name(victim.hero) + U"「まだ倒れるわけにはいかぬ……！」 窮地の仲間を支えよ。");
			}
			else
			{
				const int next = deepRules ? intent.next : StepToward(i, units[target].x, units[target].y);
				if (next >= 0 && At(next % W, next / W) < 0)
				{
					e.x = next % W;
					e.y = next / W;
				}
			}
		}
		if (chapter == 3)
			for (int i = 0; i < static_cast<int>(units.size()); ++i)
			{
				auto& c = units[i];
				if (!c.civilian || c.hp <= 0)
					continue;
				const int next = StepToward(i, 10, 3);
				if (next >= 0 && At(next % W, next / W) < 0)
				{
					c.x = next % W;
					c.y = next / W;
				}
				if (c.x == 10 && c.y == 3)
				{
					c.hp = 0;
					++escaped;
					GainSpirit(20);
					Record(
					    U"民「ありがとう……皆様も、どうかご無事で！」 避難する人々が渡し場へ到着。闘志+20。");
				}
			}
		AdvanceObjective();
		++turn;
		for (auto& u : units)
		{
			u.acted = false;
			u.moved = false;
			u.guarding = false;
		}
		CheckBattle();
		if (phase == 2 && turn > TurnLimit())
			Finish(false);
		if (phase == 2 && !failed && turn >= 4 && !(eventMask & 1))
		{
			eventMask |= 1;
			battleEvent = 1;
			Record(chapter == 3
			           ? U"追手の圧力が増す。張飛「橋は俺が守る！」 趙雲「民の傷が深い。判断を、将軍！」"
			           : U"戦列がぶつかる。ここで勢いをつかむか、傷ついた仲間を守るか。あなたの決断が戦場を動"
			             U"かす。");
		}
	}
} // namespace hero
