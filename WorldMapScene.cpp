#include "WorldMapScene.hpp"
#include "CityScene.hpp"

// コンストラクタ
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
	// -----------------------------------------
	// ★ 月を進める処理
	// -----------------------------------------
	if (KeyEnter.down())
	{
		if (m_gameManager && m_allCities)
		{
			// データ更新（収入など）は毎月行う
			m_gameManager->advanceMonth(*m_allCities);

			int m = m_gameManager->month;

			// ★ 修正：防衛戦が発生した場合の処理
			if (m_gameManager->pendingBattle.isOccurring)
			{
				int atkIdx = m_gameManager->pendingBattle.atkCityIndex;
				int defIdx = m_gameManager->pendingBattle.defCityIndex;

				// ★ 修正：スペースを削除して統一
				m_nextScene = U"Battle";
				m_sceneEnd = true;

				Console << U"[防衛戦発生] " << (*m_allCities)[atkIdx].name
					<< U" が " << (*m_allCities)[defIdx].name << U" に侵攻！";
				return;
			}
			// ---------------------------------------------------------
			// ★ 変更点：季節の変わり目（3, 6, 9, 12月）だけ演出を入れる
			// ---------------------------------------------------------
			if (m % 3 == 0)
			{
				m_cutInTimer = 3.5; // 演出開始

				// テキスト作成
				m_cutInText = U"{}年 {}月"_fmt(m_gameManager->year, m_gameManager->month);

				// 季節の色設定
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
				else { // 12月
					m_seasonText = U"冬";
					m_seasonColor = Palette::White;
				}
			}
		}
	}

	// タイマー減少
	m_cutInTimer -= Scene::DeltaTime();
	if (m_cutInTimer < 0) m_cutInTimer = 0;

	// -----------------------------------------
	// 都市選択処理（演出中は操作禁止）
	// -----------------------------------------
	if (m_cutInTimer <= 0)
	{
		m_hovered = -1;
		if (m_allCities)
		{
			for (int i = 0; i < m_allCities->size(); ++i)
			{
				const CityData& c = (*m_allCities)[i];
				if (Circle(c.pos, 20).mouseOver())
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
	Scene::SetBackground(ColorF{ 0.2, 0.25, 0.2 });

	// -----------------------------------------
	// 都市描画
	// -----------------------------------------
	if (m_allCities)
	{
		for (const auto& c : *m_allCities)
		{
			bool myCity = (c.owner == m_playerFaction.name);
			ColorF dotColor = myCity ? ColorF(m_playerFaction.color) : ColorF(c.color).withA(0.4);
			Circle{ c.pos, 16 }.draw(dotColor);
			FontAsset(U"small")(c.name).drawAt(c.pos.x, c.pos.y - 25);
		}

		// ホバー情報
		if (m_hovered >= 0 && m_cutInTimer <= 0)
		{
			const CityData& c = (*m_allCities)[m_hovered];
			RectF(20, 20, 350, 140).draw(ColorF(0, 0, 0, 0.6));
			FontAsset(U"small")(U"都市: " + c.name).draw(30, 30);
			FontAsset(U"small")(U"金: " + Format(c.gold)).draw(30, 60);
			FontAsset(U"small")(U"兵: " + Format(c.troops)).draw(150, 60);
		}
	}

	// -----------------------------------------
	// 日付表示（右上の小さいやつ）
	// -----------------------------------------
	if (m_gameManager)
	{
		const Rect dateRect(Scene::Width() - 220, 10, 200, 80);
		dateRect.draw(ColorF{ 0, 0, 0, 0.6 }).drawFrame(2, Palette::Gold);
		FontAsset(U"medium")(U"{}年 {}月"_fmt(m_gameManager->year, m_gameManager->month))
			.drawAt(dateRect.center().movedBy(0, -10), Palette::White);
	}

	// =================================================================
	// ★ 季節の豪華カットイン演出
	// =================================================================
	if (m_cutInTimer > 0)
	{
		// 進行度
		double t = m_cutInTimer / 3.5;
		double alpha = Min(t * 2.0, 1.0);
		if (t < 0.2) alpha = t * 5.0;

		// 1. 画面全体を染める
		{
			ScopedRenderStates2D blend(BlendState::Additive);
			Scene::Rect().draw(m_seasonColor.withA(0.3 * alpha));
		}

		// 2. 季節ごとの演出
		if (m_seasonText == U"春") // 桜
		{
			for (int i = 0; i < 80; ++i)
			{
				double time = Scene::Time() + i * 10.0;
				double x = Math::Fmod(i * 123.4 + time * 50, Scene::Width());
				double y = Math::Fmod(i * 321.9 + time * 30 + sin(time) * 50, Scene::Height());
				RectF(10, 15).rotated(time * 2).movedBy(x, y).draw(Palette::Pink.withA(alpha));
			}
		}
		else if (m_seasonText == U"夏") // 光
		{
			ScopedRenderStates2D blend(BlendState::Additive);
			for (int i = 0; i < 20; ++i)
			{
				double time = Scene::Time() * 0.5 + i;
				double scale = 1.0 + sin(time * 3) * 0.5;
				Vec2 pos(Scene::Width() / 2 + cos(time) * 400, Scene::Height() / 2 + sin(time) * 200);
				Circle(pos, 50 * scale).draw(ColorF(1.0, 1.0, 0.5, 0.2 * alpha));
				RectF(0, Math::Fmod(i * 100 + time * 50, Scene::Height()), Scene::Width(), 5)
					.draw(ColorF(1.0, 1.0, 1.0, 0.1 * alpha));
			}
		}
		else if (m_seasonText == U"秋") // 紅葉
		{
			for (int i = 0; i < 80; ++i)
			{
				double time = Scene::Time() + i * 5.0;
				double x = Math::Fmod(i * 200 + cos(time) * 100, Scene::Width());
				double y = Math::Fmod(time * 150, Scene::Height());
				Color leafColor = (i % 2 == 0) ? Palette::Orangered : Palette::Gold;
				Triangle(20).rotated(time * 3).movedBy(x, y).draw(leafColor.withA(alpha));
			}
		}
		else // 冬: 雪
		{
			for (int i = 0; i < 150; ++i)
			{
				double time = Scene::Time() + i;
				double x = Math::Fmod(i * 17.0 + sin(time) * 20, Scene::Width());
				double y = Math::Fmod(time * 80 + i * 50, Scene::Height());
				Circle(x, y, 3 + sin(time * 5)).draw(Palette::White.withA(alpha * 0.8));
			}
		}

		// 3. テキストアニメーション
		double scale = 1.0 + (1.0 - t) * 0.3;
		{
			const Transformer2D t2d{ Mat3x2::Scale(scale, Scene::Center()) };
			FontAsset(U"huge")(m_cutInText).drawAt(Scene::Center().movedBy(4, 4), ColorF(0, 0, 0, alpha));
			FontAsset(U"huge")(m_cutInText).drawAt(Scene::Center(), Palette::White.withA(alpha));
		}

		// 4. ハンコ
		FontAsset(U"huge")(m_seasonText).drawAt(Scene::Center().movedBy(300, 150), m_seasonColor.withA(alpha));
	}
}
