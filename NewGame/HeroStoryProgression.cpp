#include "HeroStory.hpp"

namespace hero
{
	bool Story::Choose(int choice)
	{
		if ((phase != 0 && phase != 1 && phase != 3) || choice < 0 || choice > 1)
			return false;
		const int stage = phase == 3 ? 2 : phase;
		decisions.push_back(chapter * 10 + stage * 2 + choice);
		Record(std::u32string(Book()[chapter].speaker[stage]) + U"と語り、「" +
		       Book()[chapter].choices[stage][choice] + U"」を選んだ。");
		if (phase == 1)
		{
			if (planner < 0)
				planner = 1;
			preparation = choice;
			if (choice == 0)
			{
				food += 25 + PlanPower(0) * 5;
				bonds[Companion()] += 8;
			}
			else
			{
				gold += 25;
				resolve += 6;
				if (chapter == 2)
					food += PlanPower(1) * 5;
			}
			ChangeBond(0, planner, 4);
			Record(Name(planner) +
			       (Tactical() ? U"に準備を任せた。担当者は開戦の1手目に段取りを担い、次の手から行動する。"
			                   : U"に支度を任せた。段取りの得意さを、内政と訪問に生かす。"));
			phase = 2;
			StartMission();
		}
		else
		{
			if (choice == 0)
			{
				virtue += 8;
				gold -= 10;
				bonds[Companion()] += 8;
			}
			else
			{
				resolve += 8;
				gold += 15;
				bonds[2] += 6;
			}
			if (phase == 0)
				phase = 1;
			else if (chapter == 5)
			{
				phase = 4;
				units.clear();
				Record(U"漢中王の旗の下、英雄譚の第一部を完結した。");
			}
			else
			{
				++chapter;
				phase = 0;
				units.clear();
			}
		}
		Clamp();
		return true;
	}

	void Story::StartMission()
	{
		failed = fireUsed = false;
		turn = 1;
		tasks = visits = escaped = lost = 0;
		order = preparation == 0 ? 50 : 40;
		spirit = 20;
		battleEvent = eventMask = rallies = 0;
		units.clear();
		objectiveProgress = 0;
		civilActions = {};
		if (deepRules)
		{
			if (chapter == 1 && outcomes[0] == 2)
			{
				order += 8;
				Record(U"初陣で村を守った評判が届く。徐州の開始民心+8。");
			}
			if (chapter >= 3 && outcomes[2] == 2)
			{
				spirit += 10;
				Record(U"隆中で仲間と交わした対話が初動を支える。開始闘志+10。");
			}
		}
		terrain.fill(0);
		if (!Tactical())
		{
			if (chapter == 1 && preparation == 1)
				order += PlanPower(1) * 3;
			return;
		}
		if (chapter == 3)
			for (int y = 0; y < H; ++y)
				terrain[y * W + 5] = y == 3 ? 3 : 2;
		if (chapter == 4)
			for (int x = 0; x < W; ++x)
				terrain[3 * W + x] = x == 2 || x == 5 || x == 8 ? 3 : 2;
		if (chapter == 5)
			for (int y : {0, 1, 5, 6})
				terrain[y * W + 5] = 2;
		for (int p : {W + 4, 4 * W + 4, 2 * W + 7, 5 * W + 7})
			if (terrain[p] == 0)
				terrain[p] = 1;
		const int count = chapter == 0 ? 3 : chapter == 3 || chapter == 4 ? 5 : 6;
		const int legacyBonus = deepRules ? (outcomes[1] == 2 ? 1 : 0) +
		                                        (chapter == 4 && outcomes[3] == 2 ? 1 : 0) +
		                                        (chapter == 5 && outcomes[4] == 2 ? 1 : 0)
		                                  : 0;
		const int hp = legacyBonus + 10 + virtue / 30 + (preparation == 0 ? 2 + PlanPower(0) / 2 : 0),
		          attack = 3 + resolve / 50 + preparation + (preparation == 1 && PlanPower(1) == 3 ? 1 : 0);
		for (int g = 0; g < count; ++g)
		{
			Unit u;
			u.hero = g;
			u.x = 1 + g % 2;
			u.y = chapter == 4 ? 4 + g / 2 : 1 + g;
			u.hp = u.maxHp = hp;
			u.attack = attack;
			u.acted = (g == planner);
			if (g == 4 || g == 5)
				u.range = 2;
			units.push_back(u);
		}
		const int enemies = chapter == 0 ? 3 : chapter == 5 ? 5 : 4;
		for (int i = 0; i < enemies; ++i)
		{
			Unit e;
			e.enemy = true;
			e.x = 7 + i % 3;
			e.y = chapter == 4 ? i / 3 : 1 + i;
			e.hp = e.maxHp = chapter == 5 ? 10 : 8;
			e.attack = 2;
			if (i == 2)
				e.range = 2;
			units.push_back(e);
		}
		if (chapter == 3)
			for (int y : {0, 1, 2, 4, 6})
			{
				Unit c;
				c.civilian = true;
				c.x = 0;
				c.y = y;
				c.hp = c.maxHp = 5;
				c.attack = 0;
				units.push_back(c);
			}
		Record(chapter == 3 ? U"張飛「橋を渡れ！ 追手は俺たちが止める！」 民を3組、右端の渡し場へ護送せよ。"
		       : chapter == 4
		           ? U"周瑜「合図までは動くな。江を燃やす、その時を待て！」 同盟軍と火攻めをつなげ。"
		       : chapter == 5 ? U"黄忠「この一射で、皆の進む道を開こう！」 漢中の決戦、敵の戦列を崩せ。"
		                      : U"関羽「兄者の志、今こそ戦場で示しましょう！」 三人で初陣を勝ち抜け。");
	}

