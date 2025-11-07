#include "GameManager.hpp"

GameManager::GameManager()
	: map(20, 15)
{
	map.FitToScreen(Scene::Width(), Scene::Height(), uiHeight);

	// ※ Unit は soldiers 管理前提
	units =
	{
		Unit(U"劉備",   2,  2, true),
		Unit(U"関羽",   4,  2, true),
		Unit(U"張飛",   6,  2, true),
		Unit(U"敵将A", 10,  8, false),
		Unit(U"敵将B", 12,  9, false),
	};

	for (auto& u : units)
		map.At(u.x, u.y).setOccupied(true);
}

void GameManager::Update()
{
		// 👇 まず全ユニットのpos更新
		const double dt = Scene::DeltaTime();
		for (auto& u : units)
			if (u.alive) u.Update(dt);

		// その後にターン処理
		if (playerTurn) UpdatePlayerTurn();
		else            UpdateEnemyTurn();



	map.Draw();

	// 移動範囲ハイライト（プレイヤー選択時）
	if (playerTurn && selectedIndex >= 0 && selectedIndex < units.size())
	{
		const Unit& s = units[selectedIndex];
		for (int dy = -5; dy <= 5; ++dy)
		{
			for (int dx = -5; dx <= 5; ++dx)
			{
				const int tx = s.x + dx;
				const int ty = s.y + dy;
				if (!map.InBounds(tx, ty)) continue;
				if (std::abs(dx) + std::abs(dy) > 5) continue;
				if (map.At(tx, ty).occupied()) continue;

				RectF(tx * map.tileSize, ty * map.tileSize, map.tileSize, map.tileSize)
					.draw(ColorF(0.2, 0.5, 1.0, 0.3));
			}
		}
	}

	// ユニット描画
	for (const auto& u : units)
		if (u.alive)
			u.Draw(map.tileSize);

	// 選択枠
	if (playerTurn && selectedIndex >= 0 && selectedIndex < units.size())
	{
		const Unit& s = units[selectedIndex];
		RectF(s.x * map.tileSize, s.y * map.tileSize, map.tileSize, map.tileSize)
			.drawFrame(3, Palette::Yellow);
	}

	DrawUI();
}

void GameManager::UpdatePlayerTurn()
{
	const Point cursorTile{ Cursor::Pos().x / map.tileSize, Cursor::Pos().y / map.tileSize };

	// 左クリック：プレイヤーのユニット選択
	if (MouseL.down())
	{
		selectedIndex = -1;
		for (int i = 0; i < units.size(); ++i)
		{
			const auto& u = units[i];
			if (!u.alive || !u.isPlayer) continue;
			if (u.x == cursorTile.x && u.y == cursorTile.y)
			{
				selectedIndex = i;
				break;
			}
		}
	}

	// 右クリック：移動＋隣接攻撃
	if (MouseR.down() && selectedIndex >= 0)
	{
		if (!map.InBounds(cursorTile.x, cursorTile.y)) return;

		Unit& actor = units[selectedIndex];
		const int dist = std::abs(actor.x - cursorTile.x) + std::abs(actor.y - cursorTile.y);

		if (dist <= 5 && !map.At(cursorTile.x, cursorTile.y).occupied())
		{
			// 移動
			// 移動
			map.At(actor.x, actor.y).setOccupied(false);
			actor.x = cursorTile.x;
			actor.y = cursorTile.y;
			map.At(actor.x, actor.y).setOccupied(true);

			// 👇 スムーズ移動用の目標座標を更新
			actor.targetPos = Vec2(actor.x, actor.y);


			// 隣接敵へ攻撃
			for (auto& enemy : units)
			{
				if (!enemy.alive) continue;
				if (enemy.isPlayer == actor.isPlayer) continue;

				const int md = std::abs(enemy.x - actor.x) + std::abs(enemy.y - actor.y);
				if (md == 1)
				{
					enemy.soldiers -= actor.atk;

					// 簡易ダメージ演出
					RectF(enemy.x * map.tileSize, enemy.y * map.tileSize, map.tileSize, map.tileSize)
						.draw(ColorF(1.0, 0.2, 0.2, 0.5));

					if (enemy.soldiers <= 0)
					{
						enemy.alive = false;
						map.At(enemy.x, enemy.y).setOccupied(false);
					}
				}
			}

			// プレイヤーの手番終了 → 敵ターンへ
			selectedIndex = -1;
			playerTurn = false;
		}
	}
}

