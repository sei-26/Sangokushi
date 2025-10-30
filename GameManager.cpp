#include "GameManager.hpp"
#include "Pathfinding.hpp"
#include "Map.hpp"
#include "Unit.hpp"
#include "Tile.hpp"


// GameManager コンストラクタ
GameManager::GameManager()
	: map(40, 25) // 🟩 サイズ拡大！（旧: 20,15）
{
	map.FitToScreen(Scene::Width(), Scene::Height(), uiHeight);

	units = {
		Unit(U"劉備", 2, 2, true),
		Unit(U"関羽", 4, 2, true),
		Unit(U"張飛", 6, 2, true),
		Unit(U"敵将A", 15, 10, false),
		Unit(U"敵将B", 20, 12, false),
		Unit(U"敵将C", 25, 14, false)
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

	// 🌀 スムーズ移動アニメーション
	bool anyMoving = false;
	for (auto& u : units)
	{
		// 安全チェックを追加
		if (!u.isMoving) continue;
		if (u.movePath.size() < 2)
		{
			u.isMoving = false;
			continue;
		}

		anyMoving = true;

		Vec2 current = Vec2(u.movePath.front());
		Vec2 next = Vec2(u.movePath[1]);

		u.moveTimer += Scene::DeltaTime() * u.moveSpeed;
		u.pos = current.lerp(next, u.moveTimer);

		if (u.moveTimer >= 1.0)
		{
			u.moveTimer = 0.0;
			u.movePath.pop_front();
			u.x = (int)next.x;
			u.y = (int)next.y;
			u.pos = next;

			if (u.movePath.size() < 2)
				u.isMoving = false;
		}

	}

	// === 以下は既存の描画・フェーズ処理 ===
	if (anyMoving)
	{
		map.Draw();
		for (auto& u : units)
			if (u.alive)
				u.Draw(map.tileSize);
		return;
	}

	// === 通常の更新処理 ===
	map.Draw();
	for (auto& u : units)
		if (u.alive)
			u.Draw(map.tileSize);
	for (const auto& p : movableTiles)
	{
		RectF(p.x * map.tileSize, p.y * map.tileSize, map.tileSize, map.tileSize)
			.draw(ColorF(0.2, 0.5, 1.0, 0.35));  // 半透明の青
	}
	for (auto& u : units)
	{
		if (u.alive)
			u.Draw(map.tileSize);
	}

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
	// 全員行動済み → 敵ターンへ
	if (AllUnitsActed(true))
	{
		playerTurn = false;
		movableTiles.clear(); // 🟩 ハイライト消去
		return;
	}

	// === 経路探索補助 ===
	auto inBounds = [&](Point p) { return map.InBounds(p.x, p.y); };
	auto passable = [&](Point p) { return map.At(p.x, p.y).isPassable(); };

	// === 左クリック：ユニット選択 ===
	if (MouseL.down())
	{
		Point mouse = Cursor::Pos();
		int tx = mouse.x / map.tileSize;
		int ty = mouse.y / map.tileSize;

		selectedIndex = -1;
		movableTiles.clear();

		for (int i = 0; i < (int)units.size(); ++i)
		{
			if (!units[i].alive || !units[i].isPlayer) continue;
			if (units[i].x == tx && units[i].y == ty)
			{
				if (units[i].acted) continue;
				selectedIndex = i;

				// 🟩 移動範囲探索
				const int range = units[i].moveRange;
				const Point start = { units[i].x, units[i].y };
				Array<Point> frontier = { start };
				Grid<int> cost(map.width, map.height, -1);
				cost[start.y][start.x] = 0;

				while (!frontier.isEmpty())
				{
					Point cur = frontier.front();
					frontier.pop_front();

					const Point dirs[4] = { {1,0},{-1,0},{0,1},{0,-1} };
					for (auto d : dirs)
					{
						Point nxt = cur + d;
						if (!inBounds(nxt)) continue;
						if (!passable(nxt)) continue;

						int newCost = cost[cur.y][cur.x] + 1;
						if (newCost > range) continue;
						if (cost[nxt.y][nxt.x] == -1 || newCost < cost[nxt.y][nxt.x])
						{
							cost[nxt.y][nxt.x] = newCost;
							frontier << nxt;
							movableTiles << nxt;
						}
					}
				}
				break;
			}
		}
	}

	// === 右クリック：A*移動＋攻撃（既存処理） ===
	if (MouseR.down() && selectedIndex >= 0)
	{
		Point mouse = Cursor::Pos();
		int tx = mouse.x / map.tileSize;
		int ty = mouse.y / map.tileSize;
		if (!map.InBounds(tx, ty)) return;

		Unit& actor = units[selectedIndex];
		if (actor.acted || actor.isMoving) return;

		// 🟩 移動範囲外クリックならキャンセル
		if (!movableTiles.includes(Point{ tx, ty })) return;

		// 経路探索
		auto path = AStar(Point{ actor.x, actor.y }, Point{ tx, ty }, inBounds, passable);
		if (path.isEmpty()) return;

		if (path.size() - 1 > actor.moveRange)
			path.resize(actor.moveRange + 1);

		map.At(actor.x, actor.y).setOccupied(false);
		map.At(path.back().x, path.back().y).setOccupied(true);

		actor.movePath = path;
		actor.isMoving = true;
		actor.moveTimer = 0.0;
		actor.acted = true;
		selectedIndex = -1;
		movableTiles.clear(); // 🟩 ハイライト消去
		return;
	}

	// === 移動完了後の攻撃処理（既存） ===
	for (auto& actor : units)
	{
		if (!actor.isPlayer || actor.isMoving) continue;
		if (actor.acted)
		{
			for (auto& enemy : units)
			{
				if (!enemy.alive || enemy.isPlayer) continue;
				int md = Abs(enemy.x - actor.x) + Abs(enemy.y - actor.y);
				if (md == 1)
				{
					enemy.soldiers -= actor.atk;
					enemy.lastDamage = actor.atk;
					enemy.damageTimer = 0.0;
					if (enemy.soldiers <= 0)
					{
						enemy.soldiers = 0;
						enemy.alive = false;
						map.At(enemy.x, enemy.y).setOccupied(false);
					}
				}
			}
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

	// 敵の行動テンポ（移動間隔）
	if (timer < 0.5) return;
	timer = 0;

	// 経路探索のための関数
	auto inBounds = [&](Point p) { return map.InBounds(p.x, p.y); };
	auto passable = [&](Point p) { return map.At(p.x, p.y).isPassable(); };

	for (auto& enemy : units)
	{
		// === 無効ユニットをスキップ ===
		if (!enemy.alive || enemy.isPlayer || enemy.acted) continue;
		if (enemy.isMoving) return; // 既に移動中なら次のフレームで処理

		//----------------------------------------
		// 1️⃣ 隣接攻撃チェック
		//----------------------------------------
		for (auto& player : units)
		{
			if (!player.alive || !player.isPlayer) continue;
			int md = Abs(player.x - enemy.x) + Abs(player.y - enemy.y);
			if (md == 1)
			{
				player.soldiers -= enemy.atk;
				player.lastDamage = enemy.atk;
				player.damageTimer = 0.0;

				if (player.soldiers <= 0)
				{
					player.soldiers = 0;
					player.alive = false;
					map.At(player.x, player.y).setOccupied(false);
				}

				enemy.acted = true;
				return;
			}
		}

		//----------------------------------------
		// 2️⃣ 攻撃できない → 最も近いプレイヤーを選択
		//----------------------------------------
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

		//----------------------------------------
		// 3️⃣ ターゲット隣接マス候補
		//----------------------------------------
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

		//----------------------------------------
		// 4️⃣ A*経路探索
		//----------------------------------------
		Array<Point> bestPath;
		int bestLen = 999999;
		for (auto g : adjGoals)
		{
			auto path = AStar(Point{ enemy.x, enemy.y }, g, inBounds, passable);
			if (!path.isEmpty() && path.size() < bestLen)
			{
				bestLen = (int)path.size();
				bestPath = std::move(path);
			}
		}

		if (bestPath.isEmpty())
		{
			enemy.acted = true;
			continue;
		}

		//----------------------------------------
		// 5️⃣ 経路が見つかった → 移動準備
		//----------------------------------------
		if (bestPath.size() > enemy.moveRange + 1)
			bestPath.resize(enemy.moveRange + 1);

		// 経路保存・占有予約・アニメーション開始
		map.At(enemy.x, enemy.y).setOccupied(false);
		map.At(bestPath.back().x, bestPath.back().y).setOccupied(true);

		enemy.movePath = bestPath;
		enemy.isMoving = true;
		enemy.moveTimer = 0.0;

		// 行動済みにして移動開始
		enemy.acted = true;
		return; // この敵の行動は移動フェーズへ
	}
}

		// 4️⃣ A*経路探索で最短経*



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
