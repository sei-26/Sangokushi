#pragma once
#include <Siv3D.hpp>

struct Unit {
	String name;
	int soldiers = 100;
	int atk = 10;
	int x = 0, y = 0;
	bool isPlayer = true;
	bool alive = true;

	// スムーズ移動用
	Vec2 pos;
	Vec2 targetPos;
	double moveSpeed = 8.0;

	Unit() = default;

	Unit(const String& n, int px, int py, bool player)
		: name(n)
		, soldiers(100)
		, atk(10)
		, x(px)
		, y(py)
		, isPlayer(player)
		, alive(true)
		, pos(Vec2(px, py))
		, targetPos(Vec2(px, py))
	{
	}


	void Update(double dt) {
		if (pos.distanceFrom(targetPos) > 0.001) {
			Vec2 dir = (targetPos - pos).normalized();
			pos += dir * moveSpeed * dt;
			if (pos.distanceFrom(targetPos) < 0.05)
				pos = targetPos;
		}
	}

	void Draw(int tileSize = 32) const {
		const Vec2 center = {
			pos.x * tileSize + tileSize / 2.0,
			pos.y * tileSize + tileSize / 2.0
		};

		const Color col = isPlayer ? Palette::Red : Palette::Blue;
		Circle(center, tileSize * 0.35).draw(col);
		FontAsset(U"small")(name).drawAt(center.x, center.y - tileSize * 0.4, Palette::White);

		// 兵数バー
		const double rate = Clamp(soldiers / 100.0, 0.0, 1.0);
		RectF(center.x - tileSize * 0.3, center.y - tileSize * 0.7, tileSize * 0.6, 4).draw(ColorF(0.2));
		RectF(center.x - tileSize * 0.3, center.y - tileSize * 0.7, tileSize * 0.6 * rate, 4)
			.draw(ColorF(1.0, 0.3, 0.3));
	}
};
