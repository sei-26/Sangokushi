#include "GameManager.hpp"
#include "Pathfinding.hpp"

GameManager::GameManager()
	: map(20, 15)
{
	map.FitToScreen(Scene::Width(), Scene::Height(), uiHeight);

	units = {
		Unit(U"劉備", 2, 2, true),
		Unit(U"関羽", 4, 2, true),
		Unit(U"張飛", 6, 2, true),
		Unit(U"敵将A", 10, 8, false),
		Unit(U"敵将B", 12, 9, false)
	};

	for (auto& u : units)
		map.At(u.x, u.y).setOccupied(true);
}

// ========================
// メイン更新
// ========================
void GameManager::Update()
{
	// 各ユニットのダメージエフェクト時間を更新
	for (auto& u : units)
	{
		if (u.damageTimer > 0)
		{
			u.damageTimer += Scene::DeltaTime();
			if (u.damageTimer > 1.0) // 1秒経過でリセット
			{
				u.damageTimer = 0.0;
				u.lastDamage = 0;
			}
		}
	}

	// === 以下は既存の描画・フェーズ処理 ===
	map.Draw();
	for (auto& u : units)
		if (u.alive)
			u.Draw(map.tileSize);

	if (playerTurn)
		UpdatePlayerTurn();
	else
		UpdateEnemyTurn();

	DrawUI();
}


// ========================
// プレイヤーターン
// ========================
void GameManager::UpdatePlayerTurn()
{
	if (AllUnitsActed(true))
	{
		playerTurn = false;
		return;
	}

	// 左クリック：ユニット選択
	if (MouseL.down())
	{
		Point mouse = Cursor::Pos();
		int tx = mouse.x / map.tileSize;
		int ty = mouse.y / map.tileSize;

		selectedIndex = -1;
		for (int i = 0; i < (int)units.size(); ++i)
		{
			if (!units[i].alive || !units[i].isPlayer) continue;
			if (units[i].x == tx && units[i].y == ty)
			{
				if (units[i].acted) continue;
				selectedIndex = i;
				break;
			}
		}
	}

	// 右クリック：移動＋攻撃
	if (MouseR.down() && selectedIndex >= 0)
	{
		Point mouse = Cursor::Pos();
		int tx = mouse.x / map.tileSize;
		int ty = mouse.y / map.tileSize;
		if (!map.InBounds(tx, ty)) return;

		Unit& actor = units[selectedIndex];
		if (actor.acted) return;

		int dist = std::abs(actor.x - tx) + std::abs(actor.y - ty);
		if (dist <= 5 && !map.At(tx, ty).occupied())
		{
			map.At(actor.x, actor.y).setOccupied(false);
			actor.x = tx;
			actor.y = ty;
			map.At(tx, ty).setOccupied(true);

			for (auto& enemy : units)
			{
				if (!enemy.alive || enemy.isPlayer) continue;
				int md = std::abs(enemy.x - actor.x) + std::abs(enemy.y - actor.y);
				if (md == 1)
				{
					enemy.soldiers -= actor.atk;
					RectF(enemy.x * map.tileSize, enemy.y * map.tileSize, map.tileSize, map.tileSize)
						.draw(ColorF(1.0, 0.3, 0.3, 0.6));
					if (enemy.soldiers <= 0)
					{
						enemy.alive = false;
						map.At(enemy.x, enemy.y).setOccupied(false);
					}
				}
			}

			actor.acted = true;
			selectedIndex = -1;
		}
	}
}

