#include "WorldMapScene.hpp"
#include "CityScene.hpp"
#include "OfficerPortrait.hpp"
#include "SaveLoadManager.hpp"

WorldMapScene::WorldMapScene(GameManager* gm,
	const Faction& faction,
	Array<CityData>* allCities)
	: m_gameManager(gm)
	, m_playerFaction(faction)
	, m_allCities(allCities)
	, m_hovered(-1)
	, m_selectedIndex(-1)
	, m_cutInTimer(0.0)
	, m_seasonColor(Palette::White)
{
	if (m_allCities) m_gameManager->EnsureTerritory(*m_allCities);
	// ★ セーブ・ロードボタンの配置
	m_btnSave = Rect(Scene::Width() - 250, 100, 200, 60);
	m_btnLoad = Rect(Scene::Width() - 250, 180, 200, 60);

	// ★ 外交ボタンの配置
	m_btnDiplomacy = Rect(Scene::Width() - 250, 260, 200, 60);

	// パフォーマンス最適化: 都市間接続を事前計算（初回のみ）
	if (m_allCities)
	{
		const size_t cityCount = m_allCities->size();
		m_cityConnections.resize(cityCount);

		for (size_t i = 0; i < cityCount; ++i)
		{
			const Vec2 cityA = (*m_allCities)[i].pos;
			Array<std::pair<double, int>> distances;

			for (size_t j = 0; j < cityCount; ++j)
			{
				if (i == j) continue;
				const Vec2 cityB = (*m_allCities)[j].pos;
				const double dist = cityA.distanceFrom(cityB);
				distances.push_back({ dist, static_cast<int>(j) });
			}

			// 距離でソートして最近接2都市を取得
			distances.sort_by([](const auto& a, const auto& b) { return a.first < b.first; });

			const int maxConnections = Min(2, static_cast<int>(distances.size()));
			m_cityConnections[i].resize(maxConnections);
			for (int k = 0; k < maxConnections; ++k)
			{
				m_cityConnections[i][k] = distances[k].second;
			}
		}
	}
}

