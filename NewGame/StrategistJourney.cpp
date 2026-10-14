#include "StrategistJourney.hpp"
#include <memory>

namespace frontline
{
	bool StrategistJourney::OnMap(int tile)
	{
		return Campaign::Valid(tile) && tile % Width < Columns && tile / Width < Rows;
	}
	const char32_t* StrategistJourney::EnemyName(int policy)
	{
		const char32_t* names[] = {U"追撃を好む敵", U"誘いに乗らない敵", U"森林を探る敵"};
		return names[std::clamp(policy, 0, 2)];
	}
	const char32_t* StrategistJourney::MissionName() const
	{
		return mission == 0 ? U"第一策 / 民の退路を開け" : U"第二策 / 追撃から仲間を守れ";
	}
	void StrategistJourney::Report(const std::u32string& text)
	{
		reports.push_back({world.day, text});
		message = text;
	}
	void StrategistJourney::Reset()
	{
		world = Campaign{};
		world.hexMap = true;
		world.tileRegion.fill(-1);
		for (auto& tile : world.tiles)
			tile = {Terrain::Sea, -1};
		for (int y = 1; y < Rows - 1; ++y)
			for (int x = 1; x < Columns - 1; ++x)
			{
				auto& tile = world.tiles[Campaign::At(x, y)];
				tile = {Terrain::Plain, x <= 9 ? 0 : 1};
				if (y >= 5 && y <= 9 && x >= 5 && x <= 10)
					tile.terrain = Terrain::Forest;
				if (y == 2 && x >= 8)
					tile.terrain = Terrain::Mountain;
			}
		world.cities = {{U"義勇軍の野営地", Campaign::At(2, 10), 0, 6000, 18000, 900},
		                {U"敵の集結地", Campaign::At(14, 3), 1, 2000, 30000, 1000}};
		world.cities[0].logistics = world.cities[1].logistics = 80;
		const char32_t* names[] = {U"劉備", U"関羽", U"張飛", U"黄巾の先鋒", U"黄巾の別働隊"};
		for (int i = 0; i < 5; ++i)
		{
			General general;
			general.name = names[i];
			general.faction = i < 3 ? 0 : 1;
			general.home = i < 3 ? 0 : 1;
			general.leadership = i == 0 ? 75 : i == 1 ? 90 : i == 2 ? 92 : 65;
			general.trait = i == 0 ? Trait::Guardian : Trait::Valiant;
			general.tactic = i == 0 ? Tactic::Rally : i == 1 ? Tactic::Ambush : Tactic::Charge;
			world.generals.push_back(general);
		}
		world.bonds = {{0, 1, 40}, {0, 2, 40}, {1, 2, 40}};
		stage = JourneyStage::Encounter;
		mission = trust = bestChain = preparation = deployed = casualties = survivors = 0;
		turn = 1;
		enemyPolicy = 0;
		won = reinforced = false;
		goal = Campaign::At(12, 10);
		lastChain = {};
		reports.clear();
		for (auto& known : intelligence)
			known.Reset(world);
		chain.Reset(world);
		Report(U"184年、黄巾の乱。名もなき軍師は、逃げる民と義勇軍に出会う。局地戦と会話は創作。");
	}
	void StrategistJourney::Meet()
	{
		if (stage != JourneyStage::Encounter)
			return;
		stage = JourneyStage::Preparation;
		Report(U"劉備から退路の確保を任された。兵を集め、伏兵と突破の段取りを決めよう。");
	}
	bool StrategistJourney::Reinforce()
	{
		auto& camp = world.cities[0];
		if (stage != JourneyStage::Preparation || reinforced || camp.gold < 250 || camp.food < 500)
			return false;
		camp.gold -= 250;
		camp.food -= 500;
		camp.troops += 1200;
		reinforced = true;
		Report(U"軍資金250・兵糧500を使い、義勇兵1,200を集めた。今回はこれ以上募れない。");
		return true;
	}
	bool StrategistJourney::Prepare(int choice, int policy)
	{
		if (stage != JourneyStage::Preparation || choice < 0 || choice > 1 || policy < 0 || policy > 2)
			return false;
		auto& camp = world.cities[0];
		const int food = choice == 0 ? 3000 : 4800, gold = choice == 0 ? 200 : 80;
		if (camp.food < food || camp.gold < gold || camp.troops < 1600)
		{
			message = U"兵・兵糧・軍資金が不足。募兵か準備方針を見直そう。";
			return false;
		}
		camp.food -= food;
		camp.gold -= gold;
		deployed = std::min(4200, camp.troops - 1000);
		camp.troops -= deployed;
		preparation = choice;
		enemyPolicy = policy;
		turn = 1;
		won = false;
		survivors = casualties = 0;
		lastChain = {};
		world.armies.clear();
		const int positions[] = {Campaign::At(9, 10), Campaign::At(8, 9), Campaign::At(8, 10),
		                         Campaign::At(10, 10),
		                         policy == 2 ? Campaign::At(7, 11) : Campaign::At(14, 11)};
		for (int i = 0; i < 5; ++i)
		{
			Army army;
			army.general = i;
			army.faction = i < 3 ? 0 : 1;
			army.tile = army.target = positions[i];
			army.troops = i < 3 ? deployed / 3 + (i < deployed % 3 ? 1 : 0) : i == 3 ? 1800 : 1300;
			army.food = i < 3 ? food / 3 : 3000;
			army.morale = choice == 1 && i < 3 ? 100 : 90;
			army.stance = i >= 3 && policy == 1 ? battle::Stance::Guard : battle::Stance::Balanced;
			army.scouting = i == 4 && policy == 2;
			world.armies.push_back(army);
		}
		stage = JourneyStage::Battle;
		chain.Reset(world);
		if (choice == 0)
			chain.ArmAmbush(world, 1, message);
		for (auto& known : intelligence)
			known.Reset(world);
		RefreshIntelligence();
		Report(choice == 0
		           ? (chain.concealed[1]
		                  ? U"関羽隊が森林で伏兵を準備。劉備の誘導 → 関羽の伏兵 → 張飛の突破を狙える。"
		                  : U"敵の斥候が接近し、伏兵を見破った。斥候を退けるか、別の森林へ配置し直そう。")
		           : U"多めの兵糧で出陣。関羽を選び「伏兵準備」をすれば連鎖を組める。");
		return true;
	}
	void StrategistJourney::RefreshIntelligence()
	{
		intelligence[0].Observe(world, 0);
		auto view = std::make_unique<Campaign>(world);
		// Concealment changes what the opponent sees, not physical collision or damage.
		for (int i = 0; i < 3 && i < static_cast<int>(view->armies.size()); ++i)
		{
			if (!chain.concealed[i])
				continue;
			bool exposed = false;
			for (int enemy = 3; enemy < static_cast<int>(world.armies.size()); ++enemy)
				if (TacticChain::Living(world, enemy) &&
				    world.MapDistance(world.armies[i].tile, world.armies[enemy].tile) <=
				        (world.armies[enemy].scouting ? 3 : 1) &&
				    world.ClearShot(world.armies[enemy].tile, world.armies[i].tile))
					exposed = true;
			if (!exposed)
				view->armies[i].troops = 0;
		}
		intelligence[1].Observe(*view, 1);
		for (int i = 0; i < 3 && i < static_cast<int>(world.armies.size()); ++i)
		{
			const auto* contact = intelligence[1].Find(i);
			if (chain.concealed[i] && contact && contact->current)
			{
				chain.concealed[i] = false;
				Report(world.generals[i].name + U"隊の伏兵が見破られた。敵の接近と索敵に注意。");
			}
		}
	}
	bool StrategistJourney::Move(int army, int tile)
	{
		if (stage != JourneyStage::Battle || !OnMap(tile))
			return false;
		const bool result = chain.Move(world, intelligence[0], army, tile, message);
		if (result)
		{
			RefreshIntelligence();
			Evaluate();
		}
		return result;
	}
	bool StrategistJourney::Ambush(int army)
	{
		if (stage != JourneyStage::Battle)
			return false;
		const bool result = chain.ArmAmbush(world, army, message);
		if (result)
			RefreshIntelligence();
		return result;
	}
	bool StrategistJourney::Strike(int army, int enemy)
	{
		if (stage != JourneyStage::Battle)
			return false;
		const bool result = chain.Strike(world, intelligence[0], army, enemy, message);
		if (result)
		{
			RefreshIntelligence();
			Evaluate();
		}
		return result;
	}
	ChainResult StrategistJourney::Preview(const std::vector<ChainOrder>& orders) const
	{
		return chain.Preview(world, intelligence[0], orders);
	}
	ChainResult StrategistJourney::Execute(const std::vector<ChainOrder>& orders)
	{
		if (stage != JourneyStage::Battle)
			return {};
		lastChain = chain.Execute(world, intelligence[0], orders);
		bestChain = std::max(bestChain, lastChain.completed);
		for (const auto& event : lastChain.events)
			Report(event.reason);
		if (lastChain.events.empty())
			message = U"担当・戦法・対象を予約してから発動する。最大３手。";
		RefreshIntelligence();
		Evaluate();
		return lastChain;
	}
	void StrategistJourney::Finish(bool success, const char32_t* reason)
	{
		won = success;
		stage = JourneyStage::Debrief;
		survivors = 0;
		for (int i = 0; i < 3; ++i)
		{
			survivors += world.armies[i].troops;
			if (world.armies[i].troops > 0)
				world.cities[0].food += world.armies[i].food;
			world.armies[i].food = 0;
		}
		world.cities[0].troops += survivors;
		casualties = deployed - survivors;
		if (success)
		{
			trust += 10;
			for (int i = 0; i < 3; ++i)
				for (int j = i + 1; j < 3; ++j)
					if (world.armies[i].troops > 0 && world.armies[j].troops > 0)
						world.ChangeBond(i, j, 3);
		}
		Report(reason);
	}
	void StrategistJourney::Evaluate()
	{
		if (stage != JourneyStage::Battle)
			return;
		if (world.armies[0].troops + world.armies[1].troops + world.armies[2].troops < 1800)
		{
			Finish(false, U"義勇軍の損耗が大きく、撤退した。次の戦いへ残せる兵を考え直そう。");
			return;
		}
		if (world.armies[3].troops <= 450 || world.armies[3].morale <= 30)
		{
			Finish(true, U"敵の先鋒が崩れ、退路を確保した。生き残った兵と残りの携行糧が野営地へ戻る。");
			return;
		}
		bool secured = false, threatened = false;
		for (int i = 0; i < 3; ++i)
			if (world.armies[i].troops > 0 && world.armies[i].tile == goal)
				secured = true;
		for (int i = 3; i < static_cast<int>(world.armies.size()); ++i)
			if (world.armies[i].troops > 0 && world.MapDistance(world.armies[i].tile, goal) <= 1)
				threatened = true;
		if (secured && !threatened)
			Finish(true, U"別路から退路の出口を確保した。撃破だけに頼らず、民を逃がす道を開いた。");
		else if (turn > TurnLimit)
			Finish(false, U"退路を開く猶予を失った。誘導だけに固執せず、直接攻撃や別路も試そう。");
	}
	bool StrategistJourney::Continue()
	{
		if (stage != JourneyStage::Debrief || !won)
			return false;
		if (mission == 1)
		{
			stage = JourneyStage::Complete;
			Report(U"序章の試作範囲を完了。仕官の選択と、その先の歴史戦役は今後つなぐ。");
		}
		else
		{
			++mission;
			stage = JourneyStage::Preparation;
			reinforced = false;
			enemyPolicy = 1;
			Report(U"前の戦果を受けて、新たな退路の確保を任された。残った兵・兵糧・信頼で次の軍議へ。");
		}
		return true;
	}
} // namespace frontline
