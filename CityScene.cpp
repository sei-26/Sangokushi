#include "CityScene.hpp"
#include <Siv3D.hpp>
#include "AttackSelectScene.hpp"
CityScene::CityScene(const CityData& city, const Advisor& adv)
	: m_city(city)
	, m_advisor(adv)
{
	m_message = U"何を行いますか？";
}

void CityScene::update()
{
	// ESC で戻る
	if (KeyEscape.down())
	{
		m_isEnd = true;
		m_nextScene = U"WorldMapScene";
		return;
	}

	// ================================
	//  内政コマンド判定（優先）
	// ================================

	if (m_btnAgr.leftClicked())
	{
		m_city.agriculture += 10;
		m_message = U"諸葛亮「農地を整備しましたぞ」";

		m_isEnd = true;
		m_nextScene = U"TurnEnd";
		return;
	}
	else if (m_btnCom.leftClicked())
	{
		m_city.commerce += 10;
		m_message = U"諸葛亮「市がにぎわいを見せています」";

		m_isEnd = true;
		m_nextScene = U"TurnEnd";
		return;
	}
	else if (m_btnTrain.leftClicked())
	{
		m_city.troops += 20;
		m_message = U"諸葛亮「兵を鍛錬しました」";

		m_isEnd = true;
		m_nextScene = U"TurnEnd";
		return;
	}
	else if (m_btnOrder.leftClicked())
	{
		m_city.order = Min(m_city.order + 5, 100);
		m_message = U"諸葛亮「治安が改善しました」";

		m_isEnd = true;
		m_nextScene = U"TurnEnd";
		return;
	}
	else if (m_btnAttack.leftClicked())
	{
		m_message = U"諸葛亮「出陣の準備をいたしましょう」";

		m_isEnd = true;
		m_nextScene = U"AttackSelectScene";
		return;
	}

	// ================================
	//  戻るボタン（最後に判定）
	// ================================
	if (m_btnBack.leftClicked())
	{
		m_isEnd = true;
		m_nextScene = U"WorldMapScene";
		return;
	}
}



void CityScene::draw() const
{
	Scene::SetBackground(ColorF(0.1, 0.1, 0.1));

	// ================================
	//  左側：都市ステータス
	// ================================
	m_font(U"都市: " + m_city.name).draw(30, 30);
	m_font(U"統治者: " + m_city.owner).draw(30, 70);

	m_font(U"金: " + Format(m_city.gold)).draw(30, 120);
	m_font(U"兵糧: " + Format(m_city.food)).draw(30, 160);
	m_font(U"兵士: " + Format(m_city.troops)).draw(30, 200);
	m_font(U"治安: " + Format(m_city.order)).draw(30, 240);

	m_font(U"農業: " + Format(m_city.agriculture)).draw(30, 300);
	m_font(U"商業: " + Format(m_city.commerce)).draw(30, 340);
	m_font(U"兵舎: Lv." + Format(m_city.barracks)).draw(30, 380);

	// ================================
	//  コマンドボタン
	// ================================
	m_btnAgr.draw(Palette::Seagreen);
	m_btnCom.draw(Palette::Darkcyan);
	m_btnTrain.draw(Palette::Steelblue);
	m_btnOrder.draw(Palette::Darkorange);
	m_btnAttack.draw(Palette::Firebrick);
	


	m_font(U"農業強化").drawAt(m_btnAgr.center(), Palette::White);
	m_font(U"商業強化").drawAt(m_btnCom.center(), Palette::White);
	m_font(U"訓練").drawAt(m_btnTrain.center(), Palette::White);
	m_font(U"治安維持").drawAt(m_btnOrder.center(), Palette::White);
	m_font(U"進攻").drawAt(m_btnAttack.center(), Palette::White);
	// ================================
	//  戻るボタン
	// ================================
	m_btnBack.draw(ColorF(0.2, 0.2, 0.2));
	m_font(U"戻る").drawAt(m_btnBack.center(), Palette::White);

	// ================================
	//  軍師コメント
	// ================================
	RectF(0, 450, 1280, 80).draw(ColorF(0, 0, 0, 0.5));
	m_font(m_message).draw(30, 470, Palette::White);
}
