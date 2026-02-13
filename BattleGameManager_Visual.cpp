#include "BattleGameManager.hpp"
#include <queue>

void BattleGameManager::InitializeBattle(
	const CityData& playerCity,
	const CityData& enemyCity,
	const Officer& selectedLeader,
	int selectedSoldiers,
	bool playerIsAtk)
{
	map = Map(15, 10, 60);
	map.FitToScreen(Scene::Width(), Scene::Height(), 0);
	units.clear();

	isPlayerAttacker = playerIsAtk;

	int playerMainForce = Max(selectedSoldiers, 100);
	int playerSubForce = Max(selectedSoldiers / 3, 50);

	units.push_back(Unit(selectedLeader.name, Side::Player, Point(1, 4), playerMainForce));
	units.push_back(Unit(U"副将", Side::Player, Point(1, 5), playerSubForce));

	int enemySoldiers = Max(enemyCity.troops, 100);

	String enemyLeaderName = U"敵将";
	if (!enemyCity.officers.isEmpty())
	{
		enemyLeaderName = enemyCity.officers[0].name;
	}
	else
	{
		enemyLeaderName = enemyCity.owner + U"兵";
	}

	int enemyMainForce = enemySoldiers;
	int enemySubForce = Max(enemySoldiers / 2, 50);

	units.push_back(Unit(enemyLeaderName, Side::Enemy, Point(12, 4), enemyMainForce));
	units.push_back(Unit(U"敵副将", Side::Enemy, Point(12, 6), enemySubForce));

	Print << U"[戦力配置] プレイヤー=" << playerMainForce << U"+" << playerSubForce
		<< U", 敵=" << enemyMainForce << U"+" << enemySubForce;

	phase = TurnPhase::PlayerTurn;
	actingIndex = 0;
	moveRange.clear();
}

bool BattleGameManager::IsBattleFinished() const
{
	return phase == TurnPhase::BattleEnd;
}

void BattleGameManager::Update()
{
	for (auto& u : units) u.Update(Scene::DeltaTime());

	if (phase == TurnPhase::BattleEnd) return;

	if (PlayerWon())
	{
		phase = TurnPhase::BattleEnd;
		return;
	}

	if (PlayerLost())
	{
		phase = TurnPhase::BattleEnd;
		return;
	}

	switch (phase)
	{
	case TurnPhase::PlayerTurn: UpdatePlayerTurn(); break;
	case TurnPhase::EnemyTurn:  UpdateEnemyTurn();  break;
	}
}

void BattleGameManager::UpdatePlayerTurn()
{
	bool allActed = true;
	for (auto& u : units) if (u.isPlayer && u.alive && !u.acted) allActed = false;

	if (allActed)
	{
		for (auto& u : units) u.acted = false;
		actingIndex = 0;
		moveRange.clear();
		phase = TurnPhase::EnemyTurn;
		return;
	}

	while (actingIndex < units.size())
	{
		Unit& u = units[actingIndex];
		if (u.isPlayer && u.alive && !u.acted) break;
		actingIndex++;
	}

	if (actingIndex >= units.size()) return;

	if (moveRange.isEmpty()) CalculateMoveRange();

	MoveUnit();
	TryAttack();
}

void BattleGameManager::UpdateEnemyTurn()
{
	if (actingIndex >= units.size()) actingIndex = 0;

	bool allActed = true;
	for (auto& u : units) if (!u.isPlayer && u.alive && !u.acted) allActed = false;

	if (allActed)
	{
		for (auto& u : units) u.acted = false;
		actingIndex = 0;
		moveRange.clear();
		phase = TurnPhase::PlayerTurn;
		return;
	}

	while (actingIndex < units.size())
	{
		Unit& u = units[actingIndex];
		if (!u.isPlayer && u.alive && !u.acted) break;
		actingIndex++;
	}

	if (actingIndex >= units.size()) { actingIndex = 0; return; }

	Unit& enemy = units[actingIndex];
	int tgt = FindClosestEnemyIndex(actingIndex);
	if (tgt >= 0) EnemyAction(tgt);

	enemy.acted = true;
	actingIndex++;
}

