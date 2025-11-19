#include "BattleGameManager.hpp"
#include <queue>

//==========================================================
// 戦場初期化
//==========================================================
void BattleGameManager::InitializeBattle(
	const CityData& fromCity,
	const CityData& targetCity,
	const Officer& selectedLeader,
	int selectedSoldiers)
{
	map = Map(20, 15, 32);
	map.FitToScreen(Scene::Width(), Scene::Height(), 0);
	units.clear();

	//--------------------------------------
	// 攻撃側（3ユニット）
	//--------------------------------------
	{
		Unit u(selectedLeader.name, 2, 3, true); // ★味方
		u.soldiers = selectedSoldiers;
		u.initialSoldiers = selectedSoldiers;
		u.atk = selectedLeader.war;
		units << u;

		int added = 0;
		for (const auto& off : fromCity.officers)
		{
			if (off.name == selectedLeader.name) continue;
			if (added >= 2) break;

			Unit sub(off.name, 2, 5 + added * 2, true); // ★味方
			sub.soldiers = Max(100, fromCity.troops / 10);
			sub.initialSoldiers = sub.soldiers;
			sub.atk = off.war;
			units << sub;
			added++;
		}
	}

	//--------------------------------------
	// 防衛側（3ユニット）
	//--------------------------------------
	{
		int added = 0;
		for (const auto& off : targetCity.officers)
		{
			if (added >= 3) break;

			Unit u(off.name, map.width - 3, 4 + added * 2, false); // ★敵
			u.soldiers = Max(100, targetCity.troops / 3);
			u.initialSoldiers = u.soldiers;
			u.atk = off.war;
			units << u;
			added++;
		}
	}

	// 行動初期化
	for (auto& u : units)
	{
		u.acted = false;
		u.alive = true;
	}

	phase = TurnPhase::PlayerTurn;
	actingIndex = 0;
	moveRange.clear();
}


//==========================================================
// メイン更新
//==========================================================
void BattleGameManager::Update()
{
	if (phase == TurnPhase::BattleEnd) return;

	if (phase == TurnPhase::PlayerTurn)
		UpdatePlayerTurn();
	else
		UpdateEnemyTurn();

	if (PlayerWon())
		phase = TurnPhase::BattleEnd;
}

//==========================================================
// 描画
//==========================================================
void BattleGameManager::Draw() const
{
	map.Draw();

	// 移動範囲
	for (auto& p : moveRange)
	{
		RectF r(
			p.x * map.tileSize,
			p.y * map.tileSize,
			map.tileSize,
			map.tileSize
		);
		r.stretched(-4).draw(ColorF(0.2, 0.5, 1.0, 0.25));
		r.drawFrame(3, ColorF(0.5, 0.8, 1.0, 0.7));
	}

	// ユニット
	for (auto& u : units)
		if (u.alive)
			u.Draw(map.tileSize);

	String turnText =
		(phase == TurnPhase::PlayerTurn) ? U"味方ターン" :
		(phase == TurnPhase::EnemyTurn) ? U"敵ターン" :
		U"戦闘終了";

	FontAsset(U"medium")(turnText).draw(20, 10);
}

//==========================================================
// プレイヤーターン
//==========================================================
void BattleGameManager::UpdatePlayerTurn()
{
    if (actingIndex >= units.size())
    {
        phase = TurnPhase::EnemyTurn;
        actingIndex = 0;
        moveRange.clear();
        return;
    }

    Unit& u = units[actingIndex];

    if (!u.isPlayer || !u.alive)
    {
        actingIndex++;
        return;
    }

    if (u.acted)
    {
        actingIndex++;
        return;
    }

    // ★ 行動開始時に一度だけ移動範囲生成
    if (moveRange.isEmpty())
    {
        CalculateMoveRange();
    }

    MoveUnit();
    TryAttack();
}

//==========================================================
// 選択ユニット
//==========================================================
void BattleGameManager::SelectUnit()
{
	// actingIndex のユニットが対象なので何もしない
}

//==========================================================
// BFSで移動範囲
//==========================================================
void BattleGameManager::CalculateMoveRange()
{
	moveRange.clear();

	const Unit& u = units[actingIndex];
	Point start(u.x, u.y);

	std::queue<std::pair<Point, int>> q;
	q.push({ start, 0 });

	HashSet<Point> visited;
	visited.insert(start);

	const int MAX_MOVE = 3;

	while (!q.empty())
	{
		auto [p, dist] = q.front();
		q.pop();

		// ★占有チェック（自ユニットのスタート地点以外）
		bool occupied = false;
		for (const auto& other : units)
		{
			if (other.alive &&
				!(other.x == start.x && other.y == start.y) &&
				other.x == p.x && other.y == p.y)
			{
				occupied = true;
				break;
			}
		}

		// ★占有されていなければ表示
		if (!occupied)
		{
			moveRange << p;
		}

		if (dist >= MAX_MOVE) continue;

		for (auto& nb : GetNeighbors(p))
		{
			if (!visited.contains(nb) && CanMoveTo(nb.x, nb.y))
			{
				visited.insert(nb);
				q.push({ nb, dist + 1 });
			}
		}
	}
}


//==========================================================
// クリックで移動
//==========================================================
void BattleGameManager::MoveUnit()
{
	if (!MouseL.down()) return;

	Point click(Cursor::Pos().x / map.tileSize,
		Cursor::Pos().y / map.tileSize);

	if (!moveRange.contains(click)) return;

	Unit& u = units[actingIndex];
	auto path = FindPath(Point(u.x, u.y), click);
	if (path.isEmpty()) return;

	u.x = click.x;
	u.y = click.y;
	u.pos = Vec2(click.x, click.y);
	u.acted = true;

	moveRange.clear();
}

