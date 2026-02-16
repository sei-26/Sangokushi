#pragma once
#include <Siv3D.hpp>

namespace OfficerSkill
{
	enum class SkillType
	{
		None, FireAttack, Ambush, Provoke, Rally, Heal, Charge
	};

	struct Skill
	{
		SkillType type = SkillType::None;
		int cooldown = 0;
		int currentCooldown = 0;

		bool CanUse() const { return currentCooldown == 0; }
		void Use() { currentCooldown = cooldown; }
		void AdvanceTurn() { if (currentCooldown > 0) currentCooldown--; }
	};

	static String GetSkillName(SkillType type)
	{
		switch (type)
		{
		case SkillType::FireAttack: return U"火計";
		case SkillType::Ambush: return U"伏兵";
		case SkillType::Provoke: return U"挑発";
		case SkillType::Rally: return U"鼓舞";
		case SkillType::Heal: return U"治療";
		case SkillType::Charge: return U"突撃";
		default: return U"なし";
		}
	}

	static String GetSkillIcon(SkillType type)
	{
		switch (type)
		{
		case SkillType::FireAttack: return U"🔥";
		case SkillType::Ambush: return U"🌿";
		case SkillType::Provoke: return U"😡";
		case SkillType::Rally: return U"📣";
		case SkillType::Heal: return U"💊";
		case SkillType::Charge: return U"⚡";
		default: return U"";
		}
	}

	static Color GetSkillColor(SkillType type)
	{
		switch (type)
		{
		case SkillType::FireAttack: return Color(255, 100, 0);
		case SkillType::Ambush: return Color(100, 200, 100);
		case SkillType::Provoke: return Color(255, 50, 50);
		case SkillType::Rally: return Color(100, 150, 255);
		case SkillType::Heal: return Color(100, 255, 100);
		case SkillType::Charge: return Color(255, 255, 50);
		default: return Palette::Gray;
		}
	}

	static Skill GenerateSkill(int intelligence)
	{
		Skill skill;
		if (intelligence >= 80)
		{
			skill.type = SkillType::FireAttack;
			skill.cooldown = 3;
		}
		else if (intelligence >= 70)
		{
			skill.type = SkillType::Rally;
			skill.cooldown = 3;
		}
		else if (intelligence >= 60)
		{
			skill.type = SkillType::Ambush;
			skill.cooldown = 2;
		}
		else
		{
			skill.type = SkillType::Charge;
			skill.cooldown = 2;
		}
		return skill;
	}

	static int ApplySkillDamage(SkillType type, int baseDamage)
	{
		switch (type)
		{
		case SkillType::FireAttack: return baseDamage * 2;
		case SkillType::Charge: return baseDamage * 15 / 10;
		case SkillType::Ambush: return baseDamage * 13 / 10;
		default: return baseDamage;
		}
	}
}
