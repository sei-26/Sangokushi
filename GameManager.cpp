#include "GameManager.hpp"
#include "Map.hpp"
#include "Unit.hpp"

GameManager::GameManager()
{
	map = Map(45, 30, 64);
	map.FitToScreen(Scene::Width(), Scene::Height() - 50, 50);

	pkMode = false;
	pkTimer = 0.0;
	turnCount = 1;
	battleFinished = false;
	selectedIndex = -1;
}

// =====================================================
// ★ SRPGマップ表示＋8PK式じわじわ戦闘を開始
// =====================================================
void GameManager::InitializeBattle(const Officer& attacker,
								   const Officer& defender,
								   int attackerSoldiers,
								   int defenderSoldiers)
{
	units.clear();

	// 攻撃側ユニット
	Unit atk(attacker.name, 3, 7, true);
	atk.soldiers = attackerSoldiers;
	atk.atk = attacker.war;
	units << atk;

	// 防衛側ユニット
	Unit def(defender.name, map.width - 4, 7, false);
	def.soldiers = defenderSoldiers;
	def.atk = defender.war;
	units << def;

	// マップ占有設定
	for (auto& u : units)
		map.At(u.x, u.y).setOccupied(true);

	// 8PK戦闘モード開始
	pkMode = true;
	pkTimer = 0.0;

	battleFinished = false;
	selectedIndex = -1;
	turnCount = 1;
}

// =====================================================
// ★ 8PK式じわじわ戦闘処理
// =====================================================
void GameManager::UpdatePKBattle()
{
	if (!pkMode) return;
	if (battleFinished) return;
	if (units.size() < 2) return;

	Unit& atk = units[0];
	Unit& def = units[1];

	// ---- 0.7秒に1回攻撃ラウンド ----
	pkTimer += Scene::DeltaTime();
	if (pkTimer < 0.7)
		return;

	pkTimer = 0.0;

	// ------------------------------
	// 8PK式ダメージ
	// ------------------------------
	auto calcDamage = [&](int atkStat, int targetSoldiers)
		{
			double rate = 0.03 + (atkStat - 50) * 0.001;
			rate = Clamp(rate, 0.01, 0.05);    // 1% ～ 5%
			return Max(1, int(targetSoldiers * rate));
		};

	// --- 攻撃側 → 防衛側 ---
	{
		int dmg = calcDamage(atk.atk, def.soldiers);
		def.ApplyDamage(dmg);

		if (!def.alive)
		{
			battleFinished = true;
			turnCount++;
			return;
		}
	}

	// --- 防衛側 → 攻撃側 ---
	{
		int dmg = calcDamage(def.atk, atk.soldiers);
		atk.ApplyDamage(dmg);

		if (!atk.alive)
		{
			battleFinished = true;
			turnCount++;
			return;
		}
	}

	turnCount++;
}

// =====================================================
// ★ SRPGの見た目を描画（マップ＋ユニット）
// =====================================================
void GameManager::DrawSRPGBoard() const
{
	// マップ描画
	map.Draw();

	// ユニット描画
	for (const auto& u : units)
	{
		if (u.alive)
			u.Draw(map.tileSize);
	}

	// 選択枠（使わないが一応残す）
	if (selectedIndex >= 0 &&
		selectedIndex < units.size() &&
		units[selectedIndex].alive)
	{
		const auto& s = units[selectedIndex];
		RectF(s.x * map.tileSize, s.y * map.tileSize,
			  map.tileSize, map.tileSize)
			.drawFrame(3, Palette::Yellow);
	}
}

// =====================================================
// ★ 8PK式UI（HPバー）
// =====================================================
void GameManager::DrawPKUI() const
{
	if (units.size() < 2) return;

	const Unit& atk = units[0];
	const Unit& def = units[1];

	int maxS = Max(atk.soldiers, def.soldiers);
	maxS = Max(1, maxS);

	double atkP = atk.soldiers / double(maxS);
	double defP = def.soldiers / double(maxS);

	const double W = 300;

	// 攻撃側
	FontAsset(U"medium")(U"攻撃側: {}"_fmt(atk.name)).drawAt(300, 40);
	RectF(150, 70, W, 20).draw(ColorF(0.2));
	RectF(150, 70, W * atkP, 20).draw(Palette::Red);

	// 防衛側
	FontAsset(U"medium")(U"防衛側: {}"_fmt(def.name)).drawAt(1000, 40);
	RectF(850, 70, W, 20).draw(ColorF(0.2));
	RectF(850, 70, W * defP, 20).draw(Palette::Skyblue);

	// ターン数
	FontAsset(U"small")(U"ターン {}"_fmt(turnCount))
		.drawAt(Scene::Width() / 2, 110);
}
