#include "BattleGameManager.hpp"
#include "OfficerPortrait.hpp"
#include "BattleSystem.hpp"
#include <queue>

// ★ 地形タイプを格納
Array<Array<BattleSystem::Terrain>> g_terrainMap;

void BattleGameManager::InitializeBattle(
	const CityData& playerCity,
	const CityData& enemyCity,
	const Officer& selectedLeader,
	int selectedSoldiers,
	bool playerIsAtk)
{
	// ★ 画面サイズに応じたマップサイズ
	int screenW = Scene::Width();
	int screenH = Scene::Height();

	// マップサイズを画面に合わせて調整（アスペクト比を保つ）
	int cellSize = Min(screenW / 18, screenH / 13);  // セルサイズを計算
	int mapWidth = (screenW - 100) / cellSize;
	int mapHeight = (screenH - 100) / cellSize;

	map = Map(mapWidth, mapHeight, cellSize);
	map.FitToScreen(screenW, screenH, 0);
	units.clear();

	// ★ 地形マップを生成
	g_terrainMap.clear();
	g_terrainMap.resize(mapHeight);
	for (int y = 0; y < mapHeight; ++y)
	{
		g_terrainMap[y].resize(mapWidth);
		for (int x = 0; x < mapWidth; ++x)
		{
			// ランダムに地形を配置
			int r = Random(0, 99);
			if (r < 60)
				g_terrainMap[y][x] = BattleSystem::Terrain::Plains;
			else if (r < 75)
				g_terrainMap[y][x] = BattleSystem::Terrain::Forest;
			else if (r < 88)
				g_terrainMap[y][x] = BattleSystem::Terrain::Mountain;
			else
				g_terrainMap[y][x] = BattleSystem::Terrain::River;
		}
	}

	// ★ 天候をランダム生成
	m_weather = BattleSystem::GenerateRandomWeather();

	Print << U"[戦闘開始] 天候: " << BattleSystem::GetWeatherName(m_weather);

	isPlayerAttacker = playerIsAtk;

	int playerMainForce = Max(selectedSoldiers, 100);
	int playerSubForce = Max(selectedSoldiers / 3, 50);

	// ★ 武将の戦闘力ボーナスを計算
	int leaderCombatBonus = selectedLeader.GetCombatPower() / 10;

	// プレイヤーユニット作成
	units.push_back(Unit(selectedLeader.name, Side::Player, Point(1, 4), playerMainForce));
	units.back().atk += leaderCombatBonus;  // ★ 攻撃力ボーナス付与

	units.push_back(Unit(U"副将", Side::Player, Point(1, 5), playerSubForce));

	int enemySoldiers = Max(enemyCity.troops, 100);

	String enemyLeaderName = U"敵将";
	int enemyLeaderBonus = 5;  // デフォルトボーナス

	if (!enemyCity.officers.isEmpty())
	{
		const Officer& enemyLeader = enemyCity.officers[0];
		enemyLeaderName = enemyLeader.name;
		enemyLeaderBonus = enemyLeader.GetCombatPower() / 10;  // ★ 敵武将のボーナス
	}
	else
	{
		enemyLeaderName = enemyCity.owner + U"兵";
	}

	int enemyMainForce = enemySoldiers;
	int enemySubForce = Max(enemySoldiers / 2, 50);

	units.push_back(Unit(enemyLeaderName, Side::Enemy, Point(12, 4), enemyMainForce));
	units.back().atk += enemyLeaderBonus;  // ★ 敵の攻撃力ボーナス付与

	units.push_back(Unit(U"敵副将", Side::Enemy, Point(12, 6), enemySubForce));

	Print << U"[戦力配置] " << selectedLeader.name << U"(攻+" << leaderCombatBonus << U") vs "
		<< enemyLeaderName << U"(攻+" << enemyLeaderBonus << U")";

	phase = TurnPhase::PlayerTurn;
	actingIndex = 0;
	moveRange.clear();

	// playerCity使用済み（警告回避）
	(void)playerCity;
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
	int alivePlayerCount = 0;
	int actedPlayerCount = 0;

	for (auto& u : units)
	{
		if (u.isPlayer && u.alive)
		{
			alivePlayerCount++;
			if (u.acted) actedPlayerCount++;
			if (!u.acted) allActed = false;
		}
	}

	if (allActed && alivePlayerCount > 0)
	{
		Print << U"[味方ターン終了] " << alivePlayerCount << U"体が行動完了";
		for (auto& u : units) u.acted = false;
		actingIndex = 0;
		moveRange.clear();
		phase = TurnPhase::EnemyTurn;
		return;
	}

	// 味方が全滅している場合
	if (alivePlayerCount == 0)
	{
		Print << U"[味方全滅] 敗北";
		phase = TurnPhase::BattleEnd;
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
	// ★ 敵の行動に間隔を持たせる（0.5秒待機）
	static double lastActionTime = 0.0;
	double currentTime = Scene::Time();

	// ★ まず全敵が行動済みかチェック
	bool allActed = true;
	for (auto& u : units)
	{
		if (!u.isPlayer && u.alive && !u.acted)
		{
			allActed = false;
			break;
		}
	}

	if (allActed)
	{
		// 全員行動済み → 味方ターンへ
		for (auto& u : units) u.acted = false;
		actingIndex = 0;
		moveRange.clear();
		phase = TurnPhase::PlayerTurn;
		lastActionTime = 0.0;  // リセット
		Print << U"[ターン終了] 敵ターン → 味方ターン";
		return;
	}

	// 待機時間チェック
	if (currentTime - lastActionTime < 0.5)
	{
		return;  // まだ待機時間
	}

	// 次に行動する敵を探す
	if (actingIndex >= units.size()) actingIndex = 0;

	while (actingIndex < units.size())
	{
		Unit& u = units[actingIndex];
		if (!u.isPlayer && u.alive && !u.acted) break;
		actingIndex++;
	}

	if (actingIndex >= units.size())
	{
		// 見つからない場合は最初から探す
		actingIndex = 0;
		return;
	}

	Unit& enemy = units[actingIndex];

	// 行動実行
	int tgt = FindClosestEnemyIndex(actingIndex);
	if (tgt >= 0)
	{
		EnemyAction(tgt);
		Print << U"[敵行動] " << enemy.name << U" が行動";
	}

	enemy.acted = true;
	actingIndex++;

	// ★ 行動後に時間を記録
	lastActionTime = currentTime;
}

void BattleGameManager::Draw() const
{
	double time = Scene::Time();

	// ★ 画面サイズを取得
	int screenW = Scene::Width();
	int screenH = Scene::Height();

	// =================================================================
	// 🎨 戦場の背景
	// =================================================================
	Scene::SetBackground(ColorF{ 0.22, 0.26, 0.20 });

	// 大気の霧
	{
		ScopedRenderStates2D blend(BlendState::Additive);
		for (int i = 0; i < 12; ++i)
		{
			double fogTime = time * 0.05 + i;
			double x = Math::Fmod(i * 300.0 + fogTime * 20, Scene::Width() + 200.0) - 100;
			double y = 100 + sin(fogTime * 2) * 30;
			Circle(x, y, 250).draw(ColorF(0.35, 0.40, 0.35, 0.04));
		}
	}

	// 天空の星
	{
		ScopedRenderStates2D blend(BlendState::Additive);
		for (int i = 0; i < 50; ++i)
		{
			double starX = static_cast<double>((i * 73) % Scene::Width());
			double starY = static_cast<double>((i * 41) % 300);
			double twinkle = sin(time * 3 + i) * 0.5 + 0.5;
			Circle(starX, starY, 2).draw(ColorF(1, 1, 0.9, 0.3 * twinkle));
		}
	}

	// ★ マップを中央に配置するためのオフセット計算
	double mapTotalWidth = map.width * map.tileSize;
	double mapTotalHeight = map.height * map.tileSize;
	double offsetX = (screenW - mapTotalWidth) / 2.0;
	double offsetY = (screenH - mapTotalHeight) / 2.0;

	const Transformer2D t2d{ Mat3x2::Translate(offsetX, offsetY) };

	// マップ描画
	for (int y = 0; y < map.height; ++y)
	{
		for (int x = 0; x < map.width; ++x)
		{
			RectF tile(static_cast<double>(x * map.tileSize), static_cast<double>(y * map.tileSize),
					   static_cast<double>(map.tileSize), static_cast<double>(map.tileSize));
			Color tileColor = map.tiles[y][x].getColor();

			tile.movedBy(1, 1).draw(ColorF(0, 0, 0, 0.15));

			// ★ 地形に応じた色
			Color baseColor = Palette::Green;
			if (y < g_terrainMap.size() && x < g_terrainMap[y].size())
			{
				baseColor = BattleSystem::GetTerrainColor(g_terrainMap[y][x]);
			}

			// グラデーション描画
			tile.draw(baseColor.lerp(Palette::Black, 0.25));
			tile.drawFrame(1, ColorF(0, 0, 0, 0.4));

			// ★ 地形アイコン表示
			if (y < g_terrainMap.size() && x < g_terrainMap[y].size())
			{
				auto terrain = g_terrainMap[y][x];

				if (terrain == BattleSystem::Terrain::Forest) // 森
				{
					for (int tree = 0; tree < 3; ++tree)
					{
						double treeX = tile.x + (tree + 1) * map.tileSize * 0.25;
						double treeY = tile.y + map.tileSize * 0.5;
						Triangle(treeX, treeY - 15, treeX - 8, treeY + 5, treeX + 8, treeY + 5)
							.draw(ColorF(0.15, 0.45, 0.15, 0.6));
					}
				}
				else if (terrain == BattleSystem::Terrain::Mountain) // 山
				{
					Vec2 peak = tile.center().movedBy(0, -10);
					Triangle(peak, peak.movedBy(-15, 20), peak.movedBy(15, 20))
						.draw(ColorF(0.45, 0.35, 0.25, 0.7));
				}
				else if (terrain == BattleSystem::Terrain::River) // 川
				{
					// 波線
					for (int wave = 0; wave < 3; ++wave)
					{
						double waveY = tile.y + map.tileSize * (0.3 + wave * 0.2);
						double offset = sin(time * 2 + wave) * 3;
						Line(tile.x + offset, waveY, tile.x + map.tileSize + offset, waveY)
							.draw(2, ColorF(0.6, 0.8, 1.0, 0.5));
					}
				}
			}
		}
	}

	// 移動範囲の表示（魔法陣風）
	for (auto& p : moveRange)
	{
		RectF tile(static_cast<double>(p.x * map.tileSize), static_cast<double>(p.y * map.tileSize),
				   static_cast<double>(map.tileSize), static_cast<double>(map.tileSize));

		double pulse = 0.5 + sin(time * 4) * 0.3;

		// 魔法陣の円
		{
			ScopedRenderStates2D blend(BlendState::Additive);
			Circle(tile.center(), map.tileSize * 0.4).drawFrame(2, ColorF(0.5, 0.8, 1.0, 0.4 * pulse));
			Circle(tile.center(), map.tileSize * 0.3).drawFrame(1, ColorF(0.7, 0.9, 1.0, 0.5 * pulse));

			// 回転する魔法陣
			for (int i = 0; i < 6; ++i)
			{
				double angle = time * 2 + i * Math::TwoPi / 6;
				Vec2 pos = tile.center() + Vec2(Cos(angle), Sin(angle)) * (map.tileSize * 0.25);
				Circle(pos, 3).draw(ColorF(0.8, 0.95, 1.0, 0.6 * pulse));
			}
		}

		tile.stretched(-4).draw(ColorF(0.2, 0.5, 1.0, 0.12 * pulse));
		tile.stretched(-4).drawFrame(3, ColorF(0.5, 0.8, 1.0, 0.6 * pulse));
	}

	// 行動中ユニットの光の柱
	if (actingIndex < units.size() && units[actingIndex].alive && !units[actingIndex].acted)
	{
		const Unit& activeUnit = units[actingIndex];
		Vec2 centerPos(activeUnit.pos.x * map.tileSize + map.tileSize * 0.5,
					   activeUnit.pos.y * map.tileSize + map.tileSize * 0.5);

		ScopedRenderStates2D blend(BlendState::Additive);

		// 光の柱
		for (int layer = 0; layer < 5; ++layer)
		{
			double layerAlpha = (5 - layer) * 0.08;
			double layerWidth = (layer + 1) * 15.0;

			RectF(centerPos.x - layerWidth / 2, 0, layerWidth, centerPos.y)
				.draw(ColorF(activeUnit.isPlayer ? 0.3 : 0.9,
					activeUnit.isPlayer ? 0.5 : 0.3,
					activeUnit.isPlayer ? 1.0 : 0.3, layerAlpha * 0.5));
		}

		// 足元の魔法陣
		for (int ring = 0; ring < 3; ++ring)
		{
			double radius = map.tileSize * (0.6 + ring * 0.2) + sin(time * 3 + ring) * 5;
			Circle(centerPos, radius).drawFrame(2, ColorF(activeUnit.isPlayer ? 0.4 : 1.0,
				activeUnit.isPlayer ? 0.7 : 0.4,
				activeUnit.isPlayer ? 1.0 : 0.4,
				0.4 - ring * 0.1));
		}

		// 回転するルーン文字風
		for (int i = 0; i < 8; ++i)
		{
			double angle = time + i * Math::TwoPi / 8;
			Vec2 runePos = centerPos + Vec2(Cos(angle), Sin(angle)) * (map.tileSize * 0.5);
			RectF(Arg::center(runePos), 4, 8).rotated(angle).draw(ColorF(1, 1, 0.8, 0.5));
		}
	}

	// ユニット描画
	for (auto& u : units)
	{
		if (u.alive) u.draw(map.tileSize);
	}

	// =================================================================
	// 🎮 UI表示（画面サイズ対応）
	// =================================================================
	{
		double panelWidth = Min(screenW * 0.3, 500.0);
		double panelHeight = Min(screenH * 0.1, 90.0);
		RectF turnPanel(screenW / 2 - panelWidth / 2, screenH * 0.02, panelWidth, panelHeight);

		{
			ScopedRenderStates2D blend(BlendState::Additive);
			turnPanel.stretched(4).draw(ColorF(1, 1, 0.8, 0.15));
		}

		turnPanel.movedBy(4, 4).draw(ColorF(0, 0, 0, 0.5));

		Color panelColor = (phase == TurnPhase::PlayerTurn) ?
			ColorF(0.25, 0.45, 0.9) : ColorF(0.9, 0.35, 0.35);

		if (phase == TurnPhase::BattleEnd)
		{
			panelColor = PlayerWon() ? ColorF(0.3, 0.9, 0.4) : ColorF(0.6, 0.6, 0.6);
		}

		turnPanel.draw(panelColor.lerp(Palette::Black, 0.3));
		turnPanel.drawFrame(5, ColorF(1, 1, 0.9));

		for (auto corner : { turnPanel.tl(), turnPanel.tr(), turnPanel.bl(), turnPanel.br() })
		{
			Circle(corner, 12).draw(ColorF(1, 0.9, 0.6));
			Circle(corner, 10).draw(ColorF(1, 1, 0.8, 0.8 + sin(time * 4) * 0.2));
		}

		// ★ 天候表示
		String weatherText = BattleSystem::GetWeatherIcon(m_weather) + U" " + BattleSystem::GetWeatherName(m_weather);
		FontAsset(U"title")(weatherText).drawAt(turnPanel.center().x, turnPanel.y - 40, Palette::White);

		String turnText = (phase == TurnPhase::PlayerTurn) ? U"味方ターン" : U"敵ターン";
		if (phase == TurnPhase::BattleEnd)
		{
			if (PlayerWon()) turnText = U"🎉 大勝利！ 🎉";
			else turnText = U"敗北...";
		}

		for (int layer = 4; layer > 0; --layer)
		{
			FontAsset(U"huge")(turnText).drawAt(
				turnPanel.center().movedBy(layer, layer),
				ColorF(0, 0, 0, 0.15)
			);
		}
		FontAsset(U"huge")(turnText).drawAt(turnPanel.center(), Palette::White);

		{
			ScopedRenderStates2D blend(BlendState::Additive);
			FontAsset(U"huge")(turnText).drawAt(turnPanel.center(), ColorF(1, 1, 1, 0.3));
		}
	}

	// =================================================================
	// 🎖️ 対決する武将の顔表示
	// =================================================================
	{
		// プレイヤー武将（左側）
		if (!units.isEmpty())
		{
			for (const auto& unit : units)
			{
				if (unit.side == Side::Player)
				{
					Officer playerOfficer;
					playerOfficer.name = unit.name;
					playerOfficer.leadership = 85;
					playerOfficer.power = 85;
					playerOfficer.intelligence = 75;
					playerOfficer.politics = 75;

					Vec2 playerPortraitPos(screenW * 0.15, screenH * 0.1);
					OfficerPortrait::Draw(playerOfficer, playerPortraitPos, 100);
					OfficerPortrait::DrawNamePlate(playerOfficer, playerPortraitPos, 100);

					// 兵力表示
					FontAsset(U"menu")(U"兵力: {}"_fmt(unit.soldiers))
						.drawAt(playerPortraitPos.x, playerPortraitPos.y + 80, Palette::White);
					break;
				}
			}
		}

		// 敵武将（右側）
		if (!units.isEmpty())
		{
			for (const auto& unit : units)
			{
				if (unit.side == Side::Enemy)
				{
					Officer enemyOfficer;
					enemyOfficer.name = unit.name;
					enemyOfficer.leadership = 80;
					enemyOfficer.power = 80;
					enemyOfficer.intelligence = 70;
					enemyOfficer.politics = 70;

					Vec2 enemyPortraitPos(screenW * 0.85, screenH * 0.1);
					OfficerPortrait::Draw(enemyOfficer, enemyPortraitPos, 100);
					OfficerPortrait::DrawNamePlate(enemyOfficer, enemyPortraitPos, 100);

					// 兵力表示
					FontAsset(U"menu")(U"兵力: {}"_fmt(unit.soldiers))
						.drawAt(enemyPortraitPos.x, enemyPortraitPos.y + 80, Palette::White);
					break;
				}
			}
		}

		// VS表示
		{
			ScopedRenderStates2D blend(BlendState::Additive);
			FontAsset(U"huge")(U"VS")
				.drawAt(screenW / 2, screenH * 0.1,
						ColorF(1, 0.5 + sin(time * 5) * 0.5, 0));
		}
	}

	// 操作ヒント
	if (phase == TurnPhase::PlayerTurn && !moveRange.isEmpty())
	{
		double hintWidth = Min(screenW * 0.4, 600.0);
		double hintHeight = Min(screenH * 0.07, 60.0);
		RectF hintPanel(screenW / 2 - hintWidth / 2, screenH - hintHeight - screenH * 0.02, hintWidth, hintHeight);
		hintPanel.draw(ColorF(0, 0, 0, 0.8));
		hintPanel.drawFrame(2, ColorF(1, 1, 0.8));
		FontAsset(U"menu")(U"[左クリック] 移動 ／ 隣接で自動攻撃")
			.drawAt(hintPanel.center(), ColorF(1, 1, 0.9));
	}

	// 勝利時の花火演出
	if (phase == TurnPhase::BattleEnd && PlayerWon())
	{
		ScopedRenderStates2D blend(BlendState::Additive);

		for (int i = 0; i < 30; ++i)
		{
			double fireworkTime = time * 2 + i * 0.5;
			double x = static_cast<double>((i * 137) % Scene::Width());
			double baseY = Scene::Height() / 2;

			for (int spark = 0; spark < 12; ++spark)
			{
				double angle = spark * Math::TwoPi / 12;
				double dist = Math::Fmod(fireworkTime, 2.0) * 150;
				double alpha = Max(0.0, 1.0 - Math::Fmod(fireworkTime, 2.0) / 2.0);

				Vec2 sparkPos = Vec2(x, baseY) + Vec2(Cos(angle), Sin(angle)) * dist;

				Color fireworkColor;
				if (i % 3 == 0) fireworkColor = ColorF(1.0, 0.5, 0.5);
				else if (i % 3 == 1) fireworkColor = ColorF(0.5, 1.0, 0.7);
				else fireworkColor = ColorF(0.7, 0.7, 1.0);

				Circle(sparkPos, 4).draw(fireworkColor.withAlpha(static_cast<uint8>(alpha * 200)));
			}
		}

		// 金色の紙吹雪
		for (int i = 0; i < 100; ++i)
		{
			double confettiTime = time + i * 0.1;
			double x = Math::Fmod(i * 23.7 + sin(confettiTime) * 50, static_cast<double>(Scene::Width()));
			double y = Math::Fmod(confettiTime * 100, static_cast<double>(Scene::Height()));
			double rotation = confettiTime * 5;

			RectF(Arg::center(x, y), 5, 10).rotated(rotation).draw(ColorF(1.0, 0.8, 0.5, 0.7));
		}
	}

	// 敗北時の演出
	if (phase == TurnPhase::BattleEnd && PlayerLost())
	{
		Scene::Rect().draw(ColorF(0, 0, 0, 0.3));

		for (int i = 0; i < 200; ++i)
		{
			double rainTime = time * 5 + i;
			double x = static_cast<double>((i * 7) % Scene::Width());
			double y = Math::Fmod(rainTime * 300, static_cast<double>(Scene::Height() + 100)) - 100;

			Line(x, y, x - 2, y + 15).draw(1, ColorF(0.7, 0.7, 0.8, 0.3));
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

	// ★ マップのオフセットを計算
	double mapTotalWidth = map.width * map.tileSize;
	double mapTotalHeight = map.height * map.tileSize;
	double offsetX = (Scene::Width() - mapTotalWidth) / 2.0;
	double offsetY = (Scene::Height() - mapTotalHeight) / 2.0;

	Point click = (Cursor::Pos() - Point(static_cast<int>(offsetX), static_cast<int>(offsetY))) / map.tileSize;
	if (!moveRange.contains(click)) return;

	Unit& u = units[actingIndex];
	if (u.pos == click) return;

	u.pos = click;
	moveRange.clear();

	// ★ 移動したら行動済みにする
	u.acted = true;
	actingIndex++;
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
