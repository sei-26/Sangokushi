#include "StrategistBattle.hpp"

namespace frontline
{
	bool StrategistBattle::OnMap(int tile)
	{
		return Campaign::Valid(tile) && tile % Width < MapColumns && tile / Width < MapRows;
	}
	const char32_t* StrategistBattle::PlanName(int choice)
	{
		const char32_t* names[] = {U"渡河点を確保", U"北側から牽制", U"予備隊で護送"};
		return names[std::clamp(choice, 0, 2)];
	}
	const char32_t* StrategistBattle::PlanDescription(int choice)
	{
		const char32_t* descriptions[] = {U"前衛と別働隊を渡河点へ。兵糧の通り道を押さえる。",
		                                  U"別働隊を北の橋へ送り、敵を分散させる。護送路の兵力は薄くなる。",
		                                  U"主力を護送路に集中。前衛が押されたら予備隊で支える。"};
		return descriptions[std::clamp(choice, 0, 2)];
	}
	const char32_t* StrategistBattle::RoleName(int army)
	{
		const char32_t* roles[] = {U"前衛", U"別働", U"予備", U"前線護衛", U"兵糧輸送"};
		return army >= 0 && army < 5 ? roles[army] : U"敵部隊";
	}
	const char32_t* StrategistBattle::EnemyPolicyName(int choice)
	{
		const char32_t* names[] = {U"渡河点への攻勢", U"予備を残して警戒", U"前線拠点を襲撃"};
		return names[std::clamp(choice, 0, 2)];
	}
	void StrategistBattle::Reset()
	{
		world = Campaign{};
		world.hexMap = true;
		world.tileRegion.fill(-1);
		for (auto& tile : world.tiles)
			tile = {Terrain::Sea, -1};
		for (int y = 1; y < MapRows - 1; ++y)
			for (int x = 1; x < MapColumns - 1; ++x)
			{
				auto& tile = world.tiles[Campaign::At(x, y)];
				tile = {Terrain::Plain, x <= 10 || y >= 11 ? 0 : 1};
				if (x >= 6 && x <= 16 && y >= 4 && y <= 7)
					tile.terrain = Terrain::Forest;
				if (x == 11)
					tile.terrain = y == 4 || y == 12 ? Terrain::Bridge : Terrain::River;
			}
		world.cities = {{U"後方兵站", Campaign::At(2, 12), 0, 3000, 80000, 3000},
		                {U"前線拠点", Campaign::At(21, 12), 0, 1000, 0, 0},
		                {U"敵の本陣", Campaign::At(21, 4), 1, 4000, 120000, 3000}};
		world.cities[0].logistics = 80;
		world.cities[2].logistics = 80;
		const char32_t* names[] = {U"関羽", U"趙雲",   U"張飛", U"黄忠", U"糜竺",
		                           U"曹仁", U"夏侯惇", U"徐晃", U"張遼"};
		const int xs[] = {8, 7, 5, 17, 3, 16, 18, 19, 18};
		const int ys[] = {12, 6, 11, 12, 12, 10, 8, 14, 4};
		for (int i = 0; i < 9; ++i)
		{
			General general;
			general.name = names[i];
			general.faction = i < 5 ? 0 : 1;
			general.home = i < 5 ? 0 : 2;
			general.leadership = i == 4 ? 60 : 85;
			general.trait = i == 0 ? Trait::Guardian : i == 4 ? Trait::Quartermaster : Trait::Valiant;
			general.tactic = i == 0   ? Tactic::Fortify
			                 : i == 3 ? Tactic::Volley
			                 : i == 4 ? Tactic::Supply
			                          : Tactic::Charge;
			world.generals.push_back(general);
			Army army;
			army.faction = general.faction;
			army.general = i;
			army.tile = army.target = Campaign::At(xs[i], ys[i]);
			army.troops = i == 4 ? 1000 : i < 5 ? 3000 : 3500;
			army.food = 4000;
			army.arm = i == 4             ? Arm::Transport
			           : i == 3 || i == 6 ? Arm::Bow
			           : i == 1 || i == 8 ? Arm::Cavalry
			                              : Arm::Spear;
			if (i == 4)
				army.cargoFood = 5000;
			world.armies.push_back(army);
		}
		plan = enemyPolicy = deliveredCargo = evacuatedSoldiers = outcome = 0;
		enemyPolicy = 1; // Begin with the cautious opponent; other policies are selectable.
		prepared = false;
		intelligenceEnabled = true;
		operations = {};
		arrivals = {};
		reports.clear();
		disposition.fill(0);
		reason.clear();
		for (auto& known : intelligence)
			known.Reset(world);
		RefreshIntelligence();
		Report(U"創作の検証戦役。敵情は目撃した情報で判断し、兵糧を届けよ。");
		++world.revision;
	}
	bool StrategistBattle::SetPlan(int choice)
	{
		if (world.day != 0 || outcome != 0 || choice < 0 || choice > 2)
			return false;
		const int targets[3][4] = {
		    {Campaign::At(11, 12), Campaign::At(14, 11), Campaign::At(10, 13), Campaign::At(19, 12)},
		    {Campaign::At(10, 9), Campaign::At(12, 4), Campaign::At(10, 12), Campaign::At(19, 12)},
		    {Campaign::At(11, 12), Campaign::At(9, 13), Campaign::At(15, 12), Campaign::At(19, 12)}};
		plan = choice;
		for (int i = 0; i < 4; ++i)
		{
			world.Order(i, targets[choice][i]);
			world.armies[i].stance = i == 0 || i == 3 ? battle::Stance::Guard : battle::Stance::Balanced;
		}
		// Keep the convoy at its start until the player orders departure.
		world.Order(Convoy, world.armies[Convoy].tile);
		prepared = true;
		operations = {};
		for (int i = 0; i <= Convoy; ++i)
			world.armies[i].scouting = false;
		RecordArrivals();
		Report(std::u32string(U"軍議で方針を決定：") + PlanName(choice));
		if (choice == 1)
		{
			BattleOperation scout;
			scout.target = targets[choice][1];
			scout.scout = true;
			QueueOperation(1, scout);
		}
		if (choice == 2)
		{
			BattleOperation reserve;
			reserve.target = targets[choice][2];
			reserve.trigger = OperationTrigger::AllyArrival;
			reserve.signalArmy = 0;
			reserve.signalTile = targets[choice][0];
			reserve.minTroops = 1500;
			reserve.minMorale = 35;
			reserve.fallback = world.cities[0].tile;
			QueueOperation(2, reserve);
		}
		return true;
	}
	bool StrategistBattle::Order(int army, int tile, bool retreat)
	{
		if (!prepared || outcome || army < 0 || army > Convoy || !OnMap(tile) ||
		    world.armies[army].troops <= 0 || world.tiles[tile].terrain == Terrain::Sea ||
		    world.day % 10 != 0)
			return false;
		const auto path = KnownRoute(army, tile);
		if (tile != world.armies[army].tile && path.empty())
			return false;
		if (army == Convoy && tile != world.armies[army].tile)
		{
			const int city = world.CityAt(tile);
			if (city < 0 || intelligence[0].owners[tile] != world.player)
				return false;
		}
		world.armies[army].target = tile;
		world.armies[army].path = path;
		world.armies[army].retreat = retreat;
		world.armies[army].scouting = false;
		operations[army] = {};
		arrivals[army] = {};
		RecordArrivals();
		Report(world.generals[world.armies[army].general].name +
		       (retreat ? U"隊へ後退命令。" : U"隊の進路を変更。"));
		return true;
	}
	int StrategistBattle::RemainingSoldiers() const
	{
		int total = evacuatedSoldiers;
		for (int i = 0; i < 4; ++i)
			total += world.armies[i].troops;
		return total;
	}
	bool StrategistBattle::SetStance(int army, battle::Stance stance)
	{
		if (!prepared || outcome || army < 0 || army >= Convoy || world.day % 10 != 0)
			return false;
		if (world.armies[army].scouting && stance != battle::Stance::Guard)
			return false;
		return world.SetStance(army, stance, true);
	}
	int StrategistBattle::Cargo() const
	{
		return deliveredCargo > 0 ? deliveredCargo : world.armies[Convoy].cargoFood;
	}
	void StrategistBattle::EnemyOrders()
	{
		// A visible staging period gives the player time to prepare an escort.
		if (world.day < EnemyAssemblyDays)
			return;
		for (int i = 5; i < 9; ++i)
		{
			auto& army = world.armies[i];
			if (army.troops <= 0)
				continue;
			if (army.troops < 900 || army.morale < 30)
			{
				world.Order(i, world.cities[2].tile, true);
				continue;
			}
			world.Order(i, EnemyTarget(i));
		}
	}
	void StrategistBattle::Evaluate()
	{
		if (deliveredCargo >= RequiredCargo && RemainingSoldiers() >= RequiredSoldiers &&
		    world.cities[Destination].owner == world.player)
		{
			outcome = 1;
			reason = U"兵糧の配送と兵の温存を達成。任された護送作戦を完遂した。";
		}
		else if (world.cities[Destination].owner != world.player)
		{
			outcome = 2;
			reason = U"前線拠点が陥落。護衛と渡河点の配分を見直そう。";
		}
		else if (RemainingSoldiers() < RequiredSoldiers)
		{
			outcome = 2;
			reason = U"温存する兵が不足。救援や後退の時期を見直そう。";
		}
		else if (world.armies[Convoy].troops == 0 && deliveredCargo < RequiredCargo)
		{
			outcome = 2;
			reason = U"必要な積荷を届けられなかった。輸送隊の護衛と出発時期を見直そう。";
		}
		else if (world.day >= Deadline)
		{
			outcome = 2;
			reason = U"配送期限を超過。渡河点の確保と出発時期を見直そう。";
		}
		if (outcome)
			Report(reason);
	}
	void StrategistBattle::AdvanceDay()
	{
		if (!prepared || outcome)
			return;
		RefreshIntelligence();
		std::array<Army, 5> before;
		for (int i = 0; i <= Convoy; ++i)
			before[i] = world.armies[i];
		const auto previousContacts = intelligence[0].contacts;
		if (world.day == EnemyAssemblyDays)
			Report(U"敵の集結が終わる頃だ。進軍は偵察と報告で確認しよう。");
		if (world.day % 5 == 0)
			EnemyOrders();
		ResolveOperations();
		if (world.day % 5 == 0 && world.armies[Convoy].troops > 0 &&
		    world.armies[Convoy].tile != world.armies[Convoy].target)
			world.armies[Convoy].path = KnownRoute(Convoy, world.armies[Convoy].target);
		world.AdvanceBattleDay();
		for (const auto& returned : world.returnEvents)
		{
			if (returned.general == world.armies[Convoy].general && returned.city == Destination)
				deliveredCargo = returned.cargoFood;
			else if (returned.general >= 0 && returned.general < 4)
				evacuatedSoldiers += returned.troops;
			if (returned.general >= 0 && returned.general <= Convoy)
			{
				disposition[returned.general] = 1;
				world.armies[returned.general].scouting = false;
				Report(world.generals[returned.general].name + U"隊が入城。実際の生存兵と積荷を収容した。");
			}
		}
		for (int i = 0; i <= Convoy; ++i)
			if (world.armies[i].troops <= 0 && disposition[i] == 0)
			{
				disposition[i] = 2;
				world.armies[i].scouting = false;
				if (operations[i].state != OperationState::None)
				{
					operations[i].state = OperationState::Failed;
					operations[i].status = U"部隊が敗走し、作戦を実行できない";
				}
			}
		RecordArrivals();
		RefreshIntelligence();
		for (const auto& contact : intelligence[0].contacts)
			if (contact.current &&
			    std::none_of(previousContacts.begin(), previousContacts.end(),
			                 [&](const ArmyContact& old) { return old.id == contact.id && old.current; }))
				Report(U"偵察報告：" + world.generals[contact.observed.general].name + U"隊を確認した。");
		for (const auto& old : previousContacts)
		{
			const auto* now = intelligence[0].Find(old.id);
			if (old.current && (!now || !now->current))
				Report(world.generals[old.observed.general].name + U"隊を見失った。目撃位置は過去の情報。");
		}
		for (int i = 0; i <= Convoy; ++i)
		{
			const auto& unit = world.armies[i];
			if (disposition[i] == 1 || before[i].troops <= 0)
				continue;
			if (unit.troops < before[i].troops)
				Report(world.generals[unit.general].name + U"隊から損耗の報告。救援と後退を検討せよ。");
			if (unit.supplied != before[i].supplied)
				Report(world.generals[unit.general].name +
				       (unit.supplied ? U"隊の補給が復旧。" : U"隊の補給が途絶した。"));
		}
		Evaluate();
	}
} // namespace frontline
