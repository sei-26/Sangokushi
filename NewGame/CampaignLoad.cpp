#include "CampaignSave.hpp"

namespace frontline
{
	bool LoadJSON(const s3d::JSON& json, Campaign& game)
	{
		try
		{
			const int version = json[U"version"].get<int>();
			if (version < 1 || version > 11)
				return false;
			const int player = json[U"player"].get<int>(), day = json[U"day"].get<int>(),
			          commands = json[U"commands"].get<int>();
			if (player < 0 || player > 2 || day < 0 || day > 100000 || commands < 0 || commands > 3)
				return false;
			int savedTiles = 0;
			for (const auto& value : json[U"owners"].arrayView())
			{
				(void)value;
				if (++savedTiles > TileCount)
					return false;
			}
			const bool oldGrid = version < 4 && savedTiles == 36 * 22;
			if (savedTiles != TileCount && !oldGrid)
				return false;
			if (version >= 4 &&
			    (json[U"gridWidth"].get<int>() != Width || json[U"gridHeight"].get<int>() != Height))
				return false;
			Campaign loaded;
			if (oldGrid || (version >= 4 && json[U"legacyLayout"].get<bool>()))
				loaded.ResetLegacy(player);
			else
				loaded.Reset(player);
			loaded.hexMap = version >= 10 ? json[U"hexMap"].get<bool>() : false;
			if (loaded.hexMap && loaded.legacyLayout)
				return false;
			loaded.day = day;
			loaded.commands = commands;
			if (version >= 8)
			{
				int count = 0;
				for (const auto& v : json[U"aiPlannedDay"].arrayView())
				{
					int d = v.get<int>();
					if (count >= 3 || d < -1 || d > day)
						return false;
					loaded.aiPlannedDay[count++] = d;
				}
				if (count != 3)
					return false;
				count = 0;
				for (const auto& v : json[U"aiCommands"].arrayView())
				{
					int c = v.get<int>();
					if (count >= 3 || c < 0 || c > 3)
						return false;
					loaded.aiCommands[count++] = c;
				}
				if (count != 3)
					return false;
			}
			const auto tileIndex = [&](int p) {
				if (!oldGrid)
					return p;
				return p < 0 || p >= 36 * 22 ? -1 : Campaign::At(p % 36, p / 36);
			};
			int index = 0;
			for (const auto& value : json[U"owners"].arrayView())
			{
				if (index >= TileCount)
					return false;
				const int owner = value.get<int>();
				if (owner < -1 || owner > 2)
					return false;
				const int p = tileIndex(index++);
				if (loaded.tiles[p].terrain == Terrain::Sea && owner != -1)
					return false;
				loaded.tiles[p].owner = owner;
			}
			if (index != savedTiles)
				return false;
			index = 0;
			for (const auto& j : json[U"cities"].arrayView())
			{
				if (index >= static_cast<int>(loaded.cities.size()))
					return false;
				auto& c = loaded.cities[index++];
				c.owner = j[U"owner"].get<int>();
				c.troops = j[U"troops"].get<int>();
				c.food = j[U"food"].get<int>();
				c.gold = j[U"gold"].get<int>();
				if (c.owner < 0 || c.owner > 2 || c.troops < 0 || c.troops > 1000000 || c.food < 0 ||
				    c.food > 100000000 || c.gold < 0 || c.gold > 100000000 ||
				    loaded.tiles[c.tile].owner != c.owner)
					return false;
				if (version >= 2)
				{
					c.farming = j[U"farming"].get<int>();
					c.commerce = j[U"commerce"].get<int>();
					c.order = j[U"order"].get<int>();
					c.logistics = j[U"logistics"].get<int>();
					c.worker = j[U"worker"].get<int>();
					c.work = j[U"work"].get<int>();
					c.workLeft = j[U"workLeft"].get<int>();
					if (c.farming < 0 || c.farming > 100 || c.commerce < 0 || c.commerce > 100 ||
					    c.order < 0 || c.order > 100 || c.logistics < 0 || c.logistics > 100 ||
					    c.worker < -1 || c.worker >= static_cast<int>(loaded.generals.size()) || c.work < 0 ||
					    c.work > 3 || c.workLeft < 0 || c.workLeft > 30 ||
					    ((c.worker == -1) != (c.workLeft == 0)))
						return false;
					if (version >= 3)
					{
						c.helper = j[U"helper"].get<int>();
						if (c.helper < -1 || c.helper >= static_cast<int>(loaded.generals.size()) ||
						    (c.helper >= 0 && (c.worker < 0 || c.helper == c.worker)))
							return false;
					}
				}
			}
			if (index != static_cast<int>(loaded.cities.size()))
				return false;
			index = 0;
			for (const auto& j : json[U"generals"].arrayView())
			{
				if (index >= static_cast<int>(loaded.generals.size()))
					return false;
				auto& g = loaded.generals[index++];
				g.home = j[U"home"].get<int>();
				g.readyDay = j[U"readyDay"].get<int>();
				if (g.home < 0 || g.home >= static_cast<int>(loaded.cities.size()) || g.readyDay < 0 ||
				    g.readyDay > day + 100)
					return false;
			}
			if (index != (version == 1 ? 18 : static_cast<int>(loaded.generals.size())))
				return false;
			std::vector<bool> assigned(loaded.generals.size());
			for (int c = 0; c < static_cast<int>(loaded.cities.size()); ++c)
			{
				const int g = loaded.cities[c].worker;
				if (g < 0)
					continue;
				if (assigned[g] || loaded.generals[g].faction != loaded.cities[c].owner ||
				    loaded.generals[g].home != c || loaded.generals[g].readyDay > day)
					return false;
				assigned[g] = true;
				const int h = loaded.cities[c].helper;
				if (h >= 0)
				{
					if (assigned[h] || loaded.generals[h].faction != loaded.cities[c].owner ||
					    loaded.generals[h].home != c || loaded.generals[h].readyDay > day)
						return false;
					assigned[h] = true;
				}
			}
			if (version >= 3)
			{
				loaded.randomState = json[U"randomState"].get<unsigned>();
				if (loaded.randomState == 0)
					return false;
				index = 0;
				for (const auto& value : json[U"truces"].arrayView())
				{
					if (index >= 9)
						return false;
					const int until = value.get<int>();
					if (until < 0 || until > day + 60)
						return false;
					loaded.truceUntil[index / 3][index % 3] = until;
					++index;
				}
				if (index != 9)
					return false;
				index = 0;
				for (const auto& value : json[U"regard"].arrayView())
				{
					if (index >= 9)
						return false;
					const int regard = value.get<int>();
					if (regard < 0 || regard > 100)
						return false;
					loaded.regard[index / 3][index % 3] = regard;
					++index;
				}
				if (index != 9)
					return false;
				for (int f = 0; f < 3; ++f)
				{
					if (loaded.truceUntil[f][f] != 0)
						return false;
					for (int other = 0; other < 3; ++other)
						if (loaded.truceUntil[f][other] != loaded.truceUntil[other][f] ||
						    loaded.regard[f][other] != loaded.regard[other][f])
							return false;
				}
				loaded.bonds.clear();
				for (const auto& j : json[U"bonds"].arrayView())
				{
					Bond b{j[U"a"].get<int>(), j[U"b"].get<int>(), j[U"value"].get<int>()};
					if (loaded.bonds.size() >= 276 || b.a < 0 || b.b <= b.a ||
					    b.b >= static_cast<int>(loaded.generals.size()) || b.value < 0 || b.value > 100)
						return false;
					for (const auto& existing : loaded.bonds)
						if (existing.a == b.a && existing.b == b.b)
							return false;
					loaded.bonds.push_back(b);
				}
				for (const auto& j : json[U"missions"].arrayView())
				{
					Mission m;
					m.general = j[U"general"].get<int>();
					m.helper = j[U"helper"].get<int>();
					m.city = j[U"city"].get<int>();
					m.target = j[U"target"].get<int>();
					m.faction = j[U"faction"].get<int>();
					m.targetFaction = j[U"targetFaction"].get<int>();
					m.left = j[U"left"].get<int>();
					const int kind = j[U"kind"].get<int>();
					if (loaded.missions.size() >= loaded.generals.size() || m.general < 0 ||
					    m.general >= static_cast<int>(loaded.generals.size()) || m.helper < -1 ||
					    m.helper >= static_cast<int>(loaded.generals.size()) || m.helper == m.general ||
					    m.city < 0 || m.city >= static_cast<int>(loaded.cities.size()) || m.target < 0 ||
					    m.target >= static_cast<int>(loaded.cities.size()) || m.faction < 0 ||
					    m.faction > 2 || m.targetFaction < 0 || m.targetFaction > 2 ||
					    m.faction == m.targetFaction || kind < 0 || kind > 1 || m.left <= 0 ||
					    m.left > (kind == 0 ? 20 : 30))
						return false;
					m.kind = static_cast<MissionKind>(kind);
					if (loaded.cities[m.city].owner != m.faction ||
					    loaded.cities[m.target].owner != m.targetFaction ||
					    (m.kind == MissionKind::Sabotage && !loaded.Hostile(m.faction, m.targetFaction)))
						return false;
					for (int g : {m.general, m.helper})
						if (g >= 0)
						{
							if (assigned[g] || loaded.generals[g].home != m.city ||
							    loaded.generals[g].faction != m.faction || loaded.generals[g].readyDay > day)
								return false;
							assigned[g] = true;
						}
					loaded.missions.push_back(m);
				}
				loaded.chronicle.clear();
				int previous = -1;
				for (const auto& j : json[U"chronicle"].arrayView())
				{
					const int eventDay = j[U"day"].get<int>();
					const auto eventText = j[U"text"].get<s3d::String>();
					if (loaded.chronicle.size() >= 120 || eventDay < 0 || eventDay < previous ||
					    eventDay > day || eventText.isEmpty() || eventText.size() > 300)
						return false;
					loaded.chronicle.push_back({eventDay, eventText.toUTF32()});
					previous = eventDay;
				}
			}
			for (const auto& j : json[U"armies"].arrayView())
			{
				if (loaded.armies.size() >= 10000)
					return false;
				Army a;
				a.faction = j[U"faction"].get<int>();
				a.general = j[U"general"].get<int>();
				a.tile = j[U"tile"].get<int>();
				a.target = j[U"target"].get<int>();
				a.troops = j[U"troops"].get<int>();
				a.tile = tileIndex(a.tile);
				a.target = tileIndex(a.target);
				a.food = j[U"food"].get<int>();
				if (version >= 6)
					a.cargoFood = j[U"cargoFood"].get<int>();
				a.morale = j[U"morale"].get<int>();
				a.movement = j[U"movement"].get<int>();
				if (version >= 9)
				{
					a.aiAssemblyDays = j[U"aiAssemblyDays"].get<int>();
					if (a.aiAssemblyDays < 0 || a.aiAssemblyDays > 20)
						return false;
				}
				const int arm = j[U"arm"].get<int>();
				a.supplied = j[U"supplied"].get<bool>();
				a.retreat = j[U"retreat"].get<bool>();
				if (a.faction < 0 || a.faction > 2 || a.general < 0 ||
				    a.general >= static_cast<int>(loaded.generals.size()) || !Campaign::Valid(a.tile) ||
				    !Campaign::Valid(a.target) || a.troops < 0 || a.troops > 1000000 || a.food < 0 ||
				    a.food > 1000000 || a.morale < 0 || a.morale > 100 || a.movement < 0 || a.movement > 12 ||
				    arm < 0 || arm > (version >= 6 ? 4 : 3) || a.cargoFood < 0 || a.cargoFood > 20000 ||
				    (arm != 4 && a.cargoFood != 0) || (a.troops == 0 && a.cargoFood != 0))
					return false;
				if (version == 1 && a.general >= 18)
					return false;
				a.arm = static_cast<Arm>(arm);
				if (version >= 7)
				{
					const int stance = j[U"stance"].get<int>();
					if (stance < 0 || stance > 2 || (a.arm == Arm::Transport && stance != 0))
						return false;
					a.stance = static_cast<battle::Stance>(stance);
				}
				if (version >= 3)
				{
					a.tacticLeft = j[U"tacticLeft"].get<int>();
					a.tacticReadyDay = j[U"tacticReadyDay"].get<int>();
					a.tacticQueued = j[U"tacticQueued"].get<bool>();
					if (a.tacticLeft < 0 || a.tacticLeft > 5 || a.tacticReadyDay < 0 ||
					    a.tacticReadyDay > day + 30 ||
					    (a.tacticQueued && (a.troops <= 0 || a.tacticReadyDay > day || a.morale < 30)))
						return false;
				}
				if (a.arm == Arm::Transport && (a.tacticQueued || a.tacticLeft != 0 ||
				                                (a.target != a.tile && loaded.CityAt(a.target) < 0)))
					return false;
				if (loaded.Cost(a.tile, a.arm) >= 100000 || loaded.Cost(a.target, a.arm) >= 100000 ||
				    a.faction != loaded.generals[a.general].faction)
					return false;
				if (a.troops > 0)
				{
					if (assigned[a.general] || loaded.ArmyCount(a.faction) >= Campaign::MaxArmies)
						return false;
					assigned[a.general] = true;
				}
				loaded.armies.push_back(a);
				if (a.troops > 0 &&
				    !loaded.Order(static_cast<int>(loaded.armies.size()) - 1, a.target, a.retreat) &&
				    a.arm != Arm::Transport)
					return false;
			}
			if (version >= 5)
				for (const auto& j : json[U"assignments"].arrayView())
				{
					Assignment move{j[U"general"].get<int>(), j[U"from"].get<int>(), j[U"to"].get<int>(),
					                j[U"faction"].get<int>(), j[U"left"].get<int>()};
					if (loaded.assignments.size() >= loaded.generals.size() || move.general < 0 ||
					    move.general >= static_cast<int>(loaded.generals.size()) || move.from < 0 ||
					    move.to < 0 || move.from >= static_cast<int>(loaded.cities.size()) ||
					    move.to >= static_cast<int>(loaded.cities.size()) || move.from == move.to ||
					    move.faction < 0 || move.faction > 2 || move.left < 1 || move.left > 180)
						return false;
					const auto& officer = loaded.generals[move.general];
					if (assigned[move.general] || officer.faction != move.faction ||
					    officer.home != move.from || officer.readyDay > day ||
					    loaded.cities[move.from].owner != move.faction ||
					    loaded.cities[move.to].owner != move.faction)
						return false;
					assigned[move.general] = true;
					loaded.assignments.push_back(move);
				}
			if (version >= 11)
			{
				size_t count = 0;
				for (const auto& j : json[U"regions"].arrayView())
				{
					if (!loaded.hexMap || count >= loaded.regions.size())
						return false;
					auto& r = loaded.regions[count++];
					r.owner = j[U"owner"].get<int>();
					r.city = j[U"city"].get<int>();
					if (r.owner < 0 || r.owner > 2 || r.city < -1 ||
					    r.city >= static_cast<int>(loaded.cities.size()))
						return false;
					if (r.city >= 0 && loaded.cities[r.city].owner != r.owner)
						return false;
					const int center = loaded.CityAt(r.tile);
					if (center >= 0 && (r.city != center || r.owner != loaded.cities[center].owner))
						return false;
				}
				if (count == 0)
				{
					loaded.regions.clear();
					loaded.tileRegion.fill(-1);
				}
				else if (count != loaded.regions.size())
					return false;
			}
			else
			{
				loaded.regions.clear();
				loaded.tileRegion.fill(-1);
			}
			bool all = true, any = false;
			for (const auto& c : loaded.cities)
			{
				all = all && c.owner == player;
				any = any || c.owner == player;
			}
			loaded.result = all ? 1 : !any && loaded.ArmyCount(player) == 0 ? 2 : 0;
			loaded.log.clear();
			loaded.log.push_back(U"戦況を復元しました。進路と補給を確認してください。");
			++loaded.revision;
			game = std::move(loaded);
			return true;
		}
		catch (...)
		{
			return false;
		}
	}
} // namespace frontline
