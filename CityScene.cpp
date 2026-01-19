#include "CityScene.hpp"

CityScene::CityScene(GameManager* gm, CityData& city)
	: m_gameManager(gm)
	, m_cityData(&city)
{
	m_message = U"ようこそ " + m_cityData->name + U" へ。\n指示をください。";

	// ボタン配置
	int startX = 50;
	int startY = 200;
	int btnW = 200;
	int btnH = 50;
	int gap = 60;

	m_btnAgr = Rect(startX, startY + gap * 0, btnW, btnH);
	m_btnCom = Rect(startX, startY + gap * 1, btnW, btnH);
	m_btnTrain = Rect(startX, startY + gap * 2, btnW, btnH);
	m_btnOrder = Rect(startX, startY + gap * 3, btnW, btnH);

	// ★ 出陣ボタンは少し離して配置（目立つように）
	m_btnAttack = Rect(startX, startY + gap * 5, btnW, btnH + 10);

	m_btnBack = Rect(Scene::Width() - 250, Scene::Height() - 100, 200, 60);
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
			m_message = U"農業開発を行いました。\n農業値が 10 上昇！";
		}
		else m_message = U"金が足りません！";
	}

	// 商業
	if (m_btnCom.leftClicked())
	{
		if (m_cityData->gold >= 100)
		{
			m_cityData->gold -= 100;
			m_cityData->commerce += 10;
			m_message = U"商業投資を行いました。\n商業値が 10 上昇！";
		}
		else m_message = U"金が足りません！";
	}

	// 徴兵
	if (m_btnTrain.leftClicked())
	{
		if (m_cityData->food >= 100)
		{
			m_cityData->food -= 100;
			m_cityData->troops += 200;
			m_message = U"徴兵を行いました。\n兵士が 200 増えました！";
		}
		else m_message = U"兵糧が足りません！";
	}

	// 治安
	if (m_btnOrder.leftClicked())
	{
		if (m_cityData->gold >= 50)
		{
			m_cityData->gold -= 50;
			m_cityData->order += 5;
			if (m_cityData->order > 100) m_cityData->order = 100;
			m_message = U"巡回を行いました。\n治安が改善しました。";
		}
		else m_message = U"金が足りません！";
	}

	// ★ 出陣（ここが重要！）
	if (m_btnAttack.leftClicked())
	{
		if (m_cityData->troops >= 1000)
		{
			// 本来は「攻め込む都市を選ぶ」画面に行きますが、
			// まずは戦闘画面への遷移テストとして直接飛ばします
			m_sceneEnd = true;
			m_nextScene = U"Battle"; // 次はバトルシーンへ！
		}
		else
		{
			m_message = U"兵士が足りません！\n最低でも1000人は必要です。";
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
	Scene::SetBackground(ColorF{ 0.3, 0.2, 0.1 });

	// タイトル
	FontAsset(U"title")(m_cityData->name).drawAt(Scene::Center().x, 50);

	// ステータス
	int statX = 400;
	FontAsset(U"menu")(U"支配: " + m_cityData->owner).draw(statX, 150);
	FontAsset(U"menu")(U"金: {}"_fmt(m_cityData->gold)).draw(statX, 200, Palette::Gold);
	FontAsset(U"menu")(U"兵糧: {}"_fmt(m_cityData->food)).draw(statX, 240, Palette::Orange);
	FontAsset(U"menu")(U"兵士: {}"_fmt(m_cityData->troops)).draw(statX, 280);

	FontAsset(U"menu")(U"農業: {}"_fmt(m_cityData->agriculture)).draw(statX + 300, 200);
	FontAsset(U"menu")(U"商業: {}"_fmt(m_cityData->commerce)).draw(statX + 300, 240);
	FontAsset(U"menu")(U"治安: {}"_fmt(m_cityData->order)).draw(statX + 300, 280);

	// 内政ボタン群
	m_btnAgr.draw(Palette::Darkgreen).drawFrame(2, Palette::White);
	FontAsset(U"menu")(U"農業開発 (金100)").drawAt(m_btnAgr.center());

	m_btnCom.draw(Palette::Blue).drawFrame(2, Palette::White);
	FontAsset(U"menu")(U"商業投資 (金100)").drawAt(m_btnCom.center());

	m_btnTrain.draw(Palette::Red).drawFrame(2, Palette::White);
	FontAsset(U"menu")(U"徴兵 (糧100)").drawAt(m_btnTrain.center());

	m_btnOrder.draw(Palette::Gray).drawFrame(2, Palette::White);
	FontAsset(U"menu")(U"治安維持 (金50)").drawAt(m_btnOrder.center());

	// ★ 出陣ボタン（赤くて大きい！）
	m_btnAttack.draw(Palette::Firebrick).drawFrame(2, Palette::Gold);
	FontAsset(U"title")(U"出 陣").drawAt(m_btnAttack.center(), Palette::White);

	// 戻るボタン
	m_btnBack.draw(Palette::Black).drawFrame(2, Palette::White);
	FontAsset(U"menu")(U"マップに戻る").drawAt(m_btnBack.center());

	// メッセージ
	Rect(50, 600, 800, 150).draw(ColorF{ 0, 0, 0, 0.7 }).drawFrame(2, Palette::White);
	FontAsset(U"menu")(m_message).draw(70, 620);
}
