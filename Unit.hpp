#pragma once
#include <Siv3D.hpp>

struct Unit
{
	String name;

	int soldiers = 100;         // ★兵力（絶対必要）
	int initialSoldiers = 100;  // 初期兵力
	int atk = 10;

	int x = 0, y = 0;
	bool isPlayer = true;
	bool alive = true;
	bool acted = false;

	// 移動
	Vec2 pos;
	Vec2 targetPos;
	double moveSpeed = 8.0;

	// エフェクト
	double damageTimer = 0.0;
	int lastDamage = 0;

	Unit() = default;

	Unit(const String& n, int px, int py, bool player)
		: name(n),
		x(px), y(py),
		isPlayer(player),
		alive(true),
		acted(false),
		pos(Vec2(px, py)),
		targetPos(Vec2(px, py))
	{
	}

	// 被ダメージ処理
	void ApplyDamage(int amount)
	{
		if (amount <= 0 || !alive) return;

		lastDamage = amount;
		soldiers -= amount;
		if (soldiers < 0) soldiers = 0;

		damageTimer = 0.6;
		if (soldiers <= 0) alive = false;
	}

	void Update(double dt)
	{
		Vec2 d = targetPos - pos;
		double dist = d.length();

		if (dist > 0.001)
		{
			double speed = moveSpeed;
			if (dist < 0.20)
				speed *= (dist / 0.20);

			pos += d.normalized() * speed * dt;
			if ((targetPos - pos).length() < 0.02)
				pos = targetPos;
		}

		if (damageTimer > 0.0)
		{
			damageTimer -= dt;
			if (damageTimer < 0.0) damageTimer = 0.0;
		}
	}

	void Draw(int tileSize = 32) const
	{
		const Vec2 center = pos * tileSize + Vec2(tileSize / 2.0, tileSize / 2.0);

		Color col = isPlayer ? Palette::Red : Palette::Blue;
		if (acted) col = ColorF(col, 0.5);

		Circle(center, tileSize * 0.35).draw(col);

		// HPバー
		double ratio = Math::Clamp(double(soldiers) / 100.0, 0.0, 1.0);

		RectF(center.x - tileSize * 0.35, center.y - tileSize * 0.55,
			tileSize * 0.7, 6).draw(ColorF(0, 0, 0, 0.5));

		RectF(center.x - tileSize * 0.35, center.y - tileSize * 0.55,
			tileSize * 0.7 * ratio, 6)
			.draw(isPlayer ? Palette::Orange : Palette::Skyblue);

		// 名前
		if (FontAsset::IsRegistered(U"small"))
		{
			FontAsset(U"small")(Format(name, U"(", soldiers, U")"))
				.drawAt(center.x, center.y - tileSize * 0.8);
		}
	}
};
