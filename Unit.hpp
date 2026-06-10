#pragma once
#include <Siv3D.hpp>
#include "OfficerSkill.hpp"

enum class Side { Player, Enemy };

struct Unit
{
	String name;
	Side side;
	Point pos;

	int soldiers;
	int initialSoldiers;
	int atk;

	// ★ 新規追加：スキルと士気システム
	int morale = 100;           // 士気（0〜100）
	int def = 50;               // 防御力
	OfficerSkill skill;         // 武将スキル
	bool hasSkill = false;      // スキルを持っているか

	// バフ・デバフ効果
	double atkMultiplier = 1.0; // 攻撃力倍率
	double defMultiplier = 1.0; // 防御力倍率
	int buffDuration = 0;       // バフ持続ターン数

	bool alive;
	bool acted;
	bool isPlayer;

	// アニメーション用
	double damageTimer = 0.0;
	int lastDamage = 0;
	Vec2 drawPos;
	double attackTimer = 0.0;

	// ★ スキル付きコンストラクタ
	Unit(String n, Side s, Point p, int count = 1000)
		: name(n), side(s), pos(p)
		, soldiers(count), initialSoldiers(count)
		, atk(100), alive(true), acted(false)
		, drawPos(p.x, p.y)
	{
		isPlayer = (side == Side::Player);
	}

	// ★ スキル付きコンストラクタ
	Unit(String n, Side s, Point p, int count, const OfficerSkill& officerSkill)
		: name(n), side(s), pos(p)
		, soldiers(count), initialSoldiers(count)
		, atk(100), alive(true), acted(false)
		, drawPos(p.x, p.y)
		, skill(officerSkill)
		, hasSkill(true)
	{
		isPlayer = (side == Side::Player);
	}

	void Update(double dt)
	{
		if (damageTimer > 0) damageTimer -= dt;
		if (attackTimer > 0) attackTimer -= dt;
		Vec2 target(static_cast<double>(pos.x), static_cast<double>(pos.y));
		drawPos = drawPos.lerp(target, 10.0 * dt);
	}

	void ApplyDamage(int dmg)
	{
		soldiers = Max(0, soldiers - dmg);
		lastDamage = dmg;
		damageTimer = 1.0;
		if (soldiers <= 0) alive = false;
	}

	// ★ 士気を下げる
	void ReduceMorale(int amount)
	{
		morale = Max(0, morale - amount);
		if (morale < 20 && RandomBool(0.3))
		{
			int desertCount = static_cast<int>(soldiers * 0.1);
			soldiers = Max(0, soldiers - desertCount);
			if (soldiers <= 0) alive = false;
		}
	}

	// ★ 士気を上げる
	void IncreaseMorale(int amount)
	{
		morale = Min(100, morale + amount);
	}

	// ★ スキル使用
	bool UseSkill(Array<Unit>& allUnits, int myIndex)
	{
		if (!hasSkill || !skill.CanUse()) return false;
		skill.Use();

		switch (skill.type)
		{
		case OfficerSkill::SkillType::Damage:
			for (int i = 0; i < allUnits.size(); ++i)
			{
				if (i == myIndex || allUnits[i].isPlayer == isPlayer || !allUnits[i].alive) continue;
				int damage = static_cast<int>(atk * skill.damageMultiplier);
				allUnits[i].ApplyDamage(damage);
				allUnits[i].ReduceMorale(-skill.effectValue);
				if (skill.name != U"蛇矛猛撃" && skill.name != U"赤壁の炎") break;
			}
			break;
		case OfficerSkill::SkillType::Buff:
			for (auto& unit : allUnits)
			{
				if (unit.isPlayer == isPlayer && unit.alive)
				{
					unit.atkMultiplier += skill.effectValue / 100.0;
					unit.defMultiplier += skill.effectValue / 100.0;
					unit.buffDuration = skill.effectDuration;
					unit.IncreaseMorale(skill.effectValue / 2);
				}
			}
			break;
		case OfficerSkill::SkillType::Debuff:
			for (auto& unit : allUnits)
			{
				if (unit.isPlayer != isPlayer && unit.alive)
				{
					unit.atkMultiplier -= abs(skill.effectValue) / 100.0;
					unit.ReduceMorale(abs(skill.effectValue));
				}
			}
			break;
		default:
			break;
		}
		return true;
	}

	// ★ バフ更新
	void UpdateBuffs()
	{
		if (buffDuration > 0)
		{
			buffDuration--;
			if (buffDuration == 0)
			{
				atkMultiplier = 1.0;
				defMultiplier = 1.0;
			}
		}
		if (hasSkill) skill.AdvanceCooldown();
	}