void GameManager::UpdateEnemyTurn()
{
	// 🔹 static を維持する変数（ターンごとの間隔管理）
	static double aiTimer = 0.0;
	static size_t enemyIndex = 0;

	aiTimer += Scene::DeltaTime();

	// 0.4秒に1体ずつ行動
	if (aiTimer < 0.4) return;
	aiTimer = 0.0;

	// 🔹 敵全滅チェック
	bool anyEnemyAlive = std::any_of(units.begin(), units.end(),
		[](const Unit& u) { return !u.isPlayer && u.alive; });
	if (!anyEnemyAlive)
	{
		// 全滅時 → プレイヤーターンへ
		playerTurn = true;
		enemyIndex = 0;
		return;
	}

	// 🔹 敵がもう全員行動済みならターン終了
	if (enemyIndex >= units.size())
	{
		enemyIndex = 0;   // ← ← ← ★毎ターンのリセットが肝！
		playerTurn = true;
		++turnCount;
		return;
	}

	// 敵を順番に探す
	for (; enemyIndex < units.size(); ++enemyIndex)
	{
		Unit& enemy = units[enemyIndex];
		if (!enemy.alive || enemy.isPlayer) continue;

		// --- プレイヤー生存確認 ---
		bool anyPlayerAlive = std::any_of(units.begin(), units.end(),
			[](const Unit& u) { return u.isPlayer && u.alive; });
		if (!anyPlayerAlive)
		{
			playerTurn = true;
			enemyIndex = 0;
			return;
		}

		// --- 最も近いプレイヤーを探す ---
		Unit* target = nullptr;
		int bestDist = 9999;

		for (auto& player : units)
		{
			if (!player.alive || !player.isPlayer) continue;
			int d = std::abs(player.x - enemy.x) + std::abs(player.y - enemy.y);
			if (d < bestDist)
			{
				bestDist = d;
				target = &player;
			}
		}

		if (!target)
			continue;

		// --- 隣接してたら攻撃 ---
		if (bestDist == 1)
		{
			target->soldiers -= enemy.atk;
			RectF(target->x * map.tileSize, target->y * map.tileSize, map.tileSize, map.tileSize)
				.draw(ColorF(1.0, 0.3, 0.3, 0.6));

			if (target->soldiers <= 0)
			{
				target->alive = false;
				map.At(target->x, target->y).setOccupied(false);
			}

			++enemyIndex;
			return;  // 一度行動したらこのフレーム終了
		}

		// --- 1歩近づく ---
		int dx = 0, dy = 0;
		if (target->x > enemy.x) dx = 1;
		else if (target->x < enemy.x) dx = -1;
		else if (target->y > enemy.y) dy = 1;
		else if (target->y < enemy.y) dy = -1;

		int nx = enemy.x + dx;
		int ny = enemy.y + dy;

		if (map.InBounds(nx, ny) && !map.At(nx, ny).occupied())
		{
			map.At(enemy.x, enemy.y).setOccupied(false);
			enemy.x = nx;
			enemy.y = ny;
			map.At(enemy.x, enemy.y).setOccupied(true);

			// 👇 スムーズ移動
			enemy.targetPos = Vec2(enemy.x, enemy.y);
		}


		++enemyIndex;
		return;  // 一体動かしたら終了（テンポ調整）
	}

	// 🔹 最後まで行ったらリセット
	enemyIndex = 0;
	playerTurn = true;
	++turnCount;
}





void GameManager::DrawUI()
{
	Rect(0, Scene::Height() - uiHeight, Scene::Width(), uiHeight)
		.draw(Color(0, 0, 0, 160));

	FontAsset(U"small")(Format(U"ターン: {}  [{}]", turnCount,
		playerTurn ? U"プレイヤーターン" : U"敵ターン"))
		.draw(8, Scene::Height() - uiHeight + 8, Palette::White);

	if (playerTurn && selectedIndex >= 0 && selectedIndex < units.size())
	{
		const Unit& s = units[selectedIndex];
		FontAsset(U"small")(Format(U"選択中: {}  兵数:{}  座標({}, {})",
			s.name, s.soldiers, s.x, s.y))
			.draw(8, Scene::Height() - uiHeight + 32, Palette::Yellow);
	}
}

bool GameManager::IsBattleFinished() const
{
	const bool allEnemiesDead = std::none_of(units.begin(), units.end(),
		[](const Unit& u) { return (!u.isPlayer && u.alive); });

	const bool allPlayersDead = std::none_of(units.begin(), units.end(),
		[](const Unit& u) { return (u.isPlayer && u.alive); });

	return (allEnemiesDead || allPlayersDead);
}

BattleResult GameManager::GetBattleResult() const
{
	const bool allEnemiesDead = std::none_of(units.begin(), units.end(),
		[](const Unit& u) { return (!u.isPlayer && u.alive); });

	const bool allPlayersDead = std::none_of(units.begin(), units.end(),
		[](const Unit& u) { return (u.isPlayer && u.alive); });

	if (allEnemiesDead) return BattleResult::Victory;
	if (allPlayersDead) return BattleResult::Defeat;
	return BattleResult::None;
}
