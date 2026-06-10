#include "CityScene.hpp"
#include "OfficerPortrait.hpp"

CityScene::CityScene(GameManager* gm, CityData& city)
	: m_gameManager(gm)
	, m_cityData(&city)
{
	m_message = U"ようこそ " + m_cityData->name + U" へ。\n指示をください、殿。";

	// ★ ボタンを2列配置でコンパクトに
	int screenW = Scene::Width();
	int screenH = Scene::Height();

	// ボタンサイズ（小さく）
	int btnW = static_cast<int>(Min(screenW * 0.12, 200.0));
	int btnH = static_cast<int>(Min(screenH * 0.055, 55.0));
	int startX = static_cast<int>(screenW * 0.05);  // 左端から5%
	int startY = static_cast<int>(screenH * 0.25);   // 上から25%
	int gapX = btnW + 15;  // 横の間隔
	int gapY = static_cast<int>(Min(screenH * 0.07, 70.0));  // 縦の間隔

	// 左列
	m_btnAgr = Rect(startX, startY + gapY * 0, btnW, btnH);
	m_btnCom = Rect(startX, startY + gapY * 1, btnW, btnH);
	m_btnTrain = Rect(startX, startY + gapY * 2, btnW, btnH);

	// 右列
	m_btnOrder = Rect(startX + gapX, startY + gapY * 0, btnW, btnH);
	m_btnOfficer = Rect(startX + gapX, startY + gapY * 1, btnW, btnH);
	m_btnFacility = Rect(startX + gapX, startY + gapY * 2, btnW, btnH);  // ★ 施設ボタン追加

	// 出陣ボタンは右下に
	int attackBtnW = static_cast<int>(Min(screenW * 0.175, 300.0));
	int attackBtnH = static_cast<int>(Min(screenH * 0.12, 120.0));
	m_btnAttack = Rect(screenW - attackBtnW - static_cast<int>(screenW * 0.05),
					   screenH - attackBtnH - static_cast<int>(screenH * 0.08),
					   attackBtnW,
					   attackBtnH);

	// 戻るボタン
	int backBtnW = static_cast<int>(Min(screenW * 0.12, 200.0));
	int backBtnH = static_cast<int>(Min(screenH * 0.07, 70.0));
	m_btnBack = Rect(static_cast<int>(screenW * 0.05),
					 screenH - backBtnH - static_cast<int>(screenH * 0.05),
					 backBtnW,
					 backBtnH);
}