	// ★ 実効攻撃力
	int GetEffectiveAtk() const
	{
		return static_cast<int>(atk * atkMultiplier * (morale / 100.0));
	}

	// ★ 実効防御力
	int GetEffectiveDef() const
	{
		return static_cast<int>(def * defMultiplier * (morale / 100.0));
	}

	void draw(int cellSize, Point offset = Point(0, 0)) const
	{
		if (!alive) return;

		Vec2 basePos(offset.x + drawPos.x * cellSize, offset.y + drawPos.y * cellSize);

		// ダメージを受けた時の振動
		if (damageTimer > 0)
		{
			double shake = damageTimer * 5.0;
			basePos.x += RandomClosed(-shake, shake);
		}

		Color factionColor = isPlayer ? Palette::Blue : Palette::Red;
		Color darkColor = isPlayer ? ColorF(0.2, 0.3, 0.6) : ColorF(0.6, 0.2, 0.2);

		if (acted)
		{
			factionColor = factionColor.lerp(Palette::Gray, 0.5);
			darkColor = darkColor.lerp(Palette::Gray, 0.5);
		}

		double time = Scene::Time();

		// 旗
		{
			Vec2 flagPos = basePos.movedBy(cellSize * 0.6, -cellSize * 0.3);
			double wave = sin(time * 3 + drawPos.x) * 3;

			Line(flagPos, flagPos.movedBy(0, -cellSize * 0.6)).draw(2, ColorF(0.4, 0.3, 0.2));

			Quad(
				flagPos.movedBy(0, -cellSize * 0.6),
				flagPos.movedBy(cellSize * 0.35 + wave, -cellSize * 0.55),
				flagPos.movedBy(cellSize * 0.35 + wave * 0.7, -cellSize * 0.35),
				flagPos.movedBy(0, -cellSize * 0.4)
			).draw(factionColor);

			Quad(
				flagPos.movedBy(0, -cellSize * 0.6),
				flagPos.movedBy(cellSize * 0.35 + wave, -cellSize * 0.55),
				flagPos.movedBy(cellSize * 0.35 + wave * 0.7, -cellSize * 0.35),
				flagPos.movedBy(0, -cellSize * 0.4)
			).drawFrame(1, darkColor);
		}

		// ユニット本体の影
		Circle(basePos.movedBy(cellSize * 0.5 + 3, cellSize * 0.5 + 3), cellSize * 0.35)
			.draw(ColorF(0, 0, 0, 0.4));

		// ユニット本体
		{
			Vec2 center = basePos.movedBy(cellSize * 0.5, cellSize * 0.5);

			Circle(center, cellSize * 0.35).draw(darkColor);
			Circle(center, cellSize * 0.3).draw(factionColor);
			Circle(center.movedBy(-cellSize * 0.1, -cellSize * 0.1), cellSize * 0.15)
				.draw(ColorF(1, 1, 1, 0.3));

			RectF(Arg::center(center.movedBy(0, -cellSize * 0.25)), cellSize * 0.4, cellSize * 0.15)
				.draw(darkColor.lerp(Palette::Gold, 0.3));
		}

		// 名前プレート
		{
			Vec2 namePos = basePos.movedBy(cellSize * 0.5, cellSize * 0.2);

			auto nameBounds = FontAsset(U"small")(name).region(namePos);
			RectF nameRect = nameBounds.stretched(5, 2);

			nameRect.draw(ColorF(0, 0, 0, 0.7));
			nameRect.drawFrame(1, factionColor);

			FontAsset(U"small")(name).drawAt(namePos, Palette::White);
		}

		// HPバー
		{
			double rate = static_cast<double>(soldiers) / initialSoldiers;
			double barWidth = cellSize - 10.0;
			double barHeight = 8.0;
			Vec2 barPos = basePos.movedBy(5, cellSize - barHeight - 8);

			RectF(barPos.movedBy(1, 1), barWidth, barHeight).draw(ColorF(0, 0, 0, 0.4));
			RectF(barPos, barWidth, barHeight).draw(ColorF(0.2, 0.2, 0.2));
			RectF(barPos, barWidth, barHeight).drawFrame(1, ColorF(0.5, 0.5, 0.5));

			Color barColor = ColorF(0.2, 0.8, 0.2);
			if (rate < 0.5) barColor = ColorF(0.9, 0.7, 0.2);
			if (rate < 0.25) barColor = ColorF(0.9, 0.2, 0.2);

			RectF hpBar(barPos, barWidth * rate, barHeight);
			hpBar.draw(Arg::left = barColor, Arg::right = barColor.lerp(Palette::Black, 0.3));
			RectF(barPos, barWidth * rate, barHeight * 0.4).draw(ColorF(1, 1, 1, 0.3));

			FontAsset(U"small")(soldiers).drawAt(barPos.movedBy(barWidth * 0.5, barHeight * 0.5), Palette::White);
		}

		// ★ 士気バー
		{
			double moraleRate = morale / 100.0;
			double barWidth = cellSize - 10.0;
			double barHeight = 6.0;
			Vec2 barPos = basePos.movedBy(5, cellSize - barHeight - 2);

			RectF(barPos, barWidth, barHeight).draw(ColorF(0.1, 0.1, 0.1));

			Color moraleColor = ColorF(1.0, 0.8, 0.2);
			if (moraleRate < 0.5) moraleColor = ColorF(0.9, 0.5, 0.2);
			if (moraleRate < 0.3) moraleColor = ColorF(0.9, 0.2, 0.2);

			RectF(barPos, barWidth * moraleRate, barHeight).draw(moraleColor);
			RectF(barPos, barWidth, barHeight).drawFrame(1, ColorF(0.3, 0.3, 0.3));
		}

		// ★ スキルアイコン
		if (hasSkill)
		{
			Vec2 iconPos = basePos.movedBy(cellSize - 25, 5);
			double iconSize = 20;

			Circle(iconPos.movedBy(iconSize * 0.5, iconSize * 0.5), iconSize * 0.5)
				.draw(skill.CanUse() ? ColorF(0.2, 0.6, 0.9) : ColorF(0.3, 0.3, 0.3));

			String icon = U"⚔";
			if (skill.type == OfficerSkill::SkillType::Buff) icon = U"↑";
			if (skill.type == OfficerSkill::SkillType::Debuff) icon = U"↓";

			FontAsset(U"small")(icon).drawAt(iconPos.movedBy(iconSize * 0.5, iconSize * 0.5), Palette::White);

			if (!skill.CanUse())
			{
				FontAsset(U"small")(skill.currentCooldown)
					.drawAt(iconPos.movedBy(iconSize * 0.5, iconSize * 0.5), ColorF(1, 0.3, 0.3));
			}
		}
		// ダメージ表示
		if (damageTimer > 0)
		{
			double t = damageTimer;
			double alpha = Min(t * 2.0, 1.0);
			double y = basePos.y - 30 - (1.0 - t) * 50;

			Vec2 damagePos(basePos.x + cellSize * 0.5, y);

			for (int layer = 3; layer > 0; --layer)
			{
				FontAsset(U"huge")(lastDamage).drawAt(
					damagePos.movedBy(layer, layer),
					ColorF(0, 0, 0, alpha * 0.4)
				);
			}

			FontAsset(U"huge")(lastDamage).drawAt(
				damagePos.movedBy(1, 1),
				ColorF(0.8, 0.2, 0.0, alpha)
			);
			FontAsset(U"huge")(lastDamage).drawAt(
				damagePos,
				ColorF(1.0, 0.9, 0.2, alpha)
			);

			{
				ScopedRenderStates2D blend(BlendState::Additive);
				for (int ring = 0; ring < 3; ++ring)
				{
					double radius = (1.0 - t) * 80 + ring * 15;
					Circle(basePos.movedBy(cellSize * 0.5, cellSize * 0.5), radius)
						.drawFrame(2, ColorF(1, 0.5, 0.2, alpha * (1.0 - ring * 0.3)));
				}
			}
		}

		// 攻撃エフェクト
		if (attackTimer > 0)
		{
			double t = attackTimer;
			Vec2 center = basePos.movedBy(cellSize * 0.5, cellSize * 0.5);

			ScopedRenderStates2D blend(BlendState::Additive);

			for (int i = 0; i < 5; ++i)
			{
				double angle = t * Math::TwoPi * 2 + i * 0.3;
				double dist = cellSize * 0.5;
				Vec2 swordPos = center + Vec2(Cos(angle), Sin(angle)) * dist;

				Line(center, swordPos).draw(3, ColorF(1, 1, 0.5, t * 0.7));
			}

			Circle(center, cellSize * 0.6 * (1.0 - t)).drawFrame(3, ColorF(1, 0.8, 0.3, t));
		}
	}

	void StartAttackAnimation()
	{
		attackTimer = 0.3;
	}
};
