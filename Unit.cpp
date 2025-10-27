#include "Unit.hpp"

void Unit::Draw(int tileSize) const
{
	const Vec2 center = Vec2(x * tileSize + tileSize / 2.0, y * tileSize + tileSize / 2.0);


	Color col = isPlayer ? Palette::Red : Palette::Blue;
	if (acted) col = ColorF(col, 0.5);

	if (damageTimer > 0)
	{
		double alpha = Math::Clamp(1.0 - damageTimer * 3.0, 0.0, 1.0);
		col = ColorF(1.0, 0.3, 0.3, alpha + 0.4);

	}

	Circle(center, tileSize * 0.35).draw(col);

	const double hpRatio = Max(0.0, soldiers / 10000.0);
	const double barW = tileSize * 0.7;
	const double barH = 6.0;
	const double barX = center.x - barW / 2;
	const double barY = center.y + tileSize * 0.6;

	RectF(barX, barY, barW, barH).draw(ColorF(0.2, 0.2, 0.2, 0.7));
	RectF(barX, barY, barW * hpRatio, barH).draw(isPlayer ? Palette::Orange : Palette::Skyblue);


	if (damageTimer > 0 && lastDamage > 0)
	{
		const double yOffset = -tileSize * (0.8 + damageTimer * 1.2);
		const double alpha = Math::Clamp(1.0 - damageTimer * 2.0, 0.0, 1.0);
		FontAsset(U"small")(U"-" + Format(lastDamage)).drawAt(center.x, center.y + yOffset, ColorF(1.0, 0.2, 0.2, alpha));
	}
}