void WorldMapScene::update()
{
	UpdateTerritory();
	if (m_gameManager->career.officerMode && Rect(Scene::Width() - 250, 340, 200, 60).leftClicked())
	{
		m_sceneEnd = true;
		m_nextScene = U"Career";
		return;
	}
	// ★ セーブボタン
	if (m_btnSave.leftClicked())
	{
		if (m_gameManager && m_allCities)
		{
			SaveData saveData = m_gameManager->CreateSaveData(*m_allCities);
			SaveLoadManager::Save(saveData, 1);  // スロット1にセーブ
			Print << U"💾 セーブしました！";
		}
	}

	// ★ ロードボタン
	if (m_btnLoad.leftClicked())
	{
		auto loadedData = SaveLoadManager::Load(1);  // スロット1からロード
		if (loadedData && m_gameManager && m_allCities)
		{
			// ゲームデータを復元
			*m_allCities = loadedData->cities;
			m_gameManager->year = loadedData->year;
			m_gameManager->month = loadedData->month;
			m_gameManager->playerFactionName = loadedData->playerFactionName;
			m_gameManager->career = loadedData->career;
			m_gameManager->territory = loadedData->territory;
			m_gameManager->territoryReady = loadedData->territoryReady;
			m_gameManager->turnManager.commandsUsed = loadedData->commandsUsed;
			m_gameManager->pendingBattle = GameManager::PendingBattle{};
			m_playerFaction.name = loadedData->playerFactionName;
			m_sceneEnd = true;
			m_nextScene = U"WorldMap";
			return;
			Print << U"📂 ロードしました！ " << loadedData->year << U"年" << loadedData->month << U"月";
		}
	}

	// ★ 外交ボタン
	if (m_btnDiplomacy.leftClicked() && !m_gameManager->career.officerMode)
	{
		m_sceneEnd = true;
		m_nextScene = U"Diplomacy";
	}

	if (KeyEnter.down())
	{
		if (m_gameManager && m_allCities)
		{
			m_gameManager->advanceMonth(*m_allCities);
			m_playerFaction.name = m_gameManager->playerFactionName;

			int m = m_gameManager->month;

			if (m_gameManager->pendingBattle.isOccurring)
			{
				int atkIdx = m_gameManager->pendingBattle.atkCityIndex;
				int defIdx = m_gameManager->pendingBattle.defCityIndex;

				// ★ 警告メッセージを設定
				m_cutInTimer = 3.0;
				m_cutInText = U"⚠️ 緊急警報 ⚠️";
				m_seasonText = (*m_allCities)[atkIdx].owner + U" が " + (*m_allCities)[defIdx].name + U" に侵攻！";
				m_seasonColor = Palette::Red;

				m_nextScene = U"Battle";
				m_sceneEnd = true;

				Print << U"[防衛戦発生] " << (*m_allCities)[atkIdx].name
					<< U" が " << (*m_allCities)[defIdx].name << U" に侵攻！";
				return;
			}

			if (m % 3 == 0)
			{
				m_cutInTimer = 3.5;
				m_cutInText = U"{}年 {}月"_fmt(m_gameManager->year, m_gameManager->month);

				if (m == 3) {
					m_seasonText = U"春";
					m_seasonColor = Palette::Pink;
				}
				else if (m == 6) {
					m_seasonText = U"夏";
					m_seasonColor = ColorF(0.2, 0.6, 1.0);
				}
				else if (m == 9) {
					m_seasonText = U"秋";
					m_seasonColor = Palette::Orangered;
				}
				else {
					m_seasonText = U"冬";
					m_seasonColor = Palette::White;
				}
			}
		}
	}

	m_cutInTimer -= Scene::DeltaTime();
	if (m_cutInTimer < 0) m_cutInTimer = 0;

	if (m_cutInTimer <= 0)
	{
		m_hovered = -1;
		if (m_allCities)
		{
			// ★ draw()と完全に同じ座標変換を使用
			double mapMargin = 20;  // 固定値
			double rightPanelWidth = 400;
			RectF mapArea(mapMargin, mapMargin, 
						  Scene::Width() - mapMargin * 2 - rightPanelWidth, 
						  Scene::Height() - mapMargin * 2);

			double scaleX = mapArea.w / 1520.0;
			double scaleY = mapArea.h / 820.0;
			double scale = Min(scaleX, scaleY);

			double offsetX = mapArea.x + (mapArea.w - 1520.0 * scale) / 2;
			double offsetY = mapArea.y + (mapArea.h - 820.0 * scale) / 2;

			for (int i = 0; i < m_allCities->size(); ++i)
			{
				const CityData& c = (*m_allCities)[i];

				// ★ 座標変換を適用
				Vec2 scaledPos(c.pos.x * scale + offsetX, c.pos.y * scale + offsetY);

				// ★ 当たり判定のサイズを城のサイズに合わせる
				double baseSize = 15.0 * scale;
				if (Circle(scaledPos, baseSize * 2).mouseOver())  // 城の2倍の範囲で判定
				{
					m_hovered = i;
					if (MouseL.down() && c.owner == m_playerFaction.name && m_gameManager->career.CanManageCity(c.name))
					{
						m_selectedIndex = i;
						m_sceneEnd = true;
						m_nextScene = U"City";
						return;
					}
				}
			}
		}
	}
}

