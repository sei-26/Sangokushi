#include <Siv3D.hpp>
#include "GameSceneManager.hpp"
#include "CSVDataLoader.hpp"

void Main()
{
	// ★ フルスクリーン設定
	Window::SetTitle(U"三国志風SLG - 官渡の戦い");
	Window::SetStyle(WindowStyle::Frameless);  // 枠なし
	Scene::SetResizeMode(ResizeMode::Keep);

	// モニターのサイズを取得してフルスクリーンに
	const Size monitorSize = System::GetCurrentMonitor().displayRect.size;
	Scene::Resize(monitorSize);
	Window::Resize(monitorSize);

	// フォント登録
	FontAsset::Register(U"title", 36, Typeface::Bold);
	FontAsset::Register(U"menu", 24);
	FontAsset::Register(U"small", 18);
	FontAsset::Register(U"huge", 56, Typeface::Bold);

	// =================================================================
	// 📊 CSVからデータを読み込む
	// =================================================================
	Array<CityData> cities = CSVDataLoader::LoadCities(U"cities.csv");
	CSVDataLoader::LoadOfficers(cities, U"officers.csv");

	// データが読み込めなかった場合の処理
	if (cities.isEmpty())
	{
		Print << U"[ERROR] 都市データが読み込めませんでした";
		Print << U"cities.csv と officers.csv をプロジェクトフォルダに配置してください";

		while (System::Update())
		{
			FontAsset(U"title")(U"データファイルが見つかりません").draw(50, 50, Palette::Red);
			FontAsset(U"menu")(U"cities.csv と officers.csv を").draw(50, 150, Palette::White);
			FontAsset(U"menu")(U"プロジェクトフォルダに配置してください").draw(50, 200, Palette::White);
		}
		return;
	}

	// =================================================================
	// 🎮 プレイヤー勢力の選択
	// =================================================================

	// 勢力一覧を作成
	HashSet<String> factionSet;
	for (const auto& city : cities)
	{
		factionSet.insert(city.owner);
	}

	// HashSetをArrayに変換
	Array<String> factionNames;
	for (const auto& faction : factionSet)
	{
		factionNames.push_back(faction);
	}

	// 選択画面
	String selectedFaction;
	{
		int selectedIndex = 0; // デフォルトは最初の勢力

		while (System::Update())
		{
			Scene::SetBackground(ColorF(0.1, 0.1, 0.1));

			// タイトル
			FontAsset(U"huge")(U"勢力を選択してください").drawAt(Scene::Center().x, 100, Palette::White);

			// 勢力一覧
			for (int i = 0; i < factionNames.size(); ++i)
			{
				String faction = factionNames[i];

				// その勢力の都市数と総兵力を計算
				int cityCount = 0;
				int totalTroops = 0;
				Color factionColor;

				for (const auto& city : cities)
				{
					if (city.owner == faction)
					{
						cityCount++;
						totalTroops += city.troops;
						factionColor = city.color;
					}
				}

				// ボタン
				RectF button(Scene::Center().x - 300, 250 + i * 100, 600, 80);
				bool isHovered = button.mouseOver();
				bool isSelected = (i == selectedIndex);

				// ボタンの描画
				button.movedBy(3, 3).draw(ColorF(0, 0, 0, 0.5));

				if (isSelected)
				{
					button.draw(Arg::top = ColorF(factionColor).lerp(Palette::White, 0.3),
								Arg::bottom = ColorF(factionColor));
				}
				else if (isHovered)
				{
					button.draw(Arg::top = ColorF(factionColor).lerp(Palette::White, 0.2),
								Arg::bottom = ColorF(factionColor).lerp(Palette::Black, 0.2));
				}
				else
				{
					button.draw(Arg::top = ColorF(factionColor).lerp(Palette::Black, 0.1),
								Arg::bottom = ColorF(factionColor).lerp(Palette::Black, 0.4));
				}

				// 修正点: 三項演算子に ':' を追加し、両辺を ColorF に統一して曖昧性を解消
				button.drawFrame(3, isSelected ? ColorF(Palette::White) : ColorF(0.5, 0.5, 0.5));

				// テキスト
				String text = U"{} （都市{}、兵力{}）"_fmt(faction, cityCount, totalTroops);
				FontAsset(U"title")(text).drawAt(button.center(), Palette::White);

				// クリック処理
				if (button.leftClicked())
				{
					selectedIndex = i;
				}

				// ★ ダブルクリックで即開始
				if (button.leftPressed() && selectedIndex == i)
				{
					selectedFaction = factionNames[selectedIndex];
					break;
				}
			}

			// ループを抜けた場合はゲーム開始
			if (!selectedFaction.isEmpty())
			{
				break;
			}

			// 決定ボタン
			RectF startButton(Scene::Center().x - 150, Scene::Height() - 150, 300, 80);
			bool startHovered = startButton.mouseOver();

			startButton.movedBy(3, 3).draw(ColorF(0, 0, 0, 0.5));
			startButton.draw(startHovered ? ColorF(0.3, 0.8, 0.3) : ColorF(0.2, 0.6, 0.2));
			startButton.drawFrame(3, Palette::White);

			FontAsset(U"title")(U"開始").drawAt(startButton.center(), Palette::White);

			// ★ 修正：ボタンクリックまたはEnterキーで決定
			if (startButton.leftClicked() || KeyEnter.down())
			{
				selectedFaction = factionNames[selectedIndex];
				break;
			}
		}
	}

	// =================================================================
	// 🎮 ゲーム開始
	// =================================================================

	// プレイヤー勢力のデータ作成
	Faction playerFaction;
	playerFaction.name = selectedFaction;

	// 勢力色を取得
	for (const auto& city : cities)
	{
		if (city.owner == selectedFaction)
		{
			playerFaction.color = city.color;
			break;
		}
	}

	// ゲームマネージャー
	GameManager gameManager;
	gameManager.year = 200;  // 西暦200年（官渡の戦い）
	gameManager.month = 1;

	// シーンマネージャー起動
	GameSceneManager sceneManager(&gameManager, playerFaction, cities);

	// メインループ
	while (System::Update())
	{
		// ESCキーで終了
		if (KeyEscape.down())
		{
			break;
		}

		sceneManager.update();
		sceneManager.draw();
	}
}
