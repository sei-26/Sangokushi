#include "HeroStory.hpp"
namespace hero
{
	int Story::ObjectiveTile() const
	{
		if (!deepRules)
			return -1;
		return chapter == 0 ? 5 * W + 4 : chapter == 4 ? 4 * W + 8 : chapter == 5 ? 3 * W + 6 : -1;
	}
	bool Story::VowRequired() const
	{
		return deepRules && std::find(decisions.begin(), decisions.end(), chapter * 10) != decisions.end();
	}
	std::u32string Story::ObjectiveName() const
	{
		return chapter == 0   ? U"焼けた村"
		       : chapter == 4 ? U"同盟の退路"
		       : chapter == 5 ? U"定軍山の高地"
		                      : U"仲間との約束";
	}
	bool Story::ObjectiveMet() const
	{
		if (!deepRules)
			return true;
		if (chapter == 1)
			return civilActions[1] > 0 && (VowRequired() ? civilActions[0] > 0 : civilActions[2] > 0);
		if (chapter == 2)
			return visits == 3 && civilActions[2] >= 2;
		if (chapter == 3)
			return escaped >= (VowRequired() ? 4 : 3);
		return objectiveProgress >= 2 && (chapter != 4 || fireUsed);
	}
	std::u32string Story::ObjectiveHelp() const
	{
		if (!deepRules)
			return U"以前のルールで継続中";
		if (chapter == 1)
			return VowRequired() ? U"救済と市の再開を行い、6施策以内に民心60へ"
			                     : U"市の再開と守備訓練を行い、6施策以内に民心60へ";
		if (chapter == 2)
			return U"訪問の間に仲間と対話 / 訪問3回・対話2回";
		if (chapter == 3)
			return VowRequired() ? U"約束：避難民を4組護送 / 2組喪失で失敗"
			                     : U"避難民を3組護送 / 3組喪失で失敗";
		return ObjectiveName() + (VowRequired() ? U"を守る約束 / 必須" : U"を守れば次章へ成果 / 任意") +
		       (chapter == 4 ? U"・火攻めも必要" : U"");
	}
	std::u32string Story::ChoiceImpact(int choice) const
	{
		if (!deepRules)
			return U"信望・決意と親密度に反映";
		if (chapter == 1)
			return choice == 0 ? U"救済＋商いの復興を達成条件に" : U"守備訓練＋商いの復興を達成条件に";
		if (chapter == 2)
			return U"訪問の間に仲間と語る / 対話が登用を支える";
		if (chapter == 3)
			return choice == 0 ? U"4組救出を約束 / 2組失うと敗北" : U"3組救出で撤退 / 4組なら次章に成果";
		return choice == 0 ? ObjectiveName() + U"を2手連続確保してから勝利"
		                   : U"敵を退けて勝利 / 拠点確保は任意";
	}
	void Story::AdvanceObjective()
	{
		const int tile = ObjectiveTile();
		if (tile < 0 || objectiveProgress >= 2)
			return;
		const int unit = At(tile % W, tile / W);
		bool safe = unit >= 0 && units[unit].hero >= 0 && units[unit].hp > 0;
		for (const auto& e : units)
			if (e.enemy && e.hp > 0 && std::abs(e.x - tile % W) + std::abs(e.y - tile / W) <= 1)
				safe = false;
		objectiveProgress = safe ? objectiveProgress + 1 : 0;
		if (objectiveProgress == 2)
		{
			GainSpirit(20);
			if (chapter == 0)
				for (auto& u : units)
					if (u.hero >= 0 && u.hp > 0)
						u.hp = std::min(u.maxHp, u.hp + 2);
			Record(ObjectiveName() + U"を確保！ 敵が隣接しない状態で2手守り抜いた。闘志+20。");
		}
	}
	bool Story::CanAttack(int index, int target) const
	{
		if (index < 0 || target < 0 || index >= static_cast<int>(units.size()) ||
		    target >= static_cast<int>(units.size()))
			return false;
		const auto& a = units[index];
		const auto& b = units[target];
		if (a.hp <= 0 || b.hp <= 0 || Distance(a, b) > a.range)
			return false;
		return !deepRules || battle::ClearRay(a.x, a.y, b.x, b.y, [&](int x, int y) {
			return terrain[y * W + x] == 1 || terrain[y * W + x] == 2;
		});
	}
	bool Story::Guard(int index)
	{
		if (!BattleReady() || !deepRules || index < 0 || index >= static_cast<int>(units.size()))
			return false;
		auto& u = units[index];
		if (u.hero < 0 || u.hp <= 0 || u.acted)
			return false;
		u.guarding = u.acted = true;
		Record(Name(u.hero) + U"が守備を固めた。次の敵手番は被害-2、隣接攻撃へ反撃1。");
		return true;
	}
} // namespace hero