void BattleGameManager::Draw() const
{
	double time = Scene::Time();

	// =================================================================
	// 🎨 戦場の背景（超派手版）
	// =================================================================
	Scene::SetBackground(ColorF{ 0.15, 0.18, 0.12 });

	// 🔥 戦場の煙と炎
	{
		ScopedRenderStates2D blend(BlendState::Additive);

		// 煙の層
		for (int i = 0; i < 15; ++i)
		{
			double smokeTime = time * 0.08 + i * 0.5;
			double x = Math::Fmod(i * 200 + smokeTime * 25, Scene::Width() + 400) - 200;
			double y = 80 + sin(smokeTime * 1.5) * 40;
			double size = 180 + sin(smokeTime * 2) * 30;

			Circle(x, y, size).draw(ColorF(0.3, 0.25, 0.2, 0.08));
		}

		// 遠くの火災
		for (int i = 0; i < 8; ++i)
		{
			double fireTime = time * 2 + i;
			double x = i * 240.0 + sin(fireTime) * 30;
			double y = 50 + sin(fireTime * 3) * 15;

			Circle(x, y, 40).draw(ColorF(1.0, 0.5, 0.2, 0.15));
			Circle(x, y - 20, 25).draw(ColorF(1.0, 0.7, 0.3, 0.12));
		}
	}

	// ⚡ 戦場の光（稲妻風）
	if (static_cast<int>(time * 0.5) % 7 == 0)
	{
		ScopedRenderStates2D blend(BlendState::Additive);
		double flash = sin(time * 20) * 0.5 + 0.5;
		Scene::Rect().draw(ColorF(1, 1, 0.9, 0.05 * flash));
	}

	const Transformer2D t2d{ Mat3x2::Translate(50, 50) };

	// =================================================================
	// 🗺️ マップ描画（戦場風）
	// =================================================================
	for (int y = 0; y < map.height; ++y)
	{
		for (int x = 0; x < map.width; ++x)
		{
			RectF tile(x * map.tileSize, y * map.tileSize, map.tileSize, map.tileSize);
			Color tileColor = map.tiles[y][x].getColor();

			// タイルの影
			tile.movedBy(1, 1).draw(ColorF(0, 0, 0, 0.15));

			// タイル本体
			tile.draw(Arg::top = tileColor, Arg::bottom = tileColor.lerp(Palette::Black, 0.3));

			// 枠線
			tile.drawFrame(1, ColorF(0, 0, 0, 0.4));

			// 地形ごとのテクスチャ
			int type = map.tiles[y][x].type;
			if (type == 1) // 森
			{
				for (int tree = 0; tree < 3; ++tree)
				{
					double treeX = tile.x + (tree + 1) * map.tileSize * 0.25;
					double treeY = tile.y + map.tileSize * 0.5;
					Triangle(treeX, treeY - 15, treeX - 8, treeY + 5, treeX + 8, treeY + 5)
						.draw(ColorF(0.15, 0.3, 0.15, 0.4));
				}
			}
			else if (type == 2) // 山
			{
				Vec2 peak = tile.center().movedBy(0, -10);
				Triangle(peak, peak.movedBy(-15, 20), peak.movedBy(15, 20))
					.draw(ColorF(0.3, 0.25, 0.2, 0.4));
			}
			else if (type == 3) // 城
			{
				RectF(tile.x + 10, tile.y + 10, map.tileSize - 20, map.tileSize - 20)
					.draw(ColorF(0.4, 0.4, 0.4, 0.3));
			}
		}
	}

	// =================================================================
	// 🎯 移動範囲の表示（超派手版）
	// =================================================================
	for (auto& p : moveRange)
	{
		RectF tile(p.x * map.tileSize, p.y * map.tileSize, map.tileSize, map.tileSize);

		// 🌟 脈動する光
		double pulse = 0.6 + sin(time * 5) * 0.3;

		// 外側の発光
		{
			ScopedRenderStates2D blend(BlendState::Additive);
			tile.stretched(4).draw(ColorF(0.3, 0.6, 1.0, 0.2 * pulse));
		}

		// 半透明の青い範囲
		tile.stretched(-4).draw(ColorF(0.2, 0.5, 1.0, 0.18 * pulse));

		// 魔法陣風のフレーム
		tile.stretched(-4).drawFrame(3, ColorF(0.5, 0.8, 1.0, 0.6 * pulse));

		// ✨ 四隅の輝き
		for (auto corner : { tile.tl(), tile.tr(), tile.bl(), tile.br() })
		{
			Circle(corner, 5 + sin(time * 6) * 2).draw(ColorF(0.7, 0.9, 1.0, 0.8 * pulse));
		}

		// 🌀 中心の魔法陣
		{
			ScopedRenderStates2D blend(BlendState::Additive);
			double rotation = time * 2;
			Vec2 center = tile.center();

			for (int ring = 0; ring < 2; ++ring)
			{
				double radius = 15 + ring * 8;
				Circle(center, radius).drawFrame(1.5, ColorF(0.6, 0.8, 1.0, 0.3 * pulse));
			}
		}
	}

	// =================================================================
	// 👥 ユニット描画
	// =================================================================

	// 選択中のユニットに光の柱
	if (actingIndex < units.size() && phase == TurnPhase::PlayerTurn)
	{
		const Unit& selected = units[actingIndex];
		if (selected.alive && selected.isPlayer)
		{
			Vec2 unitCenter(selected.drawPos.x * map.tileSize + map.tileSize / 2,
							selected.drawPos.y * map.tileSize + map.tileSize / 2);

			ScopedRenderStates2D blend(BlendState::Additive);

			// 🌟 光の柱
			for (int layer = 0; layer < 3; ++layer)
			{
				double height = 800 - layer * 150;
				double width = 40 - layer * 8;
				double alpha = 0.15 - layer * 0.04;

				RectF(Arg::center(unitCenter.x, unitCenter.y - height / 2), width, height)
					.draw(ColorF(0.3, 0.6, 1.0, alpha));
			}

			// 地面の光の輪
			for (int ring = 0; ring < 4; ++ring)
			{
				double radius = 30 + ring * 12 + sin(time * 4 + ring) * 5;
				Circle(unitCenter, radius).drawFrame(2, ColorF(0.5, 0.8, 1.0, 0.4 - ring * 0.08));
			}
		}
	}

	for (auto& u : units)
	{
		if (u.alive) u.draw(map.tileSize);
	}

	// =================================================================
	// 🎮 UI表示（超豪華版）
	// =================================================================
	{
		RectF turnPanel(Scene::Width() / 2 - 250, 10, 500, 90);

		// パネルの背後の光
		{
			ScopedRenderStates2D blend(BlendState::Additive);
			turnPanel.stretched(8).draw(ColorF(1, 1, 0.8, 0.15));
		}

		// パネルの影
		turnPanel.movedBy(4, 4).draw(ColorF(0, 0, 0, 0.6));

		// パネル本体
		Color panelColor = (phase == TurnPhase::PlayerTurn) ?
			ColorF(0.25, 0.45, 0.85) : ColorF(0.85, 0.35, 0.35);

		turnPanel.draw(Arg::top = panelColor, Arg::bottom = panelColor.lerp(Palette::Black, 0.5));
		turnPanel.drawFrame(5, ColorF(1, 1, 0.9));

		// 装飾的な角（宝石風）
		for (auto corner : { turnPanel.tl(), turnPanel.tr(), turnPanel.bl(), turnPanel.br() })
		{
			Circle(corner, 12).draw(ColorF(1, 1, 0.9));
			Circle(corner, 10).draw(panelColor);
			Circle(corner, 6).draw(ColorF(1, 1, 1, 0.8 + sin(time * 4) * 0.2));
		}

		// ターンテキスト
		String turnText = (phase == TurnPhase::PlayerTurn) ? U"味方ターン" : U"敵ターン";
		Color textColor = Palette::White;

		if (phase == TurnPhase::BattleEnd)
		{
			if (PlayerWon())
			{
				turnText = U"🎉 大勝利！ 🎉";
				panelColor = ColorF(0.3, 0.85, 0.4);
			}
			else
			{
				turnText = U"敗北...";
				panelColor = ColorF(0.5, 0.5, 0.5);
			}
		}

		// テキストの多重影
		for (int layer = 3; layer > 0; --layer)
		{
			FontAsset(U"huge")(turnText).drawAt(
				turnPanel.center().movedBy(layer, layer),
				ColorF(0, 0, 0, 0.3)
			);
		}
		FontAsset(U"huge")(turnText).drawAt(turnPanel.center(), textColor);

		// テキストの輝き
		{
			ScopedRenderStates2D blend(BlendState::Additive);
			FontAsset(U"huge")(turnText).drawAt(turnPanel.center(), ColorF(1, 1, 1, 0.3));
		}
	}

	// 操作ヒント
	if (phase == TurnPhase::PlayerTurn && !moveRange.isEmpty())
	{
		RectF hintPanel(Scene::Width() / 2 - 300, Scene::Height() - 60, 600, 50);
		hintPanel.draw(ColorF(0, 0, 0, 0.8));
		hintPanel.drawFrame(2, ColorF(1, 1, 0.9));
		FontAsset(U"menu")(U"[左クリック] 移動 ／ 隣接で自動攻撃")
			.drawAt(hintPanel.center(), ColorF(1, 1, 0.9));
	}

	// =================================================================
	// 🎆 勝利/敗北時の超派手演出
	// =================================================================
	if (phase == TurnPhase::BattleEnd)
	{
		double endTime = 3.0; // 演出の長さ
		double t = Min(endTime, time); // 実際は戦闘終了からの時間を使う

		if (PlayerWon())
		{
			// ✨ 勝利の光の祝福
			ScopedRenderStates2D blend(BlendState::Additive);

			// 金色の光の粒子が降り注ぐ
			for (int i = 0; i < 150; ++i)
			{
				double particleTime = time + i * 0.1;
				double x = Math::Fmod(i * 37.5 + sin(particleTime) * 100, Scene::Width());
				double y = Math::Fmod(particleTime * 80 + i * 15, Scene::Height());

				Circle(x, y, 4 + sin(particleTime * 5) * 2)
					.draw(ColorF(1.0, 0.9, 0.5, 0.6));
			}

			// 🌟 爆発する光の輪
			for (int ring = 0; ring < 8; ++ring)
			{
				double radius = ring * 100.0 + sin(time * 3 + ring) * 30;
				Circle(Scene::Center(), radius)
					.drawFrame(3, ColorF(1, 1, 0.7, 0.3 - ring * 0.03));
			}

			// ✨ 回転する星
			for (int star = 0; star < 12; ++star)
			{
				double angle = time * 2 + star * Math::TwoPi / 12;
				double dist = 200 + sin(time * 3 + star) * 50;
				Vec2 starPos = Scene::Center() + Vec2(Cos(angle), Sin(angle)) * dist;

				Circle(starPos, 12).draw(ColorF(1, 1, 0.8, 0.8));
				Circle(starPos, 8).draw(ColorF(1, 1, 1, 1));
			}
		}
		else
		{
			// 💀 敗北の暗闇
			double darknessAlpha = Min(0.6, t / endTime * 0.6);
			Scene::Rect().draw(ColorF(0, 0, 0, darknessAlpha));

			// 赤い煙
			ScopedRenderStates2D blend(BlendState::Additive);
			for (int i = 0; i < 30; ++i)
			{
				double smokeTime = time * 0.5 + i;
				double x = Math::Fmod(i * 60 + smokeTime * 20, Scene::Width());
				double y = Scene::Height() - Math::Fmod(smokeTime * 40, Scene::Height());

				Circle(x, y, 60).draw(ColorF(0.5, 0.1, 0.1, 0.1));
			}
		}
	}
}

