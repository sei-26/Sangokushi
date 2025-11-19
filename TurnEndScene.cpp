#include "TurnEndScene.hpp"

// =========================================
//  コンストラクタ
// =========================================
TurnEndScene::TurnEndScene(const Advisor& adv, int turn)
	: m_advisor(adv)
	, m_turn(turn)
	, m_advisorTexture(U"advisor.png")   // ★ 立ち絵画像（あとで差し替え）
{
}

// =========================================
//  Update
// =========================================
void TurnEndScene::update()
{
	// フェードイン 0 → 1
	m_alpha += Scene::DeltaTime() * 1.2;
	m_faceAlpha += Scene::DeltaTime() * 1.0;

	if (m_alpha >= 1.0) m_alpha = 1.0;
	if (m_faceAlpha >= 1.0) m_faceAlpha = 1.0;

	// OKボタンを押したら戻る
	const RectF okBtn{ 550, 500, 180, 50 };
	if (m_faceAlpha >= 1.0 && okBtn.leftClicked())
	{
		m_isEnd = true;
		m_nextScene = U"WorldMapScene";
	}
}

// =========================================
//  Draw
// =========================================
void TurnEndScene::draw() const
{
	// ★ 黒フェード背景
	RectF(Scene::Size()).draw(ColorF(0, 0, 0, m_alpha));

	// ★ 軍師立ち絵（左）
	if (m_faceAlpha > 0.0)
	{
		m_advisorTexture
			.draw(200, 120, ColorF(1, 1, 1, m_faceAlpha));
	}

	// ★ メッセージウィンドウ
	if (m_faceAlpha > 0.7)
	{
		RectF(380, 150, 700, 350).draw(ColorF(0.1, 0.1, 0.1, 0.85));

		m_font(U"ターン " + Format(m_turn) + U" 経過")
			.draw(420, 180, Palette::White);

		m_small(
			m_advisor.name + U"「諸事滞りなく進みましたぞ」"
		).draw(420, 260, Palette::White);

		// ★ OK ボタン
		RectF okBtn{ 550, 500, 180, 50 };
		okBtn.draw(ColorF(0.3, 0.3, 0.3));
		m_small(U"OK").drawAt(okBtn.center(), Palette::White);
	}
}
