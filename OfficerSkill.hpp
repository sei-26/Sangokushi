#pragma once
#include <Siv3D.hpp>

// 武将の固有スキル
struct OfficerSkill
{
	String name;
	String description;
	int cooldown;
	int currentCooldown = 0;

	enum class SkillType
	{
		Damage, Buff, Debuff, Heal, Special
	} type;

	double damageMultiplier = 1.0;
	int effectValue = 0;
	int effectDuration = 0;
	String effectDescription;

	bool CanUse() const { return currentCooldown == 0; }
	void AdvanceCooldown() { if (currentCooldown > 0) currentCooldown--; }
	void Use() { currentCooldown = cooldown; }

	static OfficerSkill GetSkillForOfficer(const String& officerName)
	{
		// 劉備陣営
		if (officerName == U"劉備") return { U"仁徳の采配", U"味方全員の士気+30、防御+20%", 4, 0, SkillType::Buff, 1.0, 30, 3, U"仁徳により味方を鼓舞" };
		if (officerName == U"関羽") return { U"青龍偃月刀", U"敵単体に200%ダメージ、士気-20", 3, 0, SkillType::Damage, 2.0, -20, 1, U"青龍偃月刀の一閃！" };
		if (officerName == U"張飛") return { U"蛇矛猛撃", U"敵全体に120%ダメージ、恐怖付与", 4, 0, SkillType::Damage, 1.2, -15, 2, U"蛇矛を振るう" };
		if (officerName == U"諸葛亮") return { U"八卦陣", U"味方全員の防御+50% 3ターン", 5, 0, SkillType::Buff, 1.0, 50, 3, U"神秘の陣形" };
		if (officerName == U"趙雲") return { U"七星槍", U"敵単体に180%ダメージ", 3, 0, SkillType::Damage, 1.8, 0, 1, U"連続攻撃" };

		// 曹操陣営
		if (officerName == U"曹操") return { U"覇王の威圧", U"敵全員の士気-25、攻撃-15%", 4, 0, SkillType::Debuff, 1.0, -25, 2, U"覇気で威圧" };
		if (officerName == U"夏侯惇") return { U"隻眼の咆哮", U"自身の攻撃+80% 2ターン", 3, 0, SkillType::Buff, 1.0, 80, 2, U"怒りの力" };
		if (officerName == U"許褚") return { U"虎痴の剛力", U"敵単体に250%ダメージ", 4, 0, SkillType::Damage, 2.5, 0, 1, U"全力の一撃" };
		if (officerName == U"司馬懿") return { U"死せる孔明", U"味方全員の知略+30%", 5, 0, SkillType::Buff, 1.0, 30, 3, U"高度な戦術" };

		// 孫権陣営
		if (officerName == U"孫権") return { U"江東の采配", U"味方全員の攻撃+25%", 4, 0, SkillType::Buff, 1.0, 25, 3, U"江東軍を率いる" };
		if (officerName == U"周瑜") return { U"赤壁の炎", U"敵全体に150%炎ダメージ", 5, 0, SkillType::Damage, 1.5, 0, 3, U"業火で焼き尽くす" };
		if (officerName == U"甘寧") return { U"錦帆賊の急襲", U"敵単体に180%ダメージ", 3, 0, SkillType::Damage, 1.8, 0, 1, U"電光石火の奇襲" };

		// 呂布・その他
		if (officerName == U"呂布") return { U"方天画戟", U"敵単体に300%ダメージ", 5, 0, SkillType::Damage, 3.0, 0, 1, U"最強武将の猛攻" };
		if (officerName == U"馬超") return { U"西涼の錦馬超", U"敵単体に220%ダメージ", 3, 0, SkillType::Damage, 2.2, 0, 1, U"神速の突撃" };
		if (officerName == U"黄忠") return { U"老いてなお猛し", U"敵単体に190%ダメージ", 3, 0, SkillType::Damage, 1.9, 0, 1, U"老将の熟練技" };

		// デフォルト
		return { U"奮起", U"自身の攻撃+30% 2ターン", 2, 0, SkillType::Buff, 1.0, 30, 2, U"気合を入れる" };
	}
};
