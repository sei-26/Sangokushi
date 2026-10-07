#include "StorySave.hpp"

namespace hero
{
	s3d::JSON SaveJSON(const Story& s)
	{
		s3d::JSON j;
		j[U"version"] = 3;
		j[U"planner"] = s.planner;
		s3d::Array<s3d::JSON> relations;
		for (const auto& r : s.relationships)
		{
			s3d::JSON b;
			b[U"a"] = r.a;
			b[U"b"] = r.b;
			b[U"value"] = r.value;
			relations.push_back(b);
		}
		j[U"relationships"] = relations;
		j[U"drama"] = s3d::Array<int>{s.spirit, s.battleEvent, s.eventMask, s.rallies};
		s3d::Array<int> battleChoices;
		for (auto v : s.battleDecisions)
			battleChoices.push_back(v);
		j[U"battleChoices"] = battleChoices;
		const s3d::Array<int> state{s.chapter,
		                            s.phase,
		                            s.virtue,
		                            s.resolve,
		                            s.food,
		                            s.gold,
		                            s.reputation,
		                            s.turn,
		                            s.tasks,
		                            s.order,
		                            s.visits,
		                            s.escaped,
		                            s.lost,
		                            s.preparation,
		                            static_cast<int>(s.failed),
		                            static_cast<int>(s.fireUsed)};
		j[U"state"] = state;
		s3d::Array<int> bonds, decisions;
		for (auto v : s.bonds)
			bonds.push_back(v);
		for (auto v : s.decisions)
			decisions.push_back(v);
		j[U"bonds"] = bonds;
		j[U"decisions"] = decisions;
		s3d::Array<s3d::String> journal;
		for (const auto& v : s.journal)
			journal.push_back(s3d::String(v.c_str()));
		j[U"journal"] = journal;
		s3d::Array<s3d::JSON> units;
		for (const auto& u : s.units)
		{
			s3d::JSON a;
			a[U"values"] = s3d::Array<int>{u.hero,
			                               u.x,
			                               u.y,
			                               u.hp,
			                               u.maxHp,
			                               u.attack,
			                               u.range,
			                               static_cast<int>(u.enemy),
			                               static_cast<int>(u.civilian),
			                               static_cast<int>(u.acted),
			                               static_cast<int>(u.skillUsed),
			                               u.stunned};
			units.push_back(a);
		}
		j[U"units"] = units;
		return j;
	}
} // namespace hero