// ========================
// 敵ターン（AI行動フェーズ）
// ========================
void GameManager::UpdateEnemyTurn()
{
	static double timer = 0;
	timer += Scene::DeltaTime();

	// 全員行動済み → プレイヤーターンへ
	if (AllUnitsActed(false))
	{
		ResetActions();
		playerTurn = true;
		turnCount++;
		timer = 0;
		return;
	}

	// 敵の行動間隔（テンポ調整）
	if (timer < 0.5) return;
	timer = 0;

	
	auto inBounds = [&](Point p) { return map.InBounds(p.x, p.y); };
	auto passable = [&](Point p) { return map.At(p.x, p.y).isPassable(); };

	// 敵を順番に処理
	for (auto& enemy : units)
	{
		if (!enemy.alive || enemy.isPlayer || enemy.acted) continue;

		
		for (auto& player : units)
		{
			if (!player.alive || !player.isPlayer) continue;
			int md = Abs(player.x - enemy.x) + Abs(player.y - enemy.y);
			if (md == 1)
			{
				player.soldiers -= enemy.atk;
				RectF(player.x * map.tileSize, player.y * map.tileSize, map.tileSize, map.tileSize)
					.draw(ColorF(1.0, 0.25, 0.25, 0.7));

				if (player.soldiers <= 0)
				{
					player.alive = false;
					map.At(player.x, player.y).setOccupied(false);
				}
				enemy.acted = true;
				return;
			}
		}

		
		Unit* target = nullptr;
		int bestScore = 999999;
		for (auto& player : units)
		{
			if (!player.alive || !player.isPlayer) continue;
			int dist = Abs(player.x - enemy.x) + Abs(player.y - enemy.y);
			int score = dist * 10 + player.soldiers;
			if (score < bestScore)
			{
				bestScore = score;
				target = &player;
			}
		}
		if (!target) { enemy.acted = true; continue; }

		
		Array<Point> adjGoals;
		const Point tgt = { target->x, target->y };
		const Point dirs[4] = { {1,0},{-1,0},{0,1},{0,-1} };
		for (auto d : dirs)
		{
			Point g = tgt + d;
			if (!inBounds(g)) continue;
			if (!passable(g)) continue;
			adjGoals << g;
		}
		if (adjGoals.isEmpty()) { enemy.acted = true; continue; }

		
		Array<Point> bestPath;
		int bestLen = 99999;
		for (auto g : adjGoals)
		{
			auto path = AStar(Point{ enemy.x, enemy.y }, g, inBounds, passable);
			if (!path.isEmpty() && path.size() < bestLen)
			{
				bestLen = static_cast<int>(path.size());
				bestPath = std::move(path);
			}
		}

		
		if (!bestPath.isEmpty() && bestPath.size() >= 2)
		{
			int steps = Min(enemy.moveRange, (int)bestPath.size() - 1); // ←移動力反映
			Point next = bestPath[steps];

			if (inBounds(next) && passable(next))
			{
				map.At(enemy.x, enemy.y).setOccupied(false);
				enemy.x = next.x;
				enemy.y = next.y;
				map.At(enemy.x, enemy.y).setOccupied(true);
			}
		}

		
		for (auto& player : units)
		{
			if (!player.alive || !player.isPlayer) continue;
			int md = Abs(player.x - enemy.x) + Abs(player.y - enemy.y);
			if (md == 1)
			{
				player.soldiers -= enemy.atk;
				RectF(player.x * map.tileSize, player.y * map.tileSize, map.tileSize, map.tileSize)
					.draw(ColorF(1.0, 0.25, 0.25, 0.7));

				if (player.soldiers <= 0)
				{
					player.alive = false;
					map.At(player.x, player.y).setOccupied(false);
				}
			}
		}

		enemy.acted = true;
		return; // この敵の行動を終了（テンポ調整）
	}
}


// ========================
// 共通関数
// ========================
bool GameManager::AllUnitsActed(bool playerSide) const
{
	for (const auto& u : units)
		if (u.alive && u.isPlayer == playerSide && !u.acted)
			return false;
	return true;
}

void GameManager::ResetActions()
{
	for (auto& u : units)
		u.acted = false;
}

void GameManager::DrawUI()
{
	Rect(0, Scene::Height() - uiHeight, Scene::Width(), uiHeight)
		.draw(Color(0, 0, 0, 160));

	String turnStr = playerTurn ? U"プレイヤーターン" : U"敵ターン（AI行動中）";
	FontAsset(U"small")(Format(U"ターン: {} [{}]", turnCount, turnStr))
		.draw(8, Scene::Height() - uiHeight + 8, Palette::White);
}
