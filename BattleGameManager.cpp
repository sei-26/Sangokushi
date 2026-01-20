#include "BattleGameManager.hpp"
#include <queue>

// 初期化
void BattleGameManager::InitializeBattle(
	const CityData& fromCity,
	const CityData& targetCity,
	const Officer& selectedLeader,
	int selectedSoldiers)
{
	(void)fromCity; (void)targetCity;

	map = Map(15, 10, 60);
	map.FitToScreen(Scene::Width(), Scene::Height(), 0);
	units.clear();

	// 味方
	units.push_back(Unit(selectedLeader.name, Side::Player, Point(1, 4), selectedSoldiers));
	units.push_back(Unit(U"副将", Side::Player, Point(1, 5), 1000));

	// 敵
	units.push_back(Unit(U"敵将A", Side::Enemy, Point(12, 4), 2000));
	units.push_back(Unit(U"敵将B", Side::Enemy, Point(12, 6), 1500));

	phase = TurnPhase::PlayerTurn;
	actingIndex = 0;
	moveRange.clear();
}

// ★ エラー修正：定義が見つからないと言われていた関数を追加
bool BattleGameManager::IsBattleFinished() const
{
	return phase == TurnPhase::BattleEnd;
}

// 更新
void BattleGameManager::Update()
{
	// 1. アニメーションは常に更新
	for (auto& u : units) u.Update(Scene::DeltaTime());

	// 2. すでに終わっていたら何もしない
	if (phase == TurnPhase::BattleEnd) return;

	// 3. 勝利判定
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

	// 5. ターン処理
	switch (phase)
	{
	case TurnPhase::PlayerTurn: UpdatePlayerTurn(); break;
	case TurnPhase::EnemyTurn:  UpdateEnemyTurn();  break;
	}
}

// プレイヤーターン
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

// 敵ターン
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

// 描画
void BattleGameManager::Draw() const
{
	const Transformer2D t2d{ Mat3x2::Translate(50, 50) };

	map.Draw();

	for (auto& p : moveRange)
	{
		RectF(p.x * map.tileSize, p.y * map.tileSize, map.tileSize, map.tileSize)
			.stretched(-4).draw(ColorF(0.2, 0.5, 1.0, 0.25))
			.drawFrame(3, ColorF(0.5, 0.8, 1.0, 0.7));
	}

	for (auto& u : units)
	{
		// ★ エラー修正：小文字の draw を呼び出す
		if (u.alive) u.draw(map.tileSize);
	}

	String turnText = (phase == TurnPhase::PlayerTurn) ? U"味方ターン" : U"敵ターン";
	if (phase == TurnPhase::BattleEnd)
	{
		if (PlayerWon()) turnText = U"勝利！";
		else turnText = U"敗北...";
	}

	FontAsset(U"medium")(turnText).draw(Scene::Width() / 2 - 100, -40, Palette::White);
}

// ロジック部分
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
// 味方ユニット（isPlayer == true）で、生きている（alive == true）奴が一人もいなければ「負け」
bool BattleGameManager::PlayerLost() const
{
	// 味方が全滅しているか？
	for (auto& u : units) if (u.isPlayer && u.alive) return false;
	return true;
}
void BattleGameManager::ApplyBattleResult(CityData& atkCity, CityData& defCity)
{
	
		int playerSurvivors = 0;
		int EnemySurvivors = 0;

		for (const auto& u : units)
		{
			if (u.alive)
			{
				if (u.isPlayer) playerSurvivors += u.soldiers;
				else  EnemySurvivors += u.soldiers;	
			}
		}
	if (PlayerWon())
		{
		// 攻撃側の勝利
		atkCity.troops = playerSurvivors;
		defCity.owner = atkCity.owner;
		defCity.troops = 500;
		defCity.order = Max(0, defCity.order -50);
		}
	else
	{
		atkCity.troops = playerSurvivors;
		defCity.troops = EnemySurvivors;
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