void WorldMapScene::draw() const
{
	double time = Scene::Time();

	// =================================================================
	// 🎨 背景：暗めの地図風テクスチャ（三國志風）
	// =================================================================
	Scene::SetBackground(ColorF{ 0.15, 0.13, 0.11 });

	// 地形のテクスチャ（山・川の雰囲気）
	for (int i = 0; i < 150; ++i)
	{
		double x = RandomClosed(0.0, static_cast<double>(Scene::Width()));
		double y = RandomClosed(0.0, static_cast<double>(Scene::Height()));
		double size = RandomClosed(50.0, 200.0);
		Circle(x, y, size).draw(ColorF(0.12, 0.11, 0.10, 0.2));
	}

	// =================================================================
	// 🗺️ 地図エリアの設定
	// =================================================================
	double mapMargin = 20;
	// 右側に情報パネルのスペースを確保
	double rightPanelWidth = 400;
	RectF mapArea(mapMargin, mapMargin, 
				  Scene::Width() - mapMargin * 2 - rightPanelWidth, 
				  Scene::Height() - mapMargin * 2);

	// 地図の影
	mapArea.movedBy(3, 3).draw(ColorF(0, 0, 0, 0.6));

	// 地図の背景（古地図風）
	mapArea.draw(ColorF(0.22, 0.20, 0.18));

	// 枠線
	mapArea.drawFrame(4, 0, ColorF(0.4, 0.35, 0.25));
	mapArea.drawFrame(2, 4, ColorF(0.6, 0.55, 0.45));

	if (m_allCities)
	{
		// ★ 座標スケーリング
		double scaleX = mapArea.w / 1520.0;
		double scaleY = mapArea.h / 820.0;
		double scale = Min(scaleX, scaleY);

		double offsetX = mapArea.x + (mapArea.w - 1520.0 * scale) / 2;
		double offsetY = mapArea.y + (mapArea.h - 820.0 * scale) / 2;

		// =================================================================
		// 支配済み領地と未開拓地をグリッドで表示
		// =================================================================
		for (int i = 0; i < TerritoryMap::Count; ++i)
		{
			const auto& cell = m_gameManager->territory.cells[i];
			if (cell.city < 0 || cell.city >= static_cast<int>(m_allCities->size())) continue;
			const auto& city = (*m_allCities)[cell.city];
			const Color color = city.owner == m_playerFaction.name ? m_playerFaction.color : city.color;
			const RectF tile(offsetX + (i % TerritoryMap::Columns) * TerritoryMap::CellWidth * scale,
				offsetY + (i / TerritoryMap::Columns) * TerritoryMap::CellHeight * scale,
				TerritoryMap::CellWidth * scale, TerritoryMap::CellHeight * scale);
			tile.draw(cell.level > 0 ? ColorF(color, 0.3 + cell.level * 0.12) : ColorF(0.25, 0.23, 0.19, 0.5));
			tile.drawFrame(0.5, ColorF(0.8, 0.75, 0.6, 0.15));
			if (i == m_selectedTerritory) tile.drawFrame(2, Palette::Gold);
		}
		const auto territoryOwner = [&](int index) -> String
		{
			const auto& cell = m_gameManager->territory.cells[index];
			return cell.level > 0 && cell.city >= 0 && cell.city < static_cast<int>(m_allCities->size())
				? (*m_allCities)[cell.city].owner : U"";
		};
		for (int i = 0; i < TerritoryMap::Count; ++i)
		{
			const double x = offsetX + (i % TerritoryMap::Columns) * TerritoryMap::CellWidth * scale;
			const double y = offsetY + (i / TerritoryMap::Columns) * TerritoryMap::CellHeight * scale;
			const double w = TerritoryMap::CellWidth * scale, h = TerritoryMap::CellHeight * scale;
			if (i % TerritoryMap::Columns + 1 < TerritoryMap::Columns && territoryOwner(i) != territoryOwner(i + 1))
				Line(x + w, y, x + w, y + h).draw(1.5, ColorF(0.95, 0.9, 0.75, 0.75));
			if (i / TerritoryMap::Columns + 1 < TerritoryMap::Rows && territoryOwner(i) != territoryOwner(i + TerritoryMap::Columns))
				Line(x, y + h, x + w, y + h).draw(1.5, ColorF(0.95, 0.9, 0.75, 0.75));
		}

		// =================================================================
		// 都市間の連絡路
		// =================================================================
		{
			for (int i = 0; i < m_allCities->size(); ++i)
			{
				const CityData& cityA = (*m_allCities)[i];
				Vec2 posA(cityA.pos.x * scale + offsetX, cityA.pos.y * scale + offsetY);

				for (int j : m_cityConnections[i])
				{
					if (i >= j) continue;  // 重複回避

					const CityData& cityB = (*m_allCities)[j];
					Vec2 posB(cityB.pos.x * scale + offsetX, cityB.pos.y * scale + offsetY);

					// 異なる勢力間の境界線のみ描画
					if (cityA.owner != cityB.owner)
					{
						// 太い白線（境界）
						Line(posA, posB).draw(1, ColorF(0.9, 0.85, 0.7, 0.25));
					}
					else
					{
						// 同じ勢力内は細い線
						Line(posA, posB).draw(2, ColorF(0.3, 0.3, 0.3, 0.2));
					}
				}
			}
		}

		// =================================================================
		// 🏯 都市の描画（リアルな城）
		// =================================================================
		for (int i = 0; i < m_allCities->size(); ++i)
		{
			const CityData& c = (*m_allCities)[i];
			bool myCity = (c.owner == m_playerFaction.name);
			bool isHovered = (i == m_hovered && m_cutInTimer <= 0);

			Vec2 pos(c.pos.x * scale + offsetX, c.pos.y * scale + offsetY);

			Color factionColor = myCity ? m_playerFaction.color : c.color;

			// ★ 修正：城のサイズを適切に（15pxに縮小）
			double baseSize = 15.0 * scale;  // スケールに応じたサイズ

			// ホバー時のエフェクト
			if (isHovered)
			{
				pos.y -= 3 + sin(time * 4) * 1.5;
				
				// 光るエフェクト
				ScopedRenderStates2D blend(BlendState::Additive);
				Circle(pos, (baseSize * 2 + sin(time * 3) * 3))
					.drawFrame(2, ColorF(Palette::Yellow, 0.5));
			}

			// =================================================================
			// 城のパーツ描画（シンプル版）
			// =================================================================

			// 1. 影
			Circle(pos.movedBy(2, 2), baseSize * 1.2).draw(ColorF(0, 0, 0, 0.4));

			// 2. 城壁の基礎（台形）
			double wallW = baseSize * 2.2;
			double wallH = baseSize * 1.5;
			Vec2 wallTop = pos.movedBy(0, -wallH);
			
			Quad(
				wallTop.movedBy(-wallW * 0.3, 0),
				wallTop.movedBy(wallW * 0.3, 0),
				pos.movedBy(wallW * 0.5, 0),
				pos.movedBy(-wallW * 0.5, 0)
			).draw(ColorF(0.3, 0.28, 0.25));

			// 3. 城本体（勢力色）
			double towerW = baseSize * 1.4;
			double towerH = baseSize * 1.8;
			RectF mainTower(pos.x - towerW / 2, pos.y - towerH, towerW, towerH);
			
			// 天守閣の壁
			mainTower.draw(Arg::top = ColorF(factionColor), 
						   Arg::bottom = ColorF(factionColor.r * 0.6, 
												factionColor.g * 0.6, 
												factionColor.b * 0.6));
			
			// 窓
			for (int row = 0; row < 2; ++row)
			{
				double wy = mainTower.y + (row + 0.5) * (towerH / 3);
				RectF(pos.x - 2, wy, 4, 5).draw(ColorF(0.1, 0.1, 0.1));
			}
			
			mainTower.drawFrame(1, ColorF(0.2, 0.18, 0.15));

			// 4. 屋根（三角形）
			double roofW = baseSize * 1.8;
			double roofH = baseSize * 0.8;
			
			Color roofColor = myCity ? ColorF(0.85, 0.75, 0.25) : ColorF(0.25, 0.25, 0.25);
			Triangle(
				Vec2(pos.x, mainTower.y - roofH),
				Vec2(pos.x - roofW / 2, mainTower.y),
				Vec2(pos.x + roofW / 2, mainTower.y)
			).draw(roofColor);
			
			Triangle(
				Vec2(pos.x, mainTower.y - roofH),
				Vec2(pos.x - roofW / 2, mainTower.y),
				Vec2(pos.x + roofW / 2, mainTower.y)
			).drawFrame(1, ColorF(0.15, 0.13, 0.11));

			// 5. 旗印（自分の城のみ）
			if (myCity)
			{
				Vec2 flagPole(pos.x, mainTower.y - roofH);
				double flagPoleH = baseSize * 1.2;
				
				// 旗竿
				Line(flagPole, flagPole.movedBy(0, -flagPoleH))
					.draw(2, ColorF(0.3, 0.25, 0.2));
				
				// 旗
				double flagW = baseSize * 0.8;
				double flagH = baseSize * 0.6;
				Vec2 flagTop = flagPole.movedBy(0, -flagPoleH + 3);
				double wave = sin(time * 3 + i) * 2;
				
				Quad(
					flagTop,
					flagTop.movedBy(flagW + wave, 0),
					flagTop.movedBy(flagW + wave * 0.5, flagH),
					flagTop.movedBy(0, flagH)
				).draw(factionColor);
			}

			// 6. 城のレベル表示（★）
			int cityLevel = Min(3, c.troops / 3000);  // 3000ごとに★1つ
			if (cityLevel > 0)
			{
				Vec2 starPos(pos.x - (cityLevel - 1) * 6, pos.y + baseSize * 2.2);
				for (int s = 0; s < cityLevel; ++s)
				{
					Shape2D::Star(5, starPos.movedBy(s * 12, 0), 4)
						.draw(ColorF(0, 0, 0, 0.5));
					Shape2D::Star(5, starPos.movedBy(s * 12 - 1, -1), 4)
						.draw(ColorF(Palette::Gold));
				}
			}

			// 7. 都市名
			Vec2 namePos(pos.x, pos.y - baseSize * 2.5 - roofH);
			
			// 名前の影
			FontAsset(U"small")(c.name).drawAt(namePos.movedBy(1, 1), ColorF(0, 0, 0, 0.8));
			
			// 名前本体
			FontAsset(U"small")(c.name).drawAt(namePos, 
				myCity ? ColorF(Palette::Lightgreen) : ColorF(0.95, 0.95, 0.95));
		}
	}

	// =================================================================
	// 📊 右側情報パネル（三國志風）
	// =================================================================
	{
		RectF infoPanel(Scene::Width() - rightPanelWidth + 10, 10, 
						rightPanelWidth - 20, Scene::Height() - 20);

		// パネル背景
		infoPanel.movedBy(3, 3).draw(ColorF(0, 0, 0, 0.5));
		infoPanel.draw(ColorF(0.12, 0.10, 0.08, 0.95));
		infoPanel.drawFrame(3, ColorF(0.6, 0.5, 0.35));

		// 装飾
		Line(infoPanel.pos.movedBy(10, 50), infoPanel.pos.movedBy(infoPanel.w - 10, 50))
			.draw(2, ColorF(0.5, 0.4, 0.3));

		int panelX = static_cast<int>(infoPanel.x + 20);
		int panelY = static_cast<int>(infoPanel.y + 20);

		// タイトル
		FontAsset(U"medium")(U"出陣先の都市を選択してください")
			.draw(panelX, panelY, ColorF(0.9, 0.85, 0.7));

		panelY += 60;

		// ホバー中の都市情報
		if (m_hovered >= 0 && m_cutInTimer <= 0 && m_allCities)
		{
			const CityData& c = (*m_allCities)[m_hovered];

			// 勢力名
			FontAsset(U"medium")(c.owner)
				.draw(panelX, panelY, c.owner == m_playerFaction.name ? Palette::Lime : Palette::Orange);
			panelY += 40;

			// 都市情報
			FontAsset(U"small")(U"都市: " + c.name)
				.draw(panelX, panelY, Palette::White);
			panelY += 30;

			FontAsset(U"small")(U"兵力: " + Format(c.troops))
				.draw(panelX, panelY, Palette::Lightblue);
			panelY += 25;

			FontAsset(U"small")(U"金: " + Format(c.gold))
				.draw(panelX, panelY, Palette::Gold);
			panelY += 25;

			FontAsset(U"small")(U"兵糧: " + Format(c.food))
				.draw(panelX, panelY, Palette::Lightgreen);
			panelY += 25;

			FontAsset(U"small")(U"治安: " + Format(c.order))
				.draw(panelX, panelY, Palette::Yellow);
			panelY += 35;

			// 武将情報
			if (!c.officers.isEmpty())
			{
				FontAsset(U"small")(U"━━ 配置武将 ━━")
					.draw(panelX, panelY, ColorF(0.6, 0.5, 0.4));
				panelY += 30;

				for (const auto& officer : c.officers)
				{
					FontAsset(U"small")(officer.name)
						.draw(panelX, panelY, Palette::White);
					
					FontAsset(U"small")(U"  武:" + Format(officer.power) + 
									   U" 知:" + Format(officer.intelligence))
						.draw(panelX + 10, panelY + 20, ColorF(0.8, 0.8, 0.8));
					panelY += 45;
				}
			}
		}
	}

	// =================================================================
	// ⏱️ 日付・季節表示（左上）
	// =================================================================
	{
		RectF datePanel(20, 20, 200, 80);
		datePanel.draw(ColorF(0.1, 0.08, 0.06, 0.9));
		datePanel.drawFrame(2, ColorF(0.6, 0.5, 0.35));

		String dateText = U"{}年 {}月"_fmt(m_gameManager->getYear(), m_gameManager->getMonth());
		FontAsset(U"medium")(dateText).draw(30, 35, m_seasonColor);
	}

	// =================================================================
	// 🎬 カットイン演出
	// =================================================================
	if (m_cutInTimer > 0)
	{
		double alpha = Min(m_cutInTimer / 0.5, 1.0);
		Scene::Rect().draw(ColorF(0, 0, 0, alpha * 0.7));

		FontAsset(U"title")(m_cutInText)
			.drawAt(Scene::Center().movedBy(0, -50), ColorF(1, 1, 1, alpha));

		FontAsset(U"large")(m_seasonText)
			.drawAt(Scene::Center().movedBy(0, 20), ColorF(m_seasonColor, alpha));
	}

	// =================================================================
	// 🎮 操作ガイド（下部）
	// =================================================================
	{
		String guide = U"Enter: 月を進める / ESC: メニュー";
		FontAsset(U"small")(guide)
			.draw(Scene::Width() / 2 - 200, Scene::Height() - 40, ColorF(0.7, 0.7, 0.7));
	}

	// セーブ・ロードボタンは左下に移動
	{
		// セーブボタン
		bool saveHovered = m_btnSave.mouseOver();
		m_btnSave.draw(saveHovered ? ColorF(0.3, 0.5, 0.3) : ColorF(0.2, 0.3, 0.2));
		m_btnSave.drawFrame(3, saveHovered ? Palette::Lime : Palette::Green);

		// セーブアイコン（フロッピーディスク風）
		RectF diskBody(m_btnSave.x + 20, m_btnSave.y + 15, 30, 30);
		diskBody.draw(Palette::Darkgray);
		RectF diskLabel(m_btnSave.x + 20, m_btnSave.y + 15, 30, 10);
		diskLabel.draw(Palette::Lightgray);
		RectF diskShutter(m_btnSave.x + 32, m_btnSave.y + 30, 6, 10);
		diskShutter.draw(Palette::Black);

		FontAsset(U"menu")(U"セーブ").draw(m_btnSave.x + 60, m_btnSave.y + 20, Palette::White);

		// ロードボタン
		bool loadHovered = m_btnLoad.mouseOver();
		m_btnLoad.draw(loadHovered ? ColorF(0.3, 0.3, 0.5) : ColorF(0.2, 0.2, 0.3));
		m_btnLoad.drawFrame(3, loadHovered ? Palette::Cyan : Palette::Skyblue);

		// ロードアイコン（フォルダ風）
		RectF folderBody(m_btnLoad.x + 20, m_btnLoad.y + 20, 30, 25);
		folderBody.draw(Palette::Goldenrod);
		Triangle(
			Vec2(m_btnLoad.x + 20, m_btnLoad.y + 20),
			Vec2(m_btnLoad.x + 35, m_btnLoad.y + 20),
			Vec2(m_btnLoad.x + 30, m_btnLoad.y + 15)
		).draw(Palette::Gold);

		FontAsset(U"menu")(U"ロード").draw(m_btnLoad.x + 60, m_btnLoad.y + 20, Palette::White);

		// 外交ボタン
		bool dipHovered = m_btnDiplomacy.mouseOver();
		m_btnDiplomacy.draw(dipHovered ? ColorF(0.4, 0.3, 0.5) : ColorF(0.25, 0.2, 0.3));
		m_btnDiplomacy.drawFrame(3, dipHovered ? Palette::Magenta : Palette::Purple);

		// 外交アイコン（握手風）
		Vec2 handPos1(m_btnDiplomacy.x + 25, m_btnDiplomacy.y + 30);
		Vec2 handPos2(m_btnDiplomacy.x + 45, m_btnDiplomacy.y + 30);
		Circle(handPos1, 8).draw(Palette::Wheat);
		Circle(handPos2, 8).draw(Palette::Wheat);
		Line(handPos1, handPos2).draw(4, Palette::Tan);

		FontAsset(U"menu")(U"外交").draw(m_btnDiplomacy.x + 60, m_btnDiplomacy.y + 20, Palette::White);
	}

	if (m_gameManager->career.officerMode)
	{
		Rect(Scene::Width() - 250, 340, 200, 60).draw(ColorF(0.25, 0.35, 0.4));
		FontAsset(U"menu")(U"任務・身分").drawAt(Scene::Width() - 150, 370);
		FontAsset(U"small")(m_gameManager->career.RoleName() + U" / 功績 " + Format(m_gameManager->career.merit)).draw(Scene::Width() - 250, 410);
		FontAsset(U"small")(U"外交は君主が担当します").draw(Scene::Width() - 250, 445);
	}
	// =================================================================
	// 📢 イベント通知（画面下部）
	// =================================================================
	if (m_gameManager && !m_gameManager->eventLog.isEmpty())
	{
		double notifY = Scene::Height() - 200;
		RectF notifPanel(50, notifY, Scene::Width() - 500, 150);

		// 背景
		notifPanel.draw(ColorF(0.1, 0.1, 0.15, 0.95));
		notifPanel.drawFrame(4, Palette::Gold);

		// イベントログを表示
		int lineY = static_cast<int>(notifY + 15);
		for (const auto& log : m_gameManager->eventLog)
		{
			FontAsset(U"menu")(log).draw(70, lineY, Palette::White);
			lineY += 25;
			if (lineY > notifY + 140) break;  // 最大6行
		}
	}
	DrawTerritoryPanel();
}

