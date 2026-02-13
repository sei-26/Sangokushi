#include "CityScene.hpp"

CityScene::CityScene(GameManager* gm, CityData& city)
	: m_gameManager(gm)
	, m_cityData(&city)
{
	m_message = U"ようこそ " + m_cityData->name + U" へ。\n指示をください、殿。";

	// ボタン配置（左側）
	int startX = 60;
	int startY = 280;
	int btnW = 280;
	int btnH = 65;
	int gap = 85;

	m_btnAgr = Rect(startX, startY + gap * 0, btnW, btnH);
	m_btnCom = Rect(startX, startY + gap * 1, btnW, btnH);
	m_btnTrain = Rect(startX, startY + gap * 2, btnW, btnH);
	m_btnOrder = Rect(startX, startY + gap * 3, btnW, btnH);

	// 出陣ボタンは右下に大きく
	m_btnAttack = Rect(Scene::Width() - 350, Scene::Height() - 180, 300, 120);

	// 戻るボタン
	m_btnBack = Rect(50, Scene::Height() - 100, 200, 70);
}

void CityScene::update()
{
	// 農業
	if (m_btnAgr.leftClicked())
	{
		if (m_cityData->gold >= 100)
		{
			m_cityData->gold -= 100;
			m_cityData->agriculture += 10;
			m_message = U"農業開発を行いました。\n農業値が 10 上昇！\n来年の収穫が楽しみですな。";
		}
		else m_message = U"金が足りません！\n商業を発展させましょう。";
	}

	// 商業
	if (m_btnCom.leftClicked())
	{
		if (m_cityData->gold >= 100)
		{
			m_cityData->gold -= 100;
			m_cityData->commerce += 10;
			m_message = U"商業投資を行いました。\n商業値が 10 上昇！\n市場が賑わっております。";
		}
		else m_message = U"金が足りません！\n交易路を開拓しましょう。";
	}

	// 徴兵
	if (m_btnTrain.leftClicked())
	{
		if (m_cityData->food >= 100)
		{
			m_cityData->food -= 100;
			m_cityData->troops += 200;
			m_message = U"徴兵を行いました。\n兵士が 200 増えました！\n我が軍の士気は高まっております。";
		}
		else m_message = U"兵糧が足りません！\n農地を開拓し、収穫を増やしましょう。";
	}

	// 治安
	if (m_btnOrder.leftClicked())
	{
		if (m_cityData->gold >= 50)
		{
			m_cityData->gold -= 50;
			m_cityData->order += 5;
			if (m_cityData->order > 100) m_cityData->order = 100;
			m_message = U"巡回を行いました。\n治安が改善しました。\n民は安心して暮らせます。";
		}
		else m_message = U"金が足りません！\n税収を増やしましょう。";
	}

	// 出陣
	if (m_btnAttack.leftClicked())
	{
		if (m_cityData->troops >= 1000)
		{
			m_sceneEnd = true;
			m_nextScene = U"Battle";
		}
		else
		{
			m_message = U"兵士が足りません！\n最低でも 1000 人は必要です。\n徴兵を行いましょう。";
		}
	}

	// 戻る
	if (m_btnBack.leftClicked())
	{
		m_sceneEnd = true;
		m_nextScene = U"WorldMap";
	}
}

