#include "StrategistJourney.hpp"

namespace frontline
{
	namespace
	{
		std::u32string Number(int value)
		{
			const auto text = std::to_string(value);
			return std::u32string(text.begin(), text.end());
		}
	} // namespace
	bool StrategistJourney::SelectPromise(JourneyPromise value)
	{
		if (stage != JourneyStage::Preparation || mission != 0 ||
		    (value != JourneyPromise::OpenRoad && value != JourneyPromise::EscortRefugees)) return false;
		promise = value;
        if (!IsRescue()) convoyRoute=JourneyConvoyRoute::Road;
		message = IsRescue() ? U"避難車まで送り届けると約束した。護衛600と、その兵糧900を出陣費用に加える。"
		                    : U"任務は退路を開くこと。先鋒を崩すか出口を確保し、民の通れる道を残す。";
		return true;
	}
	bool StrategistJourney::Withdraw()
	{
		if (stage != JourneyStage::Battle) return false;
		withdrawn = true;
		Finish(false, U"深追いを止め、任務を中止して撤収した。残った兵と携行糧を回収。約束は未達だが、失った兵を増やさず作戦を見直す。");
		return true;
	}
	void StrategistJourney::SetupBattlefield()
	{
		for (auto& tile : world.tiles)
			tile = {Terrain::Sea, -1};
		for (int y = 1; y < Rows - 1; ++y)
			for (int x = 1; x < Columns - 1; ++x)
			{
				auto& tile = world.tiles[Campaign::At(x, y)];
				tile = {Terrain::Plain, x <= 9 ? 0 : 1};
				if (y >= 5 && y <= 9 && x >= 5 && x <= 10)
					tile.terrain = Terrain::Forest;
                if (IsRescue() && y==12 && x>=7 && x<=10) tile.terrain=Terrain::Forest;
				if (y == 2 && x >= 8)
					tile.terrain = Terrain::Mountain;
				if (mission == 1 && x == 11 && y < 10)
					tile.terrain = y == 6 ? Terrain::Bridge : Terrain::River;
				if (mission == 2 && x == 10 && y >= 4 && y <= 8)
					tile.terrain = Terrain::Mountain;
			}
	}
	bool StrategistJourney::Hold(int army)
	{
		if (stage != JourneyStage::Battle || army < 0 || army >= 3 || !TacticChain::Living(world, army) ||
		    chain.spent[army])
			return false;
		auto& unit = world.armies[army];
		chain.spent[army] = chain.moved[army] = guarding[army] = true;
		chain.concealed[army] = false;
		unit.stance = battle::Stance::Guard;
		++world.revision;
		Report(world.generals[army].name + U"隊が固守。今の攻撃を使い、次の交戦で被害を抑える。");
		RefreshIntelligence();
		return true;
	}
	bool StrategistJourney::PositionSecured(int tile) const
	{
		bool occupied = false;
		bool braced = false;
		for (int i = 0; i < 3; ++i)
			if (world.armies[i].tile == tile && world.armies[i].troops >= 150 && world.armies[i].morale >= 25)
			{
				occupied = true;
				braced = guarding[i] && world.armies[i].troops >= 500 && world.armies[i].morale >= 40;
			}
		if (!occupied)
			return false;
		int adjacentEnemies = 0;
		for (const auto& army : world.armies)
			if (army.troops > 0 && world.Hostile(world.player, army.faction) &&
			    world.MapDistance(army.tile, tile) <= 1)
				++adjacentEnemies;
		return adjacentEnemies == 0 ||
		       (Definition().kind == JourneyObjective::HoldPositions && braced && adjacentEnemies == 1);
	}
	std::u32string StrategistJourney::ObjectiveProgress() const
	{
		if (IsRescue())
		{
			if (stage != JourneyStage::Battle) return history[0].rescued ? U"季安の避難車を送り届けた" : U"避難車の護送は未達";
			return std::u32string(ConvoyPlan(convoyRoute).name)+U" / 季安の避難車 " + Number(convoyStep) + U" / "+Number(static_cast<int>(ConvoyPath().size())-1)+U"区間・護衛兵 " + Number(world.armies[ConvoyArmy].troops) + U"。敵将を倒しても護送は続く";
		}
		if (Definition().kind == JourneyObjective::Escort)
		{
			if (stage != JourneyStage::Battle)
				return U"回収した積荷 " + Number(recoveredCargo);
			return U"糧車 " + Number(convoyStep) + U" / 6区間・積荷 " +
			       Number(world.armies[ConvoyArmy].cargoFood) + U" / 1,200";
		}
		if (Definition().kind == JourneyObjective::HoldPositions)
			return U"二拠点の維持 " + Number(heldTurns) + U" / 3指揮・Gで固守：隣接敵１隊まで維持";
		return U"先鋒の兵450以下・士気30以下、または出口を確保";
	}
	void StrategistJourney::AdvanceObjective()
	{
		if (Definition().kind == JourneyObjective::HoldPositions)
		{
			const int before = heldTurns;
			heldTurns = PositionSecured(goal) && PositionSecured(Definition().secondGoal) ? heldTurns + 1 : 0;
			if (before != heldTurns)
				Report(heldTurns > 0 ? U"二つの烽火を維持した。連続して守り切ろう。"
				                     : U"烽火の守りが崩れた。両拠点を取り戻すまで維持は数え直し。");
		}
		if (!HasConvoy() || !TacticChain::Living(world, ConvoyArmy) ||
		    convoyStep+1 >= static_cast<int>(ConvoyPath().size()))
			return;
		auto& cart = world.armies[ConvoyArmy];
		const int next = ConvoyPath()[convoyStep+1];
		if (intelligence[0].EnemyNear(world, next, 1))
		{
            ++battleReview.enemyStops;
			Report(IsRescue() ? U"避難車が停止。前路に敵がいる。追う隊と車を守る隊を分けよう。" : U"糧車が停止。前路の敵を押し返すか、引きつけて道を空けよう。");
			return;
		}
		if (!TacticChain::Free(world, next))
		{
            ++battleReview.trafficStops;
			Report(IsRescue() ? U"避難車の道が塞がれている。味方も車道を空けよう。" : U"糧車の道が塞がれている。護衛の隊も進路を空ける必要がある。");
			return;
		}
		cart.tile = cart.target = next;
		++convoyStep;
		++world.revision;
		Report(IsRescue() ? U"季安の避難車が１区間進んだ。護送完了まで、前路と後ろを見届けよう。" : U"糧車が１区間進んだ。先鋒への追撃と護衛の配分を見直そう。");
	}
	void StrategistJourney::Evaluate()
	{
		if (stage != JourneyStage::Battle)
			return;
		if (world.armies[0].troops + world.armies[1].troops + world.armies[2].troops < 1800)
		{
			Finish(false, U"主力の兵が1,800を下回った。任務を中止して撤退した。");
			return;
		}
		const auto kind = Definition().kind;
		if (HasConvoy())
		{
			const auto& cart = world.armies[ConvoyArmy];
			if (cart.troops <= 0 || (!IsRescue() && cart.cargoFood < Definition().requiredProgress))
			{
				Finish(false, IsRescue() ? U"避難車の護衛が壊滅し、送り届ける約束を果たせなかった。敵将への追撃と、車を守る配置を見直そう。"
				       : U"糧車の護衛または必要な積荷を失った。主力の追撃より護衛を優先する場面を見直そう。");
				return;
			}
			if (cart.tile == goal)
			{
				Finish(true, IsRescue() ? U"季安の避難車が東の出口へ到着。敵を倒すだけでなく、送り届ける約束を果たした。"
				                      : U"糧車が東端へ到着。守り抜いた実際の積荷と生存兵が次の軍議へ戻る。");
				return;
			}
		}
		else if (kind == JourneyObjective::HoldPositions)
		{
			if (heldTurns >= Definition().requiredProgress)
			{
				Finish(true, U"二つの烽火を３指揮守り抜いた。守備と支援の配分で本隊の退路をつないだ。");
				return;
			}
		}
		else
		{
			if (world.armies[3].troops <= 450 || world.armies[3].morale <= 30)
			{
				Finish(true, U"敵の先鋒が崩れ、退路を確保した。生存兵と携行糧が野営地へ戻る。");
				return;
			}
			if (PositionSecured(goal))
			{
				Finish(true, U"別路から退路の出口を確保した。撃破だけに頼らず道を開いた。");
				return;
			}
		}
		if (turn > TurnBudget())
			Finish(false, U"任務の期限を超過。長い連鎖を待つか、目的へ急ぐかを見直そう。");
	}
} // namespace frontline
