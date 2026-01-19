#pragma once
#include <Siv3D.hpp>

enum class Side { Player, Enemy };

struct Unit
{
	String name;
	Side side;
	Point pos;

	int soldiers;
	int initialSoldiers;
	int atk;

	bool alive;
	bool acted;
	bool isPlayer;

	// アニメーション用
	double damageTimer = 0.0;
	int lastDamage = 0;
	Vec2 drawPos;

	Unit(String n, Side s, Point p, int count = 1000)
		: name(n), side(s), pos(p)
		, soldiers(count), initialSoldiers(count)
		, atk(100), alive(true), acted(false)
		, drawPos(p.x, p.y)
	{
		isPlayer = (side == Side::Player);
	}

	void Update(double dt)
	{
		if (damageTimer > 0) damageTimer -= dt;
		Vec2 target(pos.x, pos.y);
		drawPos = drawPos.lerp(target, 10.0 * dt);
	}

	void ApplyDamage(int dmg)
	{
		soldiers = Max(0, soldiers - dmg);
		lastDamage = dmg;
		damageTimer = 1.0;
		if (soldiers <= 0) alive = false;
	}

	// ★ 修正：offset 引数を追加し、デフォルト値を設定
	// これで引数が1個でも2個でもエラーになりません
	void draw(int cellSize, Point offset = Point(0, 0)) const
	{
		if (!alive) return;

		// 座標計算（オフセットを加算）
		double px = offset.x + drawPos.x * cellSize;
		double py = offset.y + drawPos.y * cellSize;

		// 色
		Color color = isPlayer ? Palette::Blue : Palette::Red;
		if (acted) color = Palette::Gray;

		// 本体
		RectF(px + 4, py + 4, cellSize - 8, cellSize - 8).draw(color);
		RectF(px + 4, py + 4, cellSize - 8, cellSize - 8).drawFrame(2, Palette::White);

		// 名前
		FontAsset(U"Default")(name).drawAt(px + cellSize / 2, py + cellSize / 2 - 10);

		// バー
		double rate = (double)soldiers / initialSoldiers;
		RectF(px + 5, py + cellSize - 15, cellSize - 10, 6).draw(Palette::Black);
		RectF(px + 5, py + cellSize - 15, (cellSize - 10) * rate, 6).draw(Palette::Limegreen);
		FontAsset(U"Default")(soldiers).drawAt(px + cellSize / 2, py + cellSize - 5, Palette::White);

		// ダメージ
		if (damageTimer > 0)
		{
			FontAsset(U"title")(lastDamage).drawAt(px + cellSize / 2, py - 20, Palette::Yellow);
		}
	}
};