	void Story::Finish(bool success)
	{
		if (phase != 2 || failed)
			return;
		if (!success)
		{
			failed = true;
			Record(U"任務で退却。仲間を立て直して再挑戦できる。");
			return;
		}
		if (deepRules)
		{
			outcomes[chapter] = ObjectiveMet() ? 2 : 1;
			Record(ObjectiveMet() ? U"約束を果たした。今回の成果は次の章にも残る。"
			                      : U"戦場は勝ち抜いた。任意の約束は果たせず、その成果は持ち越せない。");
		}
		phase = 3;
		battleEvent = 0;
		reputation += 10 + (chapter == 3 ? escaped * 2 : 0);
		food += 20;
		gold += 15;
		for (const auto& u : units)
			if (u.hero > 0 && u.hp > 0)
				bonds[u.hero] += 5;
		for (size_t i = 0; i < units.size(); ++i)
			for (size_t j = i + 1; j < units.size(); ++j)
				if (units[i].hero > 0 && units[j].hero > 0 && units[i].hp > 0 && units[j].hp > 0)
					ChangeBond(units[i].hero, units[j].hero, 3);
		if (chapter == 2)
			bonds[4] += 15;
		Record(std::u32string(Book()[chapter].title) + U"の任務を完了。" +
		       (bonds[Companion()] >= 70 ? Name(Companion()) + U"との信頼が、次の道を支える。"
		                                 : U"仲間と歩みを振り返ろう。"));
		Clamp();
	}

	void Story::Retry()
	{
		if (phase == 2 && failed)
		{
			food = std::max(food, 60);
			gold = std::max(gold, 60);
			StartMission();
		}
	}

	bool Story::CivilAction(int action)
	{
		if (phase != 2 || failed || Tactical() || action < 0 || action > 2)
			return false;
		if (chapter == 1)
		{
			if (action == 0)
			{
				if (food < 25)
					return false;
				food -= 25;
				order += 18;
				virtue += 2;
				Record(U"糜竺と開倉し、住民へ糧を配った。");
			}
			if (action == 1)
			{
				gold += 30;
				order += 8;
				Record(U"簡雍と市を立て直し、商いを再開した。");
			}
			if (action == 2)
			{
				if (gold < 20)
					return false;
				gold -= 20;
				order -= 4;
				resolve += 4;
				Record(U"関羽と守備隊を訓練。民の負担にも目を向けたい。");
			}
			++civilActions[action];
			++tasks;
			++turn;
			order = std::clamp(order, 0, 100);
			Clamp();
			if (tasks >= 3 && order >= 60 && (!deepRules || ObjectiveMet()))
				Finish(true);
			else if (tasks >= 6)
				Finish(false);
		}
		else
		{
			if (action == 0)
			{
				if (food < 20 || (deepRules && civilActions[2] < visits))
					return false;
				food -= 20;
				++visits;
				virtue += 2;
				bonds[4] += 5;
				Record(visits == 3 ? U"三度目の訪問。諸葛亮と、天下と民の行く末を語った。"
				                   : U"隆中を訪ねた。会えぬ日にも、相手の暮らしと志を思う。");
			}
			if (action == 1)
			{
				food += 25;
				gold -= std::min(10, gold);
				Record(U"留守の軍を整え、次の訪問に備えた。");
			}
			if (action == 2)
			{
				ChangeBond(1, 2, 4);
				bonds[1] += 4;
				bonds[2] += 4;
				resolve += 2;
				food += 5;
				Record(U"関羽・張飛に人材を求める理由を伝え、理解を深めた。");
			}
			++civilActions[action];
			++turn;
			Clamp();
			if (visits == 3)
				Finish(true);
		}
		return true;
	}
} // namespace hero
