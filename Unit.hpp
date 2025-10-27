#pragma once
#include <Siv3D.hpp>

class Unit
{
public:
	String name;
	int soldiers = 10000;
	int atk = 500;
	int x = 0;
	int y = 0;
	int moveRange = 4;
	bool isPlayer = true;
	bool alive = true;
	bool acted = false;

	int lastDamage = 0;
	double damageTimer = 0.0;

	Unit() = default;
	Unit(const String& n, int px, int py, bool player)
		: name(n), x(px), y(py), isPlayer(player) {
	}

	void Draw(int tileSize = 32) const;
};
