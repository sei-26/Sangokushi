#include "BattleSystem.hpp"

BattleSystem::BattleSystem(const BattleSetup& s)
	: setup(s)
{
	// 出陣兵力を都市から減らす
	if (setup.attackerCity)
	{
		setup.attackerSoldiers =
			Min(setup.attackerSoldiers, setup.attackerCity->troops);
		setup.attackerCity->troops -= setup.attackerSoldiers;
	}

	if (setup.defenderCity)
	{
		setup.defenderSoldiers =
			Min(setup.defenderSoldiers, setup.defenderCity->troops);
		setup.defenderCity->troops -= setup.defenderSoldiers;
	}

	// 兵力はそのまま（都市兵力10000など）
	battle.InitializeBattle(
		*setup.attackerLeader,
		*setup.defenderLeader,
		setup.attackerSoldiers,
		setup.defenderSoldiers
	);
}

void BattleSystem::Update()
{
	if (finished) return;

	battle.UpdatePKBattle();

	if (battle.IsBattleFinished())
		finished = true;
}

void BattleSystem::ApplyResultToCities()
{
	if (!finished) return;

	const auto& units = battle.GetUnits();

	// 0 = 攻撃側, 1 = 防衛側
	int atkRemain = (units[0].alive ? units[0].soldiers : 0);
	int defRemain = (units[1].alive ? units[1].soldiers : 0);

	// 都市へ兵力を戻す
	if (setup.attackerCity)
		setup.attackerCity->troops += atkRemain;

	if (setup.defenderCity)
		setup.defenderCity->troops += defRemain;

	// 勝利側が都市を占領
	if (!units[1].alive)
		setup.defenderCity->owner = setup.attackerCity->owner;
}
