#include "HeroStory.hpp"
namespace hero
{
	EnemyIntent Story::PlanEnemy(int index) const
	{
		EnemyIntent result;
		if (index < 0 || index >= static_cast<int>(units.size()) || !units[index].enemy ||
		    units[index].hp <= 0 || units[index].stunned > 0)
			return result;
		const auto& e = units[index];
		const int start = e.y * W + e.x;
		std::array<int, W * H> distance, parent;
		distance.fill(1000);
		parent.fill(-1);
		std::queue<int> q;
		distance[start] = 0;
		q.push(start);
		while (!q.empty())
		{
			const int p = q.front();
			q.pop();
			for (const auto& d : {std::pair<int, int>{1, 0}, {0, 1}, {-1, 0}, {0, -1}})
			{
				const int x = p % W + d.first, y = p / W + d.second;
				if (!Valid(x, y) || terrain[y * W + x] == 2 || At(x, y) >= 0 || distance[y * W + x] != 1000)
					continue;
				const int n = y * W + x;
				distance[n] = distance[p] + 1;
				parent[n] = p;
				q.push(n);
			}
		}
		int best = -100000, goal = -1;
		for (int j = 0; j < static_cast<int>(units.size()); ++j)
		{
			const auto& victim = units[j];
			if (victim.enemy || victim.hp <= 0)
				continue;
			for (int p = 0; p < W * H; ++p)
			{
				if (distance[p] >= 1000 || std::abs(p % W - victim.x) + std::abs(p / W - victim.y) > e.range)
					continue;
				if (!battle::ClearRay(p % W, p / W, victim.x, victim.y, [&](int x, int y) {
					    return terrain[y * W + x] == 1 || terrain[y * W + x] == 2;
				    }))
					continue;
				const int damage =
				    std::max(1, e.attack - (terrain[victim.y * W + victim.x] == 1 ? 1 : 0) -
				                    officer::StoryDefense(Formation(j)) - (victim.guarding ? 2 : 0));
				const int score =
				    200 - distance[p] * 22 + (victim.hp <= damage ? 80 : 0) + damage * 8 +
				    (chapter == 3 && victim.civilian ? 28 : 0) + (victim.hero == 0 ? 12 : 0) -
				    (victim.guarding && std::abs(p % W - victim.x) + std::abs(p / W - victim.y) == 1 ? 18
				                                                                                     : 0) +
				    (e.range > 1 && terrain[p] == 1 ? 5 : 0);
				if (score > best)
				{
					best = score;
					result.target = j;
					goal = p;
				}
			}
		}
		if (goal >= 0 && goal != start)
		{
			int next = goal;
			while (parent[next] != start && parent[next] >= 0)
				next = parent[next];
			if (parent[next] == start)
				result.next = next;
		}
		return result;
	}
} // namespace hero
