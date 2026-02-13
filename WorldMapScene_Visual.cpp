#include "WorldMapScene.hpp"
#include "CityScene.hpp"

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
}

void WorldMapScene::update()
{
	if (KeyEnter.down())
	{
		if (m_gameManager && m_allCities)
		{
			m_gameManager->advanceMonth(*m_allCities);

			int m = m_gameManager->month;

			if (m_gameManager->pendingBattle.isOccurring)
			{
				int atkIdx = m_gameManager->pendingBattle.atkCityIndex;
				int defIdx = m_gameManager->pendingBattle.defCityIndex;

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
			// ★ 座標変換の計算（draw()と同じ）
			double mapMargin = Min(Scene::Width(), Scene::Height()) * 0.025;
			RectF mapArea(mapMargin, mapMargin, Scene::Width() - mapMargin * 2, Scene::Height() - mapMargin * 2);

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

				if (Circle(scaledPos, 30 * scale).mouseOver())
				{
					m_hovered = i;
					if (MouseL.down() && c.owner == m_playerFaction.name)
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
	// 🎨 背景：木目調のフレーム
	// =================================================================
	Scene::SetBackground(ColorF{ 0.15, 0.12, 0.10 });

	// 木目のテクスチャ（外側のフレーム）
	RectF outerFrame(0, 0, Scene::Width(), Scene::Height());

	// 木目の縦線
	for (int i = 0; i < 60; ++i)
	{
		double x = i * 30.0 + sin(i * 0.5) * 5;
		Line(x, 0, x + sin(i) * 10, Scene::Height())
			.draw(2, ColorF(0.12, 0.10, 0.08, 0.3));
	}

	// 木目の節
	for (int i = 0; i < 20; ++i)
	{
		double x = RandomClosed(0.0, static_cast<double>(Scene::Width()));
		double y = RandomClosed(0.0, static_cast<double>(Scene::Height()));
		Circle(x, y, RandomClosed(10.0, 30.0)).draw(ColorF(0.10, 0.08, 0.06, 0.2));
	}

	// =================================================================
	// 🗺️ 地図の本体（内側）
	// =================================================================

	// 画面サイズに応じた地図エリア
	double mapMargin = Min(Scene::Width(), Scene::Height()) * 0.025;  // 画面の2.5%をマージン
	RectF mapArea(mapMargin, mapMargin, Scene::Width() - mapMargin * 2, Scene::Height() - mapMargin * 2);

	// 地図の影
	mapArea.movedBy(4, 4).draw(ColorF(0, 0, 0, 0.6));

	// 地図の背景（古紙風）
	mapArea.draw(Arg::top = ColorF(0.88, 0.85, 0.78), Arg::bottom = ColorF(0.82, 0.78, 0.70));

	// 紙のテクスチャ（細かい）
	{
		for (int i = 0; i < 100; ++i)
		{
			double x = RandomClosed(mapArea.x, mapArea.x + mapArea.w);
			double y = RandomClosed(mapArea.y, mapArea.y + mapArea.h);
			Circle(x, y, RandomClosed(20.0, 80.0)).draw(ColorF(0.78, 0.74, 0.68, 0.03));
		}
	}

	const Transformer2D mapTransform{ Mat3x2::Identity() };

	if (m_allCities)
	{
		// ★ 都市座標を画面サイズに合わせてスケーリング
		// CSVの座標は 1600x900 を基準にしているので、現在の画面サイズに合わせる
		double scaleX = mapArea.w / 1520.0;  // 1600 - 80(margin)
		double scaleY = mapArea.h / 820.0;   // 900 - 80(margin)
		double scale = Min(scaleX, scaleY);  // アスペクト比を保つ

		// 中央寄せのオフセット
		double offsetX = mapArea.x + (mapArea.w - 1520.0 * scale) / 2;
		double offsetY = mapArea.y + (mapArea.h - 820.0 * scale) / 2;
		{
			// =================================================================
			// 🌏 領土の塗り分け（ボロノイ図）
			// =================================================================
			for (int y = 0; y < static_cast<int>(mapArea.h); y += 6)
			{
				for (int x = 0; x < static_cast<int>(mapArea.w); x += 6)
				{
					Point pixel(x, y);

					int closestIdx = -1;
					double closestDist = 99999.0;

					for (int i = 0; i < m_allCities->size(); ++i)
					{
						// ★ 座標変換を適用
						Vec2 scaledPos((*m_allCities)[i].pos.x * scale + offsetX,
									   (*m_allCities)[i].pos.y * scale + offsetY);
						Point cityPosInMap(static_cast<int>(scaledPos.x), static_cast<int>(scaledPos.y));

						double dist = pixel.distanceFrom(cityPosInMap);
						if (dist < closestDist)
						{
							closestDist = dist;
							closestIdx = i;
						}
					}

					if (closestIdx >= 0 && closestDist < 350 * scale)
					{
						const CityData& c = (*m_allCities)[closestIdx];
						Color territoryColor = (c.owner == m_playerFaction.name) ? m_playerFaction.color : c.color;

						double gradientFactor = 1.0 - (closestDist / (350 * scale));
						double alpha = 0.25 * gradientFactor;

						RectF(static_cast<double>(x), static_cast<double>(y), 6.0, 6.0)
							.draw(ColorF(territoryColor, alpha));
					}
				}
			}

			// =================================================================
			// 🛣️ 都市間を結ぶ道（白い線）
			// =================================================================
			{
				for (int i = 0; i < m_allCities->size(); ++i)
				{
					Vec2 cityA((*m_allCities)[i].pos.x * scale + offsetX,
							   (*m_allCities)[i].pos.y * scale + offsetY);

					Array<std::pair<double, int>> distances;
					for (int j = 0; j < m_allCities->size(); ++j)
					{
						if (i == j) continue;
						Vec2 cityB((*m_allCities)[j].pos.x * scale + offsetX,
								   (*m_allCities)[j].pos.y * scale + offsetY);
						double dist = cityA.distanceFrom(cityB);
						distances.push_back({ dist, j });
					}

					distances.sort_by([](const auto& a, const auto& b) { return a.first < b.first; });

					for (int k = 0; k < Min(2, static_cast<int>(distances.size())); ++k)
					{
						int j = distances[k].second;
						Vec2 cityB((*m_allCities)[j].pos.x * scale + offsetX,
								   (*m_allCities)[j].pos.y * scale + offsetY);

						Line(cityA, cityB).draw(5, ColorF(0, 0, 0, 0.15));
						Line(cityA, cityB).draw(3, ColorF(0.95, 0.93, 0.88, 0.6));
					}
				}
			}

			// =================================================================
			// 🏯 都市の描画（硬派なスタイル）
			// =================================================================
			for (int i = 0; i < m_allCities->size(); ++i)
			{
				const CityData& c = (*m_allCities)[i];
				bool myCity = (c.owner == m_playerFaction.name);
				bool isHovered = (i == m_hovered && m_cutInTimer <= 0);

				// ★ 座標変換を適用
				Vec2 pos(c.pos.x * scale + offsetX, c.pos.y * scale + offsetY);

				// ホバー時の浮遊
				if (isHovered)
				{
					pos.y -= 3 + sin(time * 5) * 2;
				}

				Color factionColor = myCity ? m_playerFaction.color : c.color;

				// 都市の影
				Circle(pos.movedBy(2, 2), 16 * scale).draw(ColorF(0, 0, 0, 0.4));

				// 都市の外枠（黒）
				Circle(pos, 16 * scale).draw(ColorF(0.2, 0.2, 0.2));

				// 都市の本体（勢力色）
				Circle(pos, 14 * scale).draw(factionColor);

				// 都市の内側（白い円）
				Circle(pos, 11 * scale).draw(ColorF(0.95, 0.95, 0.95));

				// 中心の点
				Circle(pos, 4 * scale).draw(factionColor);

				// 城壁の表現（4本の線）
				for (int angle = 0; angle < 360; angle += 90)
				{
					double rad = Math::ToRadians(static_cast<double>(angle));
					Vec2 start = pos + Vec2(Cos(rad), Sin(rad)) * 9 * scale;
					Vec2 end = pos + Vec2(Cos(rad), Sin(rad)) * 13 * scale;
					Line(start, end).draw(2, ColorF(0.3, 0.3, 0.3));
				}

				// ホバー時の光
				if (isHovered)
				{
					ScopedRenderStates2D blend(BlendState::Additive);
					Circle(pos, (18 + sin(time * 4) * 2) * scale).drawFrame(2, ColorF(factionColor, 0.4));
				}

				// 都市名（影付き）
				FontAsset(U"menu")(c.name).drawAt(pos.x + 1, pos.y - 26 * scale, ColorF(0, 0, 0, 0.7));
				FontAsset(U"menu")(c.name).drawAt(pos.x, pos.y - 25 * scale, ColorF(0.2, 0.2, 0.2));
			}

			// =================================================================
			// 📊 ホバー情報パネル
			// =================================================================
			if (m_hovered >= 0 && m_cutInTimer <= 0)
			{
				const CityData& c = (*m_allCities)[m_hovered];

				RectF panel(60, 60, 380, 180);

				// パネルの影
				panel.movedBy(4, 4).draw(ColorF(0, 0, 0, 0.5));

				// パネル本体（木目調）
				panel.draw(Arg::top = ColorF(0.18, 0.15, 0.12, 0.95), Arg::bottom = ColorF(0.14, 0.12, 0.10, 0.95));
				panel.drawFrame(3, ColorF(0.6, 0.5, 0.35));

				// 装飾
				for (auto corner : { panel.tl(), panel.tr(), panel.bl(), panel.br() })
				{
					Circle(corner, 8).draw(ColorF(0.5, 0.4, 0.3));
				}

				int textX = 80;
				int textY = 80;

				// タイトル
				FontAsset(U"title")(c.name).draw(textX, textY, ColorF(0.95, 0.9, 0.8));

				// 所有者
				FontAsset(U"menu")(U"支配: " + c.owner).draw(textX, textY + 40, ColorF(0.9, 0.9, 0.9));

				// パラメータ
				auto drawStat = [&](String label, int value, int y) {
					FontAsset(U"menu")(label + U": " + Format(value)).draw(textX, y, ColorF(0.85, 0.85, 0.85));
					};

				drawStat(U"金", c.gold, textY + 70);
				drawStat(U"兵糧", c.food, textY + 95);
				drawStat(U"兵士", c.troops, textY + 120);

				// クリック可能か表示
				if (c.owner == m_playerFaction.name)
				{
					FontAsset(U"small")(U"[クリックして入城]").draw(textX, textY + 145, ColorF(0.7, 0.9, 0.7));
				}
			}
		}

		// 地図の枠線（内側）
		mapArea.drawFrame(4, ColorF(0.3, 0.25, 0.20));
		mapArea.stretched(2).drawFrame(2, ColorF(0.5, 0.4, 0.3));

		// =================================================================
		// 📅 日付表示（右上）
		// =================================================================
		if (m_gameManager)
		{
			const RectF dateRect(Scene::Width() - 260, 60, 220, 90);

			// パネルの影
			dateRect.movedBy(3, 3).draw(ColorF(0, 0, 0, 0.5));

			// パネル本体（木目調）
			dateRect.draw(Arg::top = ColorF(0.18, 0.15, 0.12, 0.95), Arg::bottom = ColorF(0.14, 0.12, 0.10, 0.95));
			dateRect.drawFrame(3, ColorF(0.6, 0.5, 0.35));

			// 装飾
			for (auto corner : { dateRect.tl(), dateRect.tr(), dateRect.bl(), dateRect.br() })
			{
				Circle(corner, 7).draw(ColorF(0.5, 0.4, 0.3));
			}

			// 日付テキスト
			String dateText = U"{}年 {}月"_fmt(m_gameManager->year, m_gameManager->month);
			FontAsset(U"title")(dateText).drawAt(dateRect.center().movedBy(0, -10), ColorF(0.95, 0.9, 0.8));

			// ヒント
			FontAsset(U"small")(U"[Enter] 翌月へ").drawAt(dateRect.center().movedBy(0, 20), ColorF(0.75, 0.75, 0.75));
		}

		// =================================================================
		// 🎆 季節のカットイン演出（控えめに）
		// =================================================================
		if (m_cutInTimer > 0)
		{
			double t = m_cutInTimer / 3.5;
			double alpha = Min(t * 2.0, 1.0);
			if (t < 0.2) alpha = t * 5.0;

			// 画面を少し染める
			{
				ScopedRenderStates2D blend(BlendState::Additive);
				Scene::Rect().draw(m_seasonColor.withAlpha(static_cast<uint8>(0.15 * alpha * 255)));
			}

			// 季節ごとの演出（控えめ）
			if (m_seasonText == U"春")
			{
				for (int i = 0; i < 60; ++i)
				{
					double particleTime = time + i * 10.0;
					double x = Math::Fmod(i * 123.4 + particleTime * 50, static_cast<double>(Scene::Width()));
					double y = Math::Fmod(i * 321.9 + particleTime * 30 + sin(particleTime) * 50, static_cast<double>(Scene::Height()));
					Circle(x, y, 8).draw(ColorF(1.0, 0.7, 0.8, alpha * 0.5));
				}
			}
			else if (m_seasonText == U"夏")
			{
				ScopedRenderStates2D blend(BlendState::Additive);
				for (int i = 0; i < 20; ++i)
				{
					double particleTime = time * 0.5 + i;
					double scale = 1.0 + sin(particleTime * 3) * 0.5;
					Vec2 pos(Scene::Width() / 2 + cos(particleTime) * 400, Scene::Height() / 2 + sin(particleTime) * 200);
					Circle(pos, 40 * scale).draw(ColorF(1.0, 1.0, 0.5, 0.1 * alpha));
				}
			}
			else if (m_seasonText == U"秋")
			{
				for (int i = 0; i < 50; ++i)
				{
					double particleTime = time + i * 5.0;
					double x = Math::Fmod(i * 200 + cos(particleTime) * 100, static_cast<double>(Scene::Width()));
					double y = Math::Fmod(particleTime * 150, static_cast<double>(Scene::Height()));
					Triangle(Vec2(x, y - 8), Vec2(x - 6, y + 4), Vec2(x + 6, y + 4))
						.draw(Palette::Orangered.withAlpha(static_cast<uint8>(alpha * 200)));
				}
			}
			else // 冬
			{
				for (int i = 0; i < 150; ++i)
				{
					double particleTime = time + i;
					double x = Math::Fmod(i * 17.0 + sin(particleTime) * 20, static_cast<double>(Scene::Width()));
					double y = Math::Fmod(particleTime * 80 + i * 50, static_cast<double>(Scene::Height()));
					Circle(x, y, 3).draw(Palette::White.withAlpha(static_cast<uint8>(alpha * 0.7 * 255)));
				}
			}

			// テキスト
			double scale = 1.0 + (1.0 - t) * 0.2;
			{
				const Transformer2D t2d{ Mat3x2::Scale(scale, Scene::Center()) };

				// 影
				for (int layer = 3; layer > 0; --layer)
				{
					FontAsset(U"huge")(m_cutInText).drawAt(
						Scene::Center().movedBy(layer * 2, layer * 2),
						ColorF(0, 0, 0, alpha * 0.4)
					);
				}

				// メインテキスト
				FontAsset(U"huge")(m_cutInText).drawAt(Scene::Center(), ColorF(0.2, 0.2, 0.2, alpha));
			}

			// 季節の印章
			{
				Vec2 stampPos = Scene::Center().movedBy(300, 150);
				Circle(stampPos, 50).draw(m_seasonColor.withAlpha(static_cast<uint8>(alpha * 0.2 * 255)));
				Circle(stampPos, 50).drawFrame(4, m_seasonColor.withAlpha(static_cast<uint8>(alpha * 255)));
				FontAsset(U"title")(m_seasonText).drawAt(stampPos, m_seasonColor.withAlpha(static_cast<uint8>(alpha * 255)));
			}
		}
	}
}