void BattleGameManager::CalculateMoveRange()
{
	moveRange.clear();
	if (actingIndex >= units.size()) return;

	const Unit& u = units[actingIndex];
	Point start = u.pos;
	std::queue<std::pair<Point, int>> q;
	q.push({ start, 0 });
	HashSet<Point> visited;
	visited.insert(start);
	const int MAX_MOVE = 3;

	while (!q.empty())
	{
		auto [p, dist] = q.front(); q.pop();
		bool occupied = false;
		for (const auto& other : units)
		{
			if (other.alive && other.pos == p && other.pos != start) { occupied = true; break; }
		}
		if (!occupied) moveRange << p;
		if (dist >= MAX_MOVE) continue;

		for (auto& nb : GetNeighbors(p))
		{
			if (!visited.contains(nb) && CanMoveTo(nb.x, nb.y))
			{
				visited.insert(nb); q.push({ nb, dist + 1 });
			}
		}
	}
}

void BattleGameManager::MoveUnit()
{
	if (!MouseL.down()) return;
	Point click = (Cursor::Pos() - Point(50, 50)) / map.tileSize;
	if (!moveRange.contains(click)) return;

	Unit& u = units[actingIndex];
	if (u.pos == click) return;

	u.pos = click;
	moveRange.clear();
}