void CityScene::update()
{
	// ★ 重要：都市の所有者が変わっていたら強制的にWorldMapに戻す
	if (m_cityData->owner != m_gameManager->playerFactionName)
	{
		m_message = U"⚠️ この都市は敵に奪われました！";
		Print << U"[警告] " << m_cityData->name << U" は " << m_cityData->owner << U" に占領されました";
		m_sceneEnd = true;
		m_nextScene = U"WorldMap";
		return;
	}

	// ========================================
	// 武将ボーナス計算
	// ========================================
	int administrationBonus = 0;
	if (!m_cityData->officers.isEmpty())
	{
		// 最初の武将の能力を使用
		const Officer& officer = m_cityData->officers[0];
		administrationBonus = officer.GetAdministrationPower() / 10;
	}

	// ========================================
	// 農業（武将の能力でボーナス）
	// ========================================
	if (m_btnAgr.leftClicked())
	{
		// ★ コマンド制限チェック
		if (!m_gameManager->turnManager.CanExecuteCommand())
		{
			m_message = U"今月のコマンドを使い切りました！\n（残り0コマンド）";
		}
		else if (m_cityData->gold >= 100)
		{
			m_cityData->gold -= 100;
			int bonus = 10 + administrationBonus;
			m_cityData->agriculture += bonus;
			m_gameManager->turnManager.ExecuteCommand();  // ★ コマンド使用

			int remaining = m_gameManager->turnManager.GetRemainingCommands();
			m_message = U"農業開発を行いました。\n農業値が " + Format(bonus) + U" 上昇！";
			if (administrationBonus > 0)
			{
				m_message += U"\n（武将ボーナス: +" + Format(administrationBonus) + U"）";
			}
			m_message += U"\n（残り" + Format(remaining) + U"コマンド）";
		}
		else m_message = U"金が足りません！";
	}

	// ========================================
	// 商業（武将の能力でボーナス）
	// ========================================
	if (m_btnCom.leftClicked())
	{
		// ★ コマンド制限チェック
		if (!m_gameManager->turnManager.CanExecuteCommand())
		{
			m_message = U"今月のコマンドを使い切りました！\n（残り0コマンド）";
		}
		else if (m_cityData->gold >= 100)
		{
			m_cityData->gold -= 100;
			int bonus = 10 + administrationBonus;
			m_cityData->commerce += bonus;
			m_gameManager->turnManager.ExecuteCommand();  // ★ コマンド使用

			int remaining = m_gameManager->turnManager.GetRemainingCommands();
			m_message = U"商業投資を行いました。\n商業値が " + Format(bonus) + U" 上昇！";
			if (administrationBonus > 0)
			{
				m_message += U"\n（武将ボーナス: +" + Format(administrationBonus) + U"）";
			}
			m_message += U"\n（残り" + Format(remaining) + U"コマンド）";
		}
		else m_message = U"金が足りません！";
	}

	// ========================================
	// 徴兵
	// ========================================
	if (m_btnTrain.leftClicked())
	{
		// ★ コマンド制限チェック
		if (!m_gameManager->turnManager.CanExecuteCommand())
		{
			m_message = U"今月のコマンドを使い切りました！\n（残り0コマンド）";
		}
		else if (m_cityData->food >= 100)
		{
			m_cityData->food -= 100;
			m_cityData->troops += 200;
			m_gameManager->turnManager.ExecuteCommand();  // ★ コマンド使用

			int remaining = m_gameManager->turnManager.GetRemainingCommands();
			m_message = U"徴兵を行いました。\n兵士が 200 増えました！";
			m_message += U"\n（残り" + Format(remaining) + U"コマンド）";
		}
		else m_message = U"兵糧が足りません！";
	}

	// ========================================
	// 治安（武将の政治力でボーナス）
	// ========================================
	if (m_btnOrder.leftClicked())
	{
		// ★ コマンド制限チェック
		if (!m_gameManager->turnManager.CanExecuteCommand())
		{
			m_message = U"今月のコマンドを使い切りました！\n（残り0コマンド）";
		}
		else if (m_cityData->gold >= 50)
		{
			m_cityData->gold -= 50;
			int orderBonus = 5 + (administrationBonus / 2);
			m_cityData->order += orderBonus;
			if (m_cityData->order > 100) m_cityData->order = 100;
			m_gameManager->turnManager.ExecuteCommand();  // ★ コマンド使用

			int remaining = m_gameManager->turnManager.GetRemainingCommands();
			m_message = U"巡回を行いました。\n治安が " + Format(orderBonus) + U" 改善！";
			if (administrationBonus > 0)
			{
				m_message += U"\n（武将ボーナス: +" + Format(administrationBonus / 2) + U"）";
			}
			m_message += U"\n（残り" + Format(remaining) + U"コマンド）";
		}
		else m_message = U"金が足りません！";
	}

	// ========================================
	// ★ 人材（武将管理画面へ）
	// ========================================
	if (m_btnOfficer.leftClicked())
	{
		m_sceneEnd = true;
		m_nextScene = U"OfficerManagement";
	}

	// ========================================
	// ★ 施設（施設建設画面へ）
	// ========================================
	if (m_btnFacility.leftClicked())
	{
		m_sceneEnd = true;
		m_nextScene = U"Facility";
	}

	// ========================================
	// 出陣
	// ========================================
	if (m_btnAttack.leftClicked())
	{
		if (m_cityData->troops >= 1000)
		{
			m_sceneEnd = true;
			m_nextScene = U"Battle";
		}
		else
		{
			m_message = U"兵士が足りません！\n最低でも 1000 人は必要です。";
		}
	}

	// ========================================
	// 戻る
	// ========================================
	if (m_btnBack.leftClicked())
	{
		m_sceneEnd = true;
		m_nextScene = U"WorldMap";
	}
}


