#include "Unit.hpp"

void Unit::Draw(int tileSize) const
{
	// ===============================
	// 🧭 1. スムーズな描画位置
	// ===============================
	const Vec2 center = pos * tileSize + Vec2(tileSize / 2.0, tileSize / 2.0);

	// ===============================
	// 🎨 2. カラー設定（プレイヤー／敵）
	// ===============================
	Color col = isPlayer ? Palette::Red : Palette::Blue;

	// 行動済みなら半透明
	if (acted)
		col = ColorF(col, 0.5);

	// 攻撃を受けた瞬間、赤く点滅
	if (damageTimer > 0)
	{
		double flash = Math::Clamp(1.0 - damageTimer * 2.0, 0.0, 1.0);
		col = ColorF(1.0, 0.3, 0.3, 0.7 + flash * 0.3);
	}

	// ユニット本体描画
	Circle(center, tileSize * 0.35).draw(col);

	// ===============================
	// ❤️ 3. 兵士数バー（HPバー）
	// ===============================
	double ratio = Clamp((double)soldiers / 100.0, 0.0, 1.0);
	const double barW = tileSize * 0.7;
	const double barH = 6.0;
	const double barX = center.x - barW / 2;
	const double barY = center.y - tileSize * 0.55;

	// バー背景
	RectF(barX, barY, barW, barH).draw(ColorF(0.2, 0.2, 0.2, 0.7));
	// 残量バー
	RectF(barX, barY, barW * ratio, barH).draw(isPlayer ? Palette::Orange : Palette::Skyblue);

	// ===============================
	// 🧍 4. ユニット名＋兵数
	// ===============================
	FontAsset(U"small")(Format(name, U"(", soldiers, U")"))
		.drawAt(center.x, center.y - tileSize * 0.8, Palette::White);

	// ===============================
	// 💥 5. ダメージ数値ポップ表示
	// ===============================
	if (damageTimer > 0 && lastDamage > 0)
	{
		const double yOffset = -tileSize * (0.7 + damageTimer * 1.5);
		const double alpha = Math::Clamp(1.0 - damageTimer * 2.0, 0.0, 1.0);
		const Vec2 dmgPos = center + Vec2(0, yOffset);
		FontAsset(U"small")(U"-" + Format(lastDamage))
			.drawAt(dmgPos, ColorF(1.0, 0.2, 0.2, alpha));
	}
}