void BattleGameManager::TryAttack()
{
	if (actingIndex >= units.size()) return;
	Unit& atk = units[actingIndex];
	for (auto& def : units)
	{
		if (!def.alive) continue;
		if (def.isPlayer == atk.isPlayer) continue;
		if (AreAdjacent(atk, def))
		{
			ResolveCombat(atk, def);
			atk.acted = true;
			moveRange.clear();
			actingIndex++;
			return;
		}
	}
}

void BattleGameManager::EnemyAction(int targetIdx)
{
	Unit& enemy = units[actingIndex];
	Unit& target = units[targetIdx];
	if (AreAdjacent(enemy, target)) { ResolveCombat(enemy, target); return; }

	auto path = FindPath(enemy.pos, target.pos);
	if (path.size() > 1)
	{
		Point next = path[1];
		bool occupied = false;
		for (const auto& u : units) if (u.alive && u.pos == next) occupied = true;
		if (!occupied) enemy.pos = next;
	}
}

bool BattleGameManager::AreAdjacent(const Unit& a, const Unit& b) const
{
	return (Abs(a.pos.x - b.pos.x) + Abs(a.pos.y - b.pos.y)) == 1;
}

void BattleGameManager::ResolveCombat(Unit& attacker, Unit& defender)
{
	attacker.StartAttackAnimation();

	double defBonus = map.At(defender.pos.x, defender.pos.y).defenseBonus();
	int baseDmg = attacker.atk * 2 + Random(0, 20);
	int finalDmg = static_cast<int>(baseDmg / defBonus);
	defender.ApplyDamage(Max(5, finalDmg));
}