void CityScene::draw() const
{
	double time = Scene::Time();

	// ★ 画面サイズを取得
	int screenW = Scene::Width();
	int screenH = Scene::Height();

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

	// 🔥 松明の炎エフェクト（画面内に配置）
	{
		ScopedRenderStates2D blend(BlendState::Additive);

		// 松明の数と位置を画面サイズに応じて調整
		int torchCount = Min(6, static_cast<int>(screenW / 300));
		for (int i = 0; i < torchCount; ++i)
		{
			double x = (i % 3) * (screenW / 3.0) + screenW / 6.0;
			double y = (i / 3) * (screenH / 2.0) + screenH / 4.0;

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

	// 柱（画面端に配置）
	for (int side = 0; side < 2; ++side)
	{
		double x = side == 0 ? screenW * 0.03 : screenW - screenW * 0.05;

		// 柱の影
		RectF(x + 3, 0, 30, screenH).draw(ColorF(0, 0, 0, 0.3));

		// 柱本体
		RectF(x, 0, 30, screenH)
			.draw(Arg::left = ColorF(0.5, 0.4, 0.3), Arg::right = ColorF(0.3, 0.25, 0.2));

		// 🌟 柱の金色装飾
		int decorCount = Max(3, static_cast<int>(screenH / 250));
		for (int i = 0; i < decorCount; ++i)
		{
			double y = (i + 1) * (screenH / (decorCount + 1.0));

			RectF(x - 10, y - 10, 50, 20).draw(ColorF(0.7, 0.6, 0.4));
			RectF(x - 10, y - 10, 50, 20).drawFrame(2, ColorF(1.0, 0.9, 0.6));

			Circle(x + 15, y, 6).draw(ColorF(0.8, 0.2, 0.2, 0.7));
			Circle(x + 15, y, 6).drawFrame(1, ColorF(1.0, 0.5, 0.5));
		}
	}

	// =================================================================
	// 👑 タイトル（コンパクト版）
	// =================================================================
	{
		Vec2 titlePos(screenW * 0.25, screenH * 0.08);  // 左寄り、上に

		// タイトル背景パネル（小さく）
		RectF titlePanel(Arg::center(titlePos), screenW * 0.25, screenH * 0.08);

		titlePanel.draw(Arg::top = ColorF(0.25, 0.20, 0.15, 0.95), Arg::bottom = ColorF(0.18, 0.15, 0.12, 0.95));
		titlePanel.drawFrame(3, ColorF(1.0, 0.85, 0.5));

		// タイトルテキスト
		FontAsset(U"title")(m_cityData->name).drawAt(titlePos, ColorF(1.0, 0.95, 0.7));

		// サブタイトル（所有者）
		FontAsset(U"menu")(U"～ " + m_cityData->owner + U" の治世 ～")
			.drawAt(titlePos.movedBy(0, screenH * 0.04), ColorF(1.0, 0.9, 0.6));
	}

	// =================================================================
	// ⏰ 残りコマンド数表示（目立つように）
	// =================================================================
	{
		Vec2 commandPos(screenW * 0.25, screenH * 0.15);
		int remaining = m_gameManager->turnManager.GetRemainingCommands();
		Color commandColor = (remaining > 0) ? Palette::Lime : Palette::Red;

		// 背景
		RectF commandPanel(Arg::center(commandPos), 200, 40);
		commandPanel.draw(ColorF(0.1, 0.1, 0.15, 0.9));
		commandPanel.drawFrame(2, commandColor);

		// テキスト
		FontAsset(U"menu")(U"残りコマンド: {}/4"_fmt(remaining))
			.drawAt(commandPos, commandColor);
	}

	// =================================================================
	// 📊 ステータスパネル（宝石のように輝く）
	// =================================================================
	{
		// ★ 画面サイズに収まるように調整
		double panelWidth = Min(screenW * 0.28, 500.0);
		double panelHeight = Min(screenH * 0.45, 450.0);
		RectF statsPanel(screenW - panelWidth - screenW * 0.03,
						 screenH * 0.22,
						 panelWidth,
						 panelHeight);

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
			RectF barBg(static_cast<double>(statX + 50), static_cast<double>(y + 40), screenW * 0.25, 20.0);
			barBg.draw(ColorF(0.1, 0.08, 0.06));
			barBg.drawFrame(2, ColorF(0.5, 0.4, 0.3));

			// バー（実際の値）
			double ratio = Min(1.0, static_cast<double>(value) / maxValue);
			RectF bar(static_cast<double>(statX + 50), static_cast<double>(y + 40), screenW * 0.25 * ratio, 20.0);
			bar.draw(Arg::left = barColor, Arg::right = barColor.lerp(Palette::White, 0.3));

			// ✨ バーの輝き
			{
				ScopedRenderStates2D blend(BlendState::Additive);
				RectF(static_cast<double>(statX + 50), static_cast<double>(y + 40), screenW * 0.25 * ratio, 10.0)
					.draw(ColorF(1, 1, 1, 0.4));

				// 流れる光
				double flowPos = Math::Fmod(time * 200, screenW * 0.25 * ratio);
				Circle(statX + 50 + flowPos, y + 50, 8).draw(ColorF(1, 1, 1, 0.6));
			}
			};

		int baseY = statY + static_cast<int>(screenH * 0.067);
		int gap = static_cast<int>(screenH * 0.089);

		drawParameter(U"$", U"金", m_cityData->gold, 5000, Palette::Gold, baseY + gap * 0);
		drawParameter(U"米", U"兵糧", m_cityData->food, 5000, Palette::Orange, baseY + gap * 1);
		drawParameter(U"兵", U"兵士", m_cityData->troops, 10000, Palette::Red, baseY + gap * 2);
		drawParameter(U"安", U"治安", m_cityData->order, 100, Palette::Skyblue, baseY + gap * 3);

		// 発展度
		FontAsset(U"small")(U"農業: {} / 商業: {} / 兵舎: {}"_fmt(
			m_cityData->agriculture, m_cityData->commerce, m_cityData->barracks))
			.draw(statX + 10, statY + static_cast<int>(screenH * 0.433), ColorF(0.8, 0.8, 0.8));
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
		if (isHovered) RectF(btn).drawFrame(4, ColorF(1, 1, 0.9)); else RectF(btn).drawFrame(4, ColorF(0.8, 0.7, 0.5));

		// 光沢（上部ハイライト）
		RectF(static_cast<double>(btn.x), static_cast<double>(btn.y),
			  static_cast<double>(btn.w), static_cast<double>(btn.h) * 0.35);
			double glowAlpha = isHovered ? 0.3 : 0.2;
		RectF(static_cast<double>(btn.x), static_cast<double>(btn.y),
			  static_cast<double>(btn.w), static_cast<double>(btn.h) * 0.35)
			.draw(ColorF(1, 1, 1, glowAlpha));

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

	// ★ 人材ボタン
	drawFancyButton(m_btnOfficer, U"人材", U"武将管理", ColorF(0.8, 0.6, 0.3), m_btnOfficer.mouseOver());

	// ★ 施設ボタン
	drawFancyButton(m_btnFacility, U"施設", U"建設管理", ColorF(0.6, 0.5, 0.7), m_btnFacility.mouseOver());

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

		// 修正：条件演算子をif文に変更
		if (isHovered)
		{
			m_btnBack.draw(ColorF(0.35, 0.30, 0.25));
		}
		else
		{
			m_btnBack.draw(ColorF(0.25, 0.20, 0.15));
		}

		m_btnBack.drawFrame(3, ColorF(0.7, 0.6, 0.4));

		FontAsset(U"menu")(U"← マップに戻る").drawAt(m_btnBack.center().movedBy(1, 1), ColorF(0, 0, 0, 0.5));
		FontAsset(U"menu")(U"← マップに戻る").drawAt(m_btnBack.center(), Palette::White);

		if (isHovered)
		{
			ScopedRenderStates2D blend(BlendState::Additive);
			m_btnBack.drawFrame(2, ColorF(1, 1, 0.8, 0.4));
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

	// =================================================================
	// 🎖️ 配置武将の顔表示
	// =================================================================
	if (!m_cityData->officers.isEmpty())
	{
		// ★ 武将を左上に配置（都市情報パネルと被らないように）
		double portraitX = screenW * 0.3;   // 左寄り
		double portraitY = screenH * 0.35;  // 中段
		double portraitSize = 90;  // サイズ

		const Officer& mainOfficer = m_cityData->officers[0];

		// 武将の顔
		OfficerPortrait::Draw(mainOfficer, Vec2(portraitX, portraitY), portraitSize);
		OfficerPortrait::DrawNamePlate(mainOfficer, Vec2(portraitX, portraitY), portraitSize);

		// ★ 忠誠度表示（プレイヤー君主以外のみ）
		if (mainOfficer.name != m_gameManager->playerFactionName)
		{
			String loyaltyText = U"忠誠: {}（{}）"_fmt(mainOfficer.loyalty, mainOfficer.GetLoyaltyText());
			FontAsset(U"menu")(loyaltyText)
				.drawAt(portraitX, portraitY + portraitSize * 0.9, mainOfficer.GetLoyaltyColor());
		}

		// 武将が複数いる場合は下に小さく表示
		if (m_cityData->officers.size() > 1)
		{
			for (size_t i = 1; i < Min(size_t(3), m_cityData->officers.size()); ++i)
			{
				double smallX = portraitX - 80 + static_cast<double>(i) * 80;
				double smallY = portraitY + portraitSize + 40;
				OfficerPortrait::DrawSmall(m_cityData->officers[i], Vec2(smallX, smallY), 35);
			}
		}
	}
}