//==========================================================
// 攻撃
//==========================================================
void BattleGameManager::TryAttack()
{
	Unit& atk = units[actingIndex];

	for (auto& def : units)
	{
		if (!def.alive) continue;
		if (def.isPlayer == atk.isPlayer) continue;

		if (AreAdjacent(atk, def))
		{
			ResolveCombat(atk, def);
			atk.acted = true;
			break;
		}
	}

	actingIndex++;
}

//==========================================================
// 敵ターン
//==========================================================
void BattleGameManager::UpdateEnemyTurn()
{
	if (actingIndex >= units.size())
	{
		for (auto& u : units)
			u.acted = false;

		actingIndex = 0;
		phase = TurnPhase::PlayerTurn;
		moveRange.clear();
		return;
	}

	Unit& enemy = units[actingIndex];

	if (enemy.isPlayer || !enemy.alive)
	{
		actingIndex++;
		return;
	}

	int tgt = FindClosestEnemyIndex(actingIndex);

	if (tgt >= 0)
		EnemyAction(tgt);

	actingIndex++;
}

//==========================================================
// 敵AI
//==========================================================
void BattleGameManager::EnemyAction(int targetIdx)
{
	Unit& enemy = units[actingIndex];
	Unit& target = units[targetIdx];

	if (AreAdjacent(enemy, target))
	{
		ResolveCombat(enemy, target);
		return;
	}

	auto path = FindPath(Point(enemy.x, enemy.y), Point(target.x, target.y));
	if (path.size() > 1)
	{
		Point next = path[1];
		enemy.x = next.x;
		enemy.y = next.y;
		enemy.pos = Vec2(enemy.x, enemy.y);
	}
}

//==========================================================
// 隣接
//==========================================================
bool BattleGameManager::AreAdjacent(const Unit& a, const Unit& b) const
{
	return (Abs(a.x - b.x) + Abs(a.y - b.y)) == 1;
}

//==========================================================
// 戦闘計算
//==========================================================
void BattleGameManager::ResolveCombat(Unit& attacker, Unit& defender)
{
	int dmg = Max(10, attacker.atk * 2 + Random(0, 20));
	defender.ApplyDamage(dmg);

	// 攻撃エフェクト
	attacker.damageTimer = 0.2;
	attacker.lastDamage = 0;
}

//==========================================================
// ターゲット選択（最弱・最短）
//==========================================================
int BattleGameManager::FindClosestEnemyIndex(int i) const
{
	const Unit& me = units[i];

	int best = -1;
	int bestScore = 999999;

	for (int k = 0; k < units.size(); ++k)
	{
		const Unit& u = units[k];
		if (u.isPlayer == me.isPlayer) continue;
		if (!u.alive) continue;

		int dist = Abs(u.x - me.x) + Abs(u.y - me.y);
		int hpFactor = u.soldiers / 10;

		int score = dist * 10 + hpFactor;

		if (score < bestScore)
		{
			bestScore = score;
			best = k;
		}
	}
	return best;
}

//==========================================================
// 通行可
//==========================================================
bool BattleGameManager::CanMoveTo(int x, int y) const
{
	if (!map.InBounds(x, y)) return false;

	for (auto& u : units)
		if (u.alive && u.x == x && u.y == y)
			return false;

	return true;
}

//==========================================================
// 隣接タイル
//==========================================================
Array<Point> BattleGameManager::GetNeighbors(const Point& p) const
{
	Array<Point> r;
	r << Point(p.x + 1, p.y);
	r << Point(p.x - 1, p.y);
	r << Point(p.x, p.y + 1);
	r << Point(p.x, p.y - 1);
	return r;
}

//==========================================================
// BFS 経路探索
//==========================================================
Array<Point> BattleGameManager::FindPath(const Point& start, const Point& goal) const
{
	std::queue<Point> q;
	HashSet<Point> visited;
	HashTable<Point, Point> parent;

	q.push(start);
	visited.insert(start);

	bool found = false;

	while (!q.empty())
	{
		Point cur = q.front();
		q.pop();

		if (cur == goal)
		{
			found = true;
			break;
		}

		for (const auto& nb : GetNeighbors(cur))
		{
			if (!visited.contains(nb) && CanMoveTo(nb.x, nb.y))
			{
				visited.insert(nb);
				parent[nb] = cur;
				q.push(nb);
			}
		}
	}

	Array<Point> path;
	if (!found) return path;

	Point cur = goal;
	while (cur != start)
	{
		path << cur;
		cur = parent[cur];
	}
	path << start;

	path.reverse();
	return path;
}

//==========================================================
// 勝利判定
//==========================================================
bool BattleGameManager::PlayerWon() const
{
	for (auto& u : units)
		if (!u.isPlayer && u.alive)
			return false;
	return true;
}
void BattleGameManager::ApplyBattleResult(CityData& atkCity, CityData& defCity)
{
	int atkLoss = 0;
	int defLoss = 0;

	for (const auto& u : units)
	{
		if (u.isPlayer)
			atkLoss += Max(0, u.initialSoldiers - u.soldiers);
		else
			defLoss += Max(0, u.initialSoldiers - u.soldiers);
	}

	atkCity.troops = Max(0, atkCity.troops - atkLoss);
	defCity.troops = Max(0, defCity.troops - defLoss);

	// ★都市占領
	if (defCity.troops <= 0)
	{
		defCity.owner = atkCity.owner;
	}
}