int BattleGameManager::FindClosestEnemyIndex(int i) const
{
	const Unit& me = units[i];
	int best = -1; int bestScore = 999999;
	for (int k = 0; k < units.size(); ++k)
	{
		const Unit& u = units[k];
		if (u.isPlayer == me.isPlayer || !u.alive) continue;
		int dist = Abs(u.pos.x - me.pos.x) + Abs(u.pos.y - me.pos.y);
		int score = dist * 10 + (u.soldiers / 100);
		if (score < bestScore) { bestScore = score; best = k; }
	}
	return best;
}

bool BattleGameManager::CanMoveTo(int x, int y) const
{
	if (!map.InBounds(x, y)) return false;
	for (auto& u : units) if (u.alive && u.pos.x == x && u.pos.y == y) return false;
	return true;
}

Array<Point> BattleGameManager::GetNeighbors(const Point& p) const
{
	Array<Point> r;
	r << Point(p.x + 1, p.y); r << Point(p.x - 1, p.y);
	r << Point(p.x, p.y + 1); r << Point(p.x, p.y - 1);
	return r;
}

Array<Point> BattleGameManager::FindPath(const Point& start, const Point& goal) const
{
	std::queue<Point> q; HashSet<Point> visited; HashTable<Point, Point> parent;
	q.push(start); visited.insert(start); bool found = false;
	while (!q.empty())
	{
		Point cur = q.front(); q.pop();
		if (cur == goal) { found = true; break; }
		for (const auto& nb : GetNeighbors(cur))
		{
			if (!visited.contains(nb))
			{
				if (nb == goal || CanMoveTo(nb.x, nb.y))
				{
					visited.insert(nb); parent[nb] = cur; q.push(nb);
				}
			}
		}
	}
	Array<Point> path; if (!found) return path;
	Point cur = goal; while (cur != start) { path << cur; cur = parent[cur]; }
	path << start; path.reverse(); return path;
}

bool BattleGameManager::PlayerWon() const
{
	for (auto& u : units) if (!u.isPlayer && u.alive) return false;
	return true;
}

bool BattleGameManager::PlayerLost() const
{
	for (auto& u : units) if (u.isPlayer && u.alive) return false;
	return true;
}

void BattleGameManager::ApplyBattleResult(CityData& atkCity, CityData& defCity)
{
	int playerSurvivors = 0;
	int enemySurvivors = 0;
	for (const auto& u : units)
	{
		if (u.alive)
		{
			if (u.isPlayer) playerSurvivors += u.soldiers;
			else            enemySurvivors += u.soldiers;
		}
	}

	if (PlayerWon())
	{
		if (isPlayerAttacker)
		{
			atkCity.troops = playerSurvivors;
			defCity.owner = atkCity.owner;
			defCity.troops = 500;
			defCity.order = Max(0, defCity.order - 50);
		}
		else
		{
			defCity.troops = playerSurvivors;
			atkCity.troops = enemySurvivors;
		}
	}
	else
	{
		if (isPlayerAttacker)
		{
			atkCity.troops = playerSurvivors;
			defCity.troops = enemySurvivors;
		}
		else
		{
			atkCity.troops = enemySurvivors;
			defCity.troops = 500;
			defCity.owner = atkCity.owner;
			defCity.order = Max(0, defCity.order - 50);
		}
	}
}