void CityScene::draw() const
{
	double time = Scene::Time();

	// =================================================================
	// 🎨 背景：豪華絢爛な宮殿
	// =================================================================
	Scene::SetBackground(ColorF{ 0.08, 0.06, 0.05 });

	// 石壁のテクスチャ
	for (int i = 0; i < 100; ++i)
	{
		double x = RandomClosed(0.0, static_cast<double>(Scene::Width()));
		double y = RandomClosed(0.0, static_cast<double>(Scene::Height()));
		RectF(x, y, RandomClosed(30.0, 80.0), RandomClosed(30.0, 80.0)).draw(ColorF(0.15, 0.13, 0.11, 0.3));
	}

	// ✨ 金色のパーティクル（煌めく光の粒）
	{
		ScopedRenderStates2D blend(BlendState::Additive);
		for (int i = 0; i < 80; ++i)
		{
			double particleTime = time * 0.5 + i * 0.1;
			double x = Math::Fmod(i * 47.3 + sin(particleTime) * 200, Scene::Width());
			double y = Math::Fmod(particleTime * 30 + i * 13.7, Scene::Height());
			double size = 2 + sin(particleTime * 3) * 1.5;
			double alpha = 0.3 + sin(particleTime * 5) * 0.2;

			Circle(x, y, size).draw(ColorF(1.0, 0.9, 0.5, alpha));
		}
	}

	// 🔥 松明の炎エフェクト
	{
		ScopedRenderStates2D blend(BlendState::Additive);
		for (int i = 0; i < 6; ++i)
		{
			double x = (i % 3) * 600.0 + 200;
			double y = (i / 3) * 500.0 + 150;

			// 炎の揺らめき
			for (int flame = 0; flame < 5; ++flame)
			{
				double flameTime = time * 4 + i + flame * 0.3;
				double flameY = y + sin(flameTime) * 15 - flame * 8;
				double flameSize = 25 - flame * 4;
				double flameAlpha = 0.4 - flame * 0.07;

				Circle(x + sin(flameTime * 2) * 5, flameY, flameSize)
					.draw(ColorF(1.0, 0.6 - flame * 0.1, 0.2, flameAlpha));
			}
		}
	}

	// 柱（左右）- より豪華に
	for (int side = 0; side < 2; ++side)
	{
		double x = side == 0 ? 50.0 : Scene::Width() - 80.0;

		// 柱の影
		RectF(x + 3, 0, 30, Scene::Height()).draw(ColorF(0, 0, 0, 0.3));

		// 柱本体（グラデーション）
		RectF(x, 0, 30, Scene::Height())
			.draw(Arg::left = ColorF(0.5, 0.4, 0.3), Arg::right = ColorF(0.3, 0.25, 0.2));

		// 🌟 柱の金色装飾
		for (int i = 0; i < 5; ++i)
		{
			double y = i * 200.0 + 100;

			// 装飾プレート
			RectF(x - 10, y, 50, 20).draw(ColorF(0.7, 0.6, 0.4));
			RectF(x - 10, y, 50, 20).drawFrame(2, ColorF(1.0, 0.9, 0.6));

			// 宝石風の装飾
			Circle(x + 15, y + 10, 6).draw(ColorF(0.8, 0.2, 0.2, 0.7));
			Circle(x + 15, y + 10, 6).drawFrame(1, ColorF(1.0, 0.5, 0.5));
		}
	}

	// =================================================================
	// 👑 タイトル（超豪華版）
	// =================================================================
	{
		Vec2 titlePos(Scene::Center().x, 80);

		// 🌟 タイトル背後の光芒
		{
			ScopedRenderStates2D blend(BlendState::Additive);
			for (int ray = 0; ray < 12; ++ray)
			{
				double angle = (time * 0.3 + ray * Math::TwoPi / 12);
				double rayLength = 250 + sin(time * 2 + ray) * 30;
				Vec2 rayEnd = titlePos + Vec2(Cos(angle), Sin(angle)) * rayLength;

				Triangle(
					titlePos,
					titlePos + Vec2(Cos(angle - 0.05), Sin(angle - 0.05)) * rayLength,
					titlePos + Vec2(Cos(angle + 0.05), Sin(angle + 0.05)) * rayLength
				).draw(ColorF(1.0, 0.9, 0.5, 0.1));
			}
		}

		// タイトル背景パネル
		RectF titlePanel(Arg::center(titlePos), 600, 100);

		// パネルの発光
		{
			ScopedRenderStates2D blend(BlendState::Additive);
			titlePanel.stretched(5).draw(ColorF(1.0, 0.8, 0.3, 0.2));
		}

		titlePanel.draw(Arg::top = ColorF(0.25, 0.20, 0.15, 0.95), Arg::bottom = ColorF(0.18, 0.15, 0.12, 0.95));
		titlePanel.drawFrame(5, ColorF(1.0, 0.85, 0.5));

		// 装飾的な角（宝石風）
		for (auto corner : { titlePanel.tl(), titlePanel.tr(), titlePanel.bl(), titlePanel.br() })
		{
			Circle(corner, 15).draw(ColorF(0.9, 0.7, 0.4));
			Circle(corner, 12).draw(ColorF(1.0, 0.9, 0.6));
			Circle(corner, 8).draw(ColorF(1.0, 1.0, 0.8, 0.8 + sin(time * 3) * 0.2));
		}

		// タイトルテキスト（多重影で立体感）
		for (int layer = 5; layer > 0; --layer)
		{
			FontAsset(U"huge")(m_cityData->name).drawAt(
				titlePos.movedBy(layer, layer),
				ColorF(0, 0, 0, 0.15)
			);
		}
		FontAsset(U"huge")(m_cityData->name).drawAt(titlePos, ColorF(1.0, 0.95, 0.7));

		// ✨ タイトルの輝き
		{
			ScopedRenderStates2D blend(BlendState::Additive);
			FontAsset(U"huge")(m_cityData->name).drawAt(titlePos, ColorF(1.0, 1.0, 0.8, 0.3));
		}

		// サブタイトル
		FontAsset(U"menu")(U"～ " + m_cityData->owner + U" の治世 ～")
			.drawAt(titlePos.movedBy(0, 45), ColorF(1.0, 0.9, 0.6));
	}

	// =================================================================
	// 📊 ステータスパネル（宝石のように輝く）
	// =================================================================
	{
		RectF statsPanel(Scene::Width() - 550, 200, 500, 450);

		// パネルの輝き
		{
			ScopedRenderStates2D blend(BlendState::Additive);
			statsPanel.stretched(3).draw(ColorF(1.0, 0.8, 0.5, 0.15));
		}

		// パネルの影
		statsPanel.movedBy(4, 4).draw(ColorF(0, 0, 0, 0.5));

		// パネル本体
		statsPanel.draw(Arg::top = ColorF(0.2, 0.17, 0.14, 0.95), Arg::bottom = ColorF(0.14, 0.12, 0.10, 0.95));
		statsPanel.drawFrame(4, ColorF(0.9, 0.75, 0.5));

		// 装飾（煌めく宝石）
		for (auto corner : { statsPanel.tl(), statsPanel.tr(), statsPanel.bl(), statsPanel.br() })
		{
			Circle(corner, 12).draw(ColorF(0.8, 0.6, 0.4));
			Circle(corner, 10).draw(ColorF(1.0, 0.85, 0.6, 0.7 + sin(time * 4) * 0.3));
		}

		int statX = static_cast<int>(statsPanel.x) + 30;
		int statY = static_cast<int>(statsPanel.y) + 30;

		// タイトル
		FontAsset(U"title")(U"都市情報").draw(statX, statY, ColorF(1.0, 0.95, 0.7));

		// パラメータ表示関数
		auto drawParameter = [&](String icon, String name, int value, int maxValue, Color barColor, int y) {
			// アイコン背景（輝く）
			{
				ScopedRenderStates2D blend(BlendState::Additive);
				Circle(static_cast<double>(statX + 20), static_cast<double>(y + 20), 22.0)
					.draw(barColor.withAlpha(static_cast<uint8>(50)));
			}

			Circle(static_cast<double>(statX + 20), static_cast<double>(y + 20), 18.0).draw(ColorF(0.3, 0.25, 0.2));
			Circle(static_cast<double>(statX + 20), static_cast<double>(y + 20), 18.0).drawFrame(3, barColor);
			FontAsset(U"menu")(icon).drawAt(statX + 20, y + 20, barColor);

			// 名前と数値
			FontAsset(U"menu")(name).draw(statX + 50, y + 10, Palette::White);
			FontAsset(U"title")(Format(value)).draw(statX + 180, y + 5, barColor);

			// バー（背景）
			RectF barBg(static_cast<double>(statX + 50), static_cast<double>(y + 40), 400.0, 20.0);
			barBg.draw(ColorF(0.1, 0.08, 0.06));
			barBg.drawFrame(2, ColorF(0.5, 0.4, 0.3));

			// バー（実際の値 - 虹色グラデーション）
			double ratio = Min(1.0, static_cast<double>(value) / maxValue);
			RectF bar(static_cast<double>(statX + 50), static_cast<double>(y + 40), 400.0 * ratio, 20.0);
			bar.draw(Arg::left = barColor, Arg::right = barColor.lerp(Palette::White, 0.3));

			// ✨ バーの輝き
			{
				ScopedRenderStates2D blend(BlendState::Additive);
				RectF(static_cast<double>(statX + 50), static_cast<double>(y + 40), 400.0 * ratio, 10.0)
					.draw(ColorF(1, 1, 1, 0.4));

				// 流れる光
				double flowPos = Math::Fmod(time * 200, 400.0 * ratio);
				Circle(statX + 50 + flowPos, y + 50, 8).draw(ColorF(1, 1, 1, 0.6));
			}
			};

		drawParameter(U"$", U"金", m_cityData->gold, 5000, Palette::Gold, statY + 60);
		drawParameter(U"米", U"兵糧", m_cityData->food, 5000, Palette::Orange, statY + 140);
		drawParameter(U"兵", U"兵士", m_cityData->troops, 10000, Palette::Red, statY + 220);
		drawParameter(U"安", U"治安", m_cityData->order, 100, Palette::Skyblue, statY + 300);

		// 発展度
		FontAsset(U"small")(U"農業: {} / 商業: {} / 兵舎: {}"_fmt(
			m_cityData->agriculture, m_cityData->commerce, m_cityData->barracks))
			.draw(statX + 10, statY + 390, ColorF(0.8, 0.8, 0.8));
	}

	// =================================================================
	// 🎮 内政ボタン群（超煌びやか）
	// =================================================================

	auto drawFancyButton = [&](const Rect& btn, String text, String subtext, Color mainColor, bool isHovered) {
		// ホバー時の輝き
		if (isHovered)
		{
			ScopedRenderStates2D blend(BlendState::Additive);
			RectF(btn).stretched(10).draw(mainColor.withAlpha(static_cast<uint8>(80)));
		}

		// ボタンの影
		btn.movedBy(4, 4).draw(ColorF(0, 0, 0, 0.5));

		// ボタン本体
		Color topColor = isHovered ? mainColor.lerp(Palette::White, 0.4) : mainColor;
		Color bottomColor = isHovered ? mainColor : mainColor.lerp(Palette::Black, 0.4);


		RectF(btn).draw(Arg::top = topColor, Arg::bottom = bottomColor);

		// 金色の枠
		RectF(btn).drawFrame(4, isHovered ? ColorF(1, 1, 0.9) : ColorF(0.8, 0.7, 0.5));

		// 光沢（上部ハイライト）
		RectF(static_cast<double>(btn.x), static_cast<double>(btn.y),
			  static_cast<double>(btn.w), static_cast<double>(btn.h) * 0.35)
			.draw(ColorF(1, 1, 1, isHovered ? 0.3 : 0.2));

		// テキスト（多重影）
		FontAsset(U"title")(text).drawAt(btn.center().movedBy(3, -7), ColorF(0, 0, 0, 0.6));
		FontAsset(U"title")(text).drawAt(btn.center().movedBy(0, -10), Palette::White);

		// サブテキスト
		FontAsset(U"small")(subtext).drawAt(btn.center().movedBy(0, 15), ColorF(1, 1, 1, 0.9));

		// ホバー時のキラキラ
		if (isHovered)
		{
			ScopedRenderStates2D blend(BlendState::Additive);
			RectF(btn).drawFrame(3, ColorF(1, 1, 0.7, 0.6 + sin(time * 8) * 0.4));

			// 四隅の星
			for (auto corner : { btn.tl(), btn.tr(), btn.bl(), btn.br() })
			{
				Circle(corner, 5).draw(ColorF(1, 1, 0.8, 0.8));
			}
		}
		};

	drawFancyButton(m_btnAgr, U"農業開発", U"金 100 消費", ColorF(0.3, 0.7, 0.4), m_btnAgr.mouseOver());
	drawFancyButton(m_btnCom, U"商業投資", U"金 100 消費", ColorF(0.4, 0.5, 0.9), m_btnCom.mouseOver());
	drawFancyButton(m_btnTrain, U"徴兵", U"兵糧 100 消費", ColorF(0.9, 0.4, 0.4), m_btnTrain.mouseOver());
	drawFancyButton(m_btnOrder, U"治安維持", U"金 50 消費", ColorF(0.6, 0.6, 0.7), m_btnOrder.mouseOver());

	// =================================================================
	// ⚔️ 出陣ボタン（究極の豪華さ）
	// =================================================================
	{
		bool isHovered = m_btnAttack.mouseOver();
		double pulse = 1.0 + sin(time * 3) * 0.08;

		// 🌟 背後の光の爆発
		{
			ScopedRenderStates2D blend(BlendState::Additive);
			for (int ring = 0; ring < 4; ++ring)
			{
				double ringSize = (ring + 1) * 25.0 + sin(time * 3 + ring) * 10;
				RectF(m_btnAttack).stretched(ringSize)
					.draw(ColorF(1.0, 0.5, 0.2, 0.15 - ring * 0.03));
			}
		}

		// ボタンの影
		m_btnAttack.movedBy(6, 6).draw(ColorF(0, 0, 0, 0.6));

		// ボタン本体（炎のようなグラデーション）
		Color topColor = ColorF(1.0, 0.3, 0.2) * pulse;
		Color bottomColor = ColorF(0.7, 0.15, 0.05) * pulse;

		if (isHovered)
		{
			topColor = ColorF(1.0, 0.5, 0.4);
			bottomColor = ColorF(0.9, 0.3, 0.15);
		}

		RectF(m_btnAttack).draw(Arg::top = topColor, Arg::bottom = bottomColor);

		// 金色の枠（脈動）
		RectF(m_btnAttack).drawFrame(6, ColorF(1.0, 0.9, 0.4, 0.9 + sin(time * 5) * 0.1));

		// 光沢
		RectF(static_cast<double>(m_btnAttack.x), static_cast<double>(m_btnAttack.y),
			  static_cast<double>(m_btnAttack.w), static_cast<double>(m_btnAttack.h) * 0.4)
			.draw(ColorF(1, 1, 1, 0.4));

		// テキスト（多重影で立体感）
		Vec2 textPos = m_btnAttack.center();
		for (int layer = 6; layer > 0; --layer)
		{
			FontAsset(U"huge")(U"出 陣").drawAt(
				textPos.movedBy(layer, layer),
				ColorF(0.2, 0, 0, 0.15)
			);
		}
		FontAsset(U"huge")(U"出 陣").drawAt(textPos, ColorF(1.0, 1.0, 0.9));

		// テキストの輝き
		{
			ScopedRenderStates2D blend(BlendState::Additive);
			FontAsset(U"huge")(U"出 陣").drawAt(textPos, ColorF(1.0, 1.0, 0.8, 0.5));
		}

		// 剣のアイコン（両脇）
		for (int side = -1; side <= 1; side += 2)
		{
			Vec2 swordPos = textPos.movedBy(side * 120.0, 0);

			// 剣の刃
			Line(swordPos.movedBy(0, -25), swordPos.movedBy(0, 20))
				.draw(10, ColorF(0.9, 0.9, 1.0));

			// 剣の先端
			Vec2 tip = swordPos.movedBy(0, -30);
			Triangle(tip, tip.movedBy(-8, 12), tip.movedBy(8, 12))
				.draw(ColorF(1.0, 1.0, 1.0));

			// 剣の輝き
			{
				ScopedRenderStates2D blend(BlendState::Additive);
				Line(swordPos.movedBy(0, -25), swordPos.movedBy(0, 20))
					.draw(4, ColorF(1, 1, 1, 0.6));
			}
		}

		// ホバー時のエフェクト
		if (isHovered)
		{
			ScopedRenderStates2D blend(BlendState::Additive);

			// 外側の光の層
			for (int i = 0; i < 5; ++i)
			{
				double offset = i * 8.0 + sin(time * 5 + i) * 3;
				RectF(m_btnAttack).stretched(offset)
					.drawFrame(3, ColorF(1, 0.7, 0.3, 0.5 - i * 0.1));
			}

			// キラキラの粒子
			for (int i = 0; i < 20; ++i)
			{
				double angle = time * 2 + i * Math::TwoPi / 20;
				double dist = 180 + sin(time * 4 + i) * 20;
				Vec2 sparklePos = textPos + Vec2(Cos(angle), Sin(angle)) * dist;

				Circle(sparklePos, 4).draw(ColorF(1, 1, 0.8, 0.8));
			}
		}
	}

	// =================================================================
	// 🚪 戻るボタン
	// =================================================================
	{
		bool isHovered = m_btnBack.mouseOver();

		m_btnBack.movedBy(2, 2).draw(ColorF(0, 0, 0, 0.4));
		RectF(m_btnBack).draw(isHovered ? ColorF(0.35, 0.30, 0.25) : ColorF(0.25, 0.20, 0.15));
		RectF(m_btnBack).drawFrame(3, ColorF(0.7, 0.6, 0.4));

		FontAsset(U"menu")(U"← マップに戻る").drawAt(m_btnBack.center().movedBy(1, 1), ColorF(0, 0, 0, 0.5));
		FontAsset(U"menu")(U"← マップに戻る").drawAt(m_btnBack.center(), Palette::White);

		if (isHovered)
		{
			ScopedRenderStates2D blend(BlendState::Additive);
			RectF(m_btnBack).drawFrame(2, ColorF(1, 1, 0.8, 0.4));
		}
	}

	// =================================================================
	// 💬 メッセージパネル（巻物風）
	// =================================================================
	{
		RectF msgPanel(100, Scene::Height() - 250, Scene::Width() - 550, 140);

		// パネルの影
		msgPanel.movedBy(3, 3).draw(ColorF(0, 0, 0, 0.5));

		// パネル本体（巻物風）
		msgPanel.draw(Arg::top = ColorF(0.98, 0.94, 0.85, 0.95), Arg::bottom = ColorF(0.92, 0.88, 0.78, 0.95));
		msgPanel.drawFrame(4, ColorF(0.7, 0.6, 0.4));

		// 巻物の端の装飾
		for (auto corner : { msgPanel.tl(), msgPanel.tr() })
		{
			Circle(corner, 18).draw(ColorF(0.6, 0.5, 0.4));
			Circle(corner, 15).draw(ColorF(0.7, 0.6, 0.5));
		}

		// メッセージテキスト
		FontAsset(U"menu")(m_message).draw(msgPanel.x + 30, msgPanel.y + 25, ColorF(0.15, 0.12, 0.08));
	}
}
