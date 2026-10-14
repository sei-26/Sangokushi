#include "StrategistBattle.hpp"

namespace frontline
{
	const char32_t* StrategistBattle::TriggerName(OperationTrigger trigger)
	{
		const char32_t* names[] = {U"即時", U"指定日", U"味方の到着", U"敵の確認"};
		return names[std::clamp(static_cast<int>(trigger), 0, 3)];
	}
	const char32_t* StrategistBattle::StateName(OperationState state)
	{
		const char32_t* names[] = {U"直接命令", U"条件待ち", U"実行中", U"配置完了", U"中止", U"実行不能"};
		return names[std::clamp(static_cast<int>(state), 0, 5)];
	}
	bool StrategistBattle::QueueOperation(int army, BattleOperation operation)
	{
		if (!prepared || outcome || world.day % 10 != 0 || army < 0 || army > Convoy ||
		    world.armies[army].troops <= 0 || !OnMap(operation.target) ||
		    world.Cost(operation.target, world.armies[army].arm) >= 100000 ||
		    operation.trigger < OperationTrigger::Immediate ||
		    operation.trigger > OperationTrigger::EnemySighted || operation.startDay < 1 ||
		    operation.startDay > Deadline || operation.minTroops < 0 || operation.minMorale < 0 ||
		    operation.minMorale > 100 || operation.deadline < 0 || operation.deadline > Deadline ||
		    (operation.fallback != -1 &&
		     (!OnMap(operation.fallback) ||
		      world.Cost(operation.fallback, world.armies[army].arm) >= 100000)) ||
		    (operation.scout && (army == Convoy || operation.retreat)))
			return false;
		if (army == Convoy && operation.target != world.armies[army].tile)
		{
			const int city = world.CityAt(operation.target);
			if (city < 0 || intelligence[0].owners[operation.target] != world.player)
				return false;
		}
		if (operation.trigger == OperationTrigger::EnemySighted && !OnMap(operation.watchTile))
			return false;
		if (operation.trigger == OperationTrigger::AllyArrival)
		{
			if (operation.signalArmy < 0 || operation.signalArmy > Convoy || operation.signalArmy == army ||
			    !OnMap(operation.signalTile))
				return false;
			int source = operation.signalArmy;
			std::array<bool, 5> visited{};
			while (source >= 0 && source <= Convoy && !visited[source])
			{
				if (source == army)
					return false; // Avoid a cycle of mutually waiting orders.
				visited[source] = true;
				const auto& next = operations[source];
				if (next.state == OperationState::None || next.trigger != OperationTrigger::AllyArrival)
					break;
				source = next.signalArmy;
			}
		}
		operation.state = OperationState::Waiting;
		operation.issuedDay = world.day;
		operation.status = U"開始条件を確認中";
		operations[army] = operation;
		arrivals[army] = {};
		world.Order(army, world.armies[army].tile);
		world.armies[army].scouting = false;
		Report(world.generals[world.armies[army].general].name + U"隊の作戦を予約：" +
		       TriggerName(operation.trigger));
		return true;
	}
	void StrategistBattle::ResolveOperations()
	{
		// Every decision is taken before applying any order. A task started now
		// cannot emit a same-day arrival signal for another task.
		std::array<int, 5> decisions{}; // 0 keep, 1 start, 2 abort
		std::array<std::u32string, 5> reasons;
		const int nextDay = world.day + 1;
		for (int i = 0; i <= Convoy; ++i)
		{
			const auto& task = operations[i];
			const auto& unit = world.armies[i];
			if (task.state == OperationState::None || task.state == OperationState::Aborted ||
			    task.state == OperationState::Failed)
				continue;
			if (unit.troops <= 0)
				continue;
			if ((task.minTroops > 0 && unit.troops <= task.minTroops) ||
			    (task.minMorale > 0 && unit.morale <= task.minMorale) ||
			    (task.deadline > 0 && nextDay > task.deadline))
			{
				decisions[i] = 2;
				reasons[i] = task.minTroops > 0 && unit.troops <= task.minTroops   ? U"兵数が中止条件に到達"
				             : task.minMorale > 0 && unit.morale <= task.minMorale ? U"士気が中止条件に到達"
				                                                                   : U"任務期限を超過";
				continue;
			}
			if (task.state != OperationState::Waiting)
				continue;
			bool ready = false;
			switch (task.trigger)
			{
			case OperationTrigger::Immediate:
				ready = true;
				break;
			case OperationTrigger::Date:
				ready = nextDay >= task.startDay;
				reasons[i] = U"指定日を待っている";
				break;
			case OperationTrigger::AllyArrival:
				if (world.armies[task.signalArmy].troops <= 0 && disposition[task.signalArmy] != 1)
				{
					decisions[i] = 2;
					reasons[i] = U"合図を送る部隊が敗走";
					break;
				}
				ready = arrivals[task.signalArmy].tile == task.signalTile &&
				        arrivals[task.signalArmy].day <= world.day &&
				        (arrivals[task.signalArmy].day >= task.issuedDay ||
				         world.armies[task.signalArmy].tile == task.signalTile);
				if (!ready)
				{
					const auto& source = operations[task.signalArmy];
					const int expected = source.state == OperationState::Waiting ||
					                             source.state == OperationState::Executing ||
					                             source.state == OperationState::Holding
					                         ? source.target
					                         : world.armies[task.signalArmy].target;
					if (expected != task.signalTile)
					{
						decisions[i] = 2;
						reasons[i] = U"合図元の目的地が変更された";
						break;
					}
				}
				reasons[i] = U"指定した味方の到着合図を待っている";
				break;
			case OperationTrigger::EnemySighted:
				ready = intelligence[0].EnemyNear(world, task.watchTile, 2);
				reasons[i] = U"監視地点の周囲で敵を確認するまで待機";
				break;
			}
			if (ready)
			{
				decisions[i] = 1;
				reasons[i] = U"開始条件が成立";
			}
		}
		for (int i = 0; i <= Convoy; ++i)
		{
			auto& task = operations[i];
			if (task.state == OperationState::None || world.armies[i].troops <= 0)
				continue;
			if (decisions[i] == 0)
			{
				if (task.state == OperationState::Waiting)
					task.status = reasons[i];
				continue;
			}
			world.armies[i].scouting = false;
			const int target = decisions[i] == 1 ? task.target : task.fallback;
			const int destination = target < 0 ? world.armies[i].tile : target;
			auto path = KnownRoute(i, destination);
			const bool reachable = destination == world.armies[i].tile || !path.empty();
			if (reachable)
			{
				auto& unit = world.armies[i];
				unit.target = destination;
				unit.path = std::move(path);
				unit.retreat = decisions[i] == 2 && target >= 0 ? true : task.retreat;
				unit.scouting = decisions[i] == 1 && task.scout;
				if (unit.scouting)
					unit.stance = battle::Stance::Guard;
				task.state = decisions[i] == 1 ? OperationState::Executing : OperationState::Aborted;
				task.status = reasons[i] + (decisions[i] == 2 ? (target >= 0 ? U"・後退" : U"・待機") : U"");
			}
			else
			{
				world.Order(i, world.armies[i].tile);
				task.state = OperationState::Failed;
				task.status = U"把握している情報では経路を確保できず、現在地で待機";
				if (decisions[i] == 1 && task.fallback >= 0)
				{
					auto fallbackPath = KnownRoute(i, task.fallback);
					if (task.fallback == world.armies[i].tile || !fallbackPath.empty())
					{
						world.armies[i].target = task.fallback;
						world.armies[i].path = std::move(fallbackPath);
						world.armies[i].retreat = true;
						task.state = OperationState::Aborted;
						task.status = U"目的地への経路がなく、代替地点へ後退";
					}
				}
			}
			Report(world.generals[world.armies[i].general].name + U"隊：" + task.status);
		}
	}
	void StrategistBattle::RecordArrivals()
	{
		for (int i = 0; i <= Convoy; ++i)
		{
			const auto& unit = world.armies[i];
			auto& task = operations[i];
			if (unit.troops <= 0 && disposition[i] != 1)
				continue;
			const int target = task.state == OperationState::None ? unit.target : task.target;
			if (unit.tile != target || task.state == OperationState::Waiting ||
			    task.state == OperationState::Aborted || task.state == OperationState::Failed)
				continue;
			if (arrivals[i].tile != target || arrivals[i].day < 0)
			{
				arrivals[i] = {target, world.day};
				Report(world.generals[unit.general].name + U"隊が指定地点へ到着。合図を送った。");
			}
			if (task.state == OperationState::Executing)
			{
				task.state = OperationState::Holding;
				task.status = disposition[i] == 1 ? U"入城完了・到着合図を送信"
				              : task.scout        ? U"配置完了・偵察を継続"
				                                  : U"配置完了・中止条件を監視";
			}
		}
	}
} // namespace frontline
