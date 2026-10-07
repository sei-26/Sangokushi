#include "StorySave.hpp"

namespace hero
{
	bool LoadJSON(const s3d::JSON& j, Story& output)
	{
		try
		{
			const int version = j[U"version"].get<int>();
			if (version < 1 || version > 3)
				return false;
			s3d::Array<int> a;
			for (const auto& v : j[U"state"].arrayView())
				a.push_back(v.get<int>());
			if (a.size() != 16 || a[0] < 0 || a[0] > 5 || a[1] < 0 || a[1] > 4 || (a[1] == 4 && a[0] != 5))
				return false;
			for (int i : {2, 3, 9})
				if (a[i] < 0 || a[i] > 100)
					return false;
			for (int i : {4, 5})
				if (a[i] < 0 || a[i] > 999)
					return false;
			if (a[6] < 0 || a[6] > 1000 || a[7] < 1 || a[7] > 1000 || a[8] < 0 || a[8] > 6 || a[10] < 0 ||
			    a[10] > 3 || a[11] < 0 || a[12] < 0 || a[11] + a[12] > 5)
				return false;
			for (int i : {13, 14, 15})
				if (a[i] < 0 || a[i] > 1)
					return false;
			if (a[14] && a[1] != 2)
				return false;
			Story s;
			s.chapter = a[0];
			s.phase = a[1];
			s.virtue = a[2];
			s.resolve = a[3];
			s.food = a[4];
			s.gold = a[5];
			s.preparation = a[13];
			s.planner = version >= 3 ? j[U"planner"].get<int>() : -1;
			if (s.planner < -1 || s.planner >= s.Unlocked())
				return false;
			if ((s.phase == 2 || s.phase == 3) && s.Tactical())
				s.StartMission();
			s.reputation = a[6];
			s.turn = a[7];
			s.tasks = a[8];
			s.order = a[9];
			s.visits = a[10];
			s.escaped = a[11];
			s.lost = a[12];
			s.failed = a[14] != 0;
			s.fireUsed = a[15] != 0;
			int n = 0;
			for (const auto& v : j[U"bonds"].arrayView())
			{
				if (n >= 6)
					return false;
				int b = v.get<int>();
				if (b < 0 || b > 100)
					return false;
				s.bonds[n++] = b;
			}
			if (n != 6)
				return false;
			for (const auto& v : j[U"decisions"].arrayView())
			{
				int d = v.get<int>();
				if (d < 0 || d / 10 > s.chapter || d % 10 > 5 || s.decisions.size() >= 18)
					return false;
				s.decisions.push_back(d);
			}
			s.journal.clear();
			for (const auto& v : j[U"journal"].arrayView())
			{
				auto t = v.get<s3d::String>();
				if (t.size() > 500 || s.journal.size() >= 100)
					return false;
				s.journal.emplace_back(t.c_str());
			}
			n = 0;
			std::array<bool, W * H> occupied{};
			for (const auto& item : j[U"units"].arrayView())
			{
				s3d::Array<int> v;
				for (const auto& x : item[U"values"].arrayView())
					v.push_back(x.get<int>());
				if (v.size() != 12 || n >= static_cast<int>(s.units.size()))
					return false;
				auto& u = s.units[n++];
				if (v[0] != u.hero || v[7] != static_cast<int>(u.enemy) ||
				    v[8] != static_cast<int>(u.civilian) || !Story::Valid(v[1], v[2]) || v[4] < 5 ||
				    v[4] > 20 || v[3] > v[4] || v[3] < 0 || v[5] < (u.civilian ? 0 : 1) || v[5] > 10 ||
				    v[6] < 1 || v[6] > 3 || v[9] < 0 || v[9] > 1 || v[10] < 0 || v[10] > 1 || v[11] < 0 ||
				    v[11] > 1)
					return false;
				int tile = v[2] * W + v[1];
				if (s.terrain[tile] == 2 || (v[3] > 0 && occupied[tile]))
					return false;
				if (v[3] > 0)
					occupied[tile] = true;
				u.x = v[1];
				u.y = v[2];
				u.hp = v[3];
				u.maxHp = v[4];
				u.attack = v[5];
				u.range = v[6];
				u.acted = v[9] != 0;
				u.skillUsed = v[10] != 0;
				u.stunned = v[11];
			}
			if (n != static_cast<int>(s.units.size()))
				return false;
			if (version >= 2)
			{
				s3d::Array<int> d;
				for (const auto& v : j[U"drama"].arrayView())
					d.push_back(v.get<int>());
				if (d.size() != 4 || d[0] < 0 || d[0] > 100 || d[1] < 0 || d[1] > 1 || d[2] < 0 || d[2] > 1 ||
				    d[3] < 0 || d[3] > 100 ||
				    ((d[1] != 0) && (s.phase != 2 || !s.Tactical() || s.failed || d[2] != 1)))
					return false;
				s.spirit = d[0];
				s.battleEvent = d[1];
				s.eventMask = d[2];
				s.rallies = d[3];
				for (const auto& v : j[U"battleChoices"].arrayView())
				{
					int x = v.get<int>();
					if (x < 0 || x / 10 > s.chapter || x % 10 > 1 || s.battleDecisions.size() >= 100)
						return false;
					s.battleDecisions.push_back(x);
				}
			}
			else
			{
				s.spirit = 20;
				s.battleEvent = 0;
				s.eventMask = s.turn >= 4 ? 1 : 0;
				s.rallies = 0;
			}
			if (version >= 3)
			{
				s.relationships.clear();
				std::array<bool, 36> seen{};
				for (const auto& r : j[U"relationships"].arrayView())
				{
					int x = r[U"a"].get<int>(), y = r[U"b"].get<int>(), v = r[U"value"].get<int>();
					if (x < 1 || y > 5 || x >= y || v < 0 || v > 100 || seen[x * 6 + y])
						return false;
					seen[x * 6 + y] = true;
					s.relationships.push_back({x, y, v});
				}
			}
			output = std::move(s);
			return true;
		}
		catch (...)
		{
			return false;
		}
	}
} // namespace hero
