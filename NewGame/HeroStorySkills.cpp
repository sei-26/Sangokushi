#include "HeroStory.hpp"

namespace hero
{
	void Story::GainSpirit(int amount)
	{
		spirit = std::clamp(spirit + amount, 0, 100);
	}

	bool Story::Rally()
	{
		if (!BattleReady() || spirit < 60)
			return false;
		spirit -= 60;
		++rallies;
		for (auto& u : units)
			if (u.hero >= 0 && u.hp > 0)
			{
				u.acted = false;
				u.hp = std::min(u.maxHp, u.hp + 2);
			}
		Record(U"劉備「ここで終わらせぬ！ 我らの旗に、もう一度力を！」 "
		       U"絆の号令で生存する武将が再行動、耐久+2。");
		return true;
	}

	bool Story::ResolveBattleEvent(int choice)
	{
		if (phase != 2 || failed || battleEvent == 0 || choice < 0 || choice > 1)
			return false;
		const int event = battleEvent;
		battleDecisions.push_back(chapter * 10 + (event - 1) * 2 + choice);
		if (battleDecisions.size() > 100)
			battleDecisions.erase(battleDecisions.begin());
		battleEvent = 0;
		if (choice == 0)
		{
			GainSpirit(25);
			bonds[chapter == 3 ? 2 : Companion()] = std::min(100, bonds[chapter == 3 ? 2 : Companion()] + 8);
			for (auto& u : units)
				if (u.hero >= 0 && u.hp > 0)
					u.acted = false;
			if (chapter == 3)
				for (auto& u : units)
					if (u.enemy && u.hp > 0)
						u.stunned = std::max(u.stunned, 1);
			Record(chapter == 3
			           ? U"張飛「ここは俺が引き受けた！ 子龍、民を頼む！」 "
			             U"追手を1手足止め。武将全員が再行動、闘志+25。"
			           : U"仲間の声が戦列をつなぐ。武将全員が再行動、闘志+25。ひとりの強さを、皆の力に。");
		}
		else
		{
			for (auto& u : units)
				if (!u.enemy && u.hp > 0)
					u.hp = std::min(u.maxHp, u.hp + 4);
			GainSpirit(15);
			virtue = std::min(100, virtue + 3);
			Record(chapter == 3 ? U"趙雲「ひとりも見捨てません。私の後へ！」 武将と民の耐久+4、闘志+15。"
			                    : U"劉備「傷ついた者を下げよ。皆で生きて帰るぞ！」 味方の耐久+4、闘志+15。");
		}
		return true;
	}

	bool Story::Skill(int index)
	{
		if (!BattleReady() || index < 0 || index >= static_cast<int>(units.size()))
			return false;
		auto& u = units[index];
		if (u.hp <= 0 || u.hero < 0 || u.acted || u.skillUsed)
			return false;
		bool effect = false;
		for (int i = 0; i < static_cast<int>(units.size()); ++i)
		{
			auto& t = units[i];
			if (t.hp <= 0)
				continue;
			if ((u.hero == 0 || u.hero == 3) && !t.enemy && Distance(u, t) <= 2 && t.hp < t.maxHp)
			{
				t.hp = std::min(t.maxHp, t.hp + (u.hero == 0 ? 4 : 5));
				effect = true;
			}
			if ((u.hero == 1 || u.hero == 5) && t.enemy && Distance(u, t) <= (u.hero == 1 ? 1 : 3))
			{
				t.hp = std::max(0, t.hp - (u.hero == 1 ? 7 : 6));
				effect = true;
				break;
			}
			if (u.hero == 2 && t.enemy && Distance(u, t) <= 2)
			{
				t.stunned = 1;
				effect = true;
			}
			if (u.hero == 4 && t.hero >= 0 && t.hero != 4 && t.acted && Distance(u, t) <= 2)
			{
				t.acted = false;
				effect = true;
				break;
			}
		}
		if (!effect)
			return false;
		u.skillUsed = u.acted = true;
		GainSpirit(12);
		Record(Name(u.hero) + U"が「" + SkillName(u.hero) + U"」で仲間を支えた。");
		CheckBattle();
		return true;
	}

	bool Story::FireSignal()
	{
		if (chapter != 4 || !BattleReady() || turn < 3 || fireUsed || food < 15)
			return false;
		food -= 15;
		fireUsed = true;
		const bool cooperative = std::find(decisions.begin(), decisions.end(), 40) != decisions.end();
		for (auto& u : units)
			if (u.enemy && u.hp > 0 && u.y < 3)
				u.hp = std::max(0, u.hp - (cooperative ? 6 : 4));
		GainSpirit(25);
		Record(U"周瑜「今だ、火を放て！」 連合軍の火攻に合わせて進軍。江上の炎が敵の戦列を断つ。");
		CheckBattle();
		return true;
	}
} // namespace hero
