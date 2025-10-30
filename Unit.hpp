#pragma once
#include <Siv3D.hpp>

class Unit
{
public:
	String name;
	int soldiers = 100;
	int atk = 15;
	int x = 0;
	int y = 0;
	int moveRange = 3;
	bool isPlayer = true;
	bool alive = true;
	bool acted = false;

	// スムーズ移動用
	Vec2 pos;
	Array<Point> movePath;
	bool isMoving = false;
	double moveTimer = 0.0;
	double moveSpeed = 4.0;

	// 攻撃演出
	int lastDamage = 0;
	double damageTimer = 0.0;

	Unit() = default;
	Unit(const String& n, int px, int py, bool player)
		: name(n), x(px), y(py), isPlayer(player)
	{
		pos = Vec2(px, py);
	}

	void Draw(int tileSize = 32) const;

};
