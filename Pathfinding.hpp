#pragma once
#include <Siv3D.hpp>
#include <queue>
#include <unordered_map>

// A* 経路探索（4方向）
// 戻り値: スタート→ゴールの座標列（見つからなければ空）
inline Array<Point> AStar(
	const Point start,
	const Point goal,
	const std::function<bool(Point)>& inBounds,
	const std::function<bool(Point)>& passable)
{
	auto h = [&](const Point& p) { return Abs(goal.x - p.x) + Abs(goal.y - p.y); };

	struct Node { Point p; int f; };
	struct Cmp { bool operator()(const Node& a, const Node& b) const { return a.f > b.f; } };

	std::priority_queue<Node, std::vector<Node>, Cmp> open;
	std::unordered_map<long long, Point> came;
	std::unordered_map<long long, int> g;

	auto key = [](const Point& p)-> long long { return (static_cast<long long>(p.y) << 32) | (unsigned)p.x; };

	auto push = [&](const Point& p, int f) { open.push(Node{ p, f }); };

	g[key(start)] = 0;
	push(start, h(start));

	const Point dirs[4] = { {1,0},{-1,0},{0,1},{0,-1} };

	while (!open.empty())
	{
		Point cur = open.top().p; open.pop();
		if (cur == goal) break;

		for (auto d : dirs)
		{
			Point nxt = cur + d;
			if (!inBounds(nxt) || !passable(nxt)) continue;

			int ng = g[key(cur)] + 1;
			auto it = g.find(key(nxt));
			if (it == g.end() || ng < it->second)
			{
				g[key(nxt)] = ng;
				came[key(nxt)] = cur;
				push(nxt, ng + h(nxt));
			}
		}
	}

	// 経路復元
	Array<Point> path;
	if (!came.count(key(goal)) && !(start == goal)) return path; // 空（未到達）
	Point cur = goal;
	path << cur;
	while (!(cur == start))
	{
		auto it = came.find(key(cur));
		if (it == came.end()) { path.clear(); return path; } // 念のため
		cur = it->second;
		path << cur;
	}
	path.reverse(); // start -> goal
	return path;
}
