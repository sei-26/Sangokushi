#pragma once
#include <Siv3D.hpp>
#include "SceneBase.hpp"
#include "CityData.hpp"
#include "CityFacility.hpp"
#include "GameManager.hpp"
#include "TurnManager.hpp"

// 施設建設画面
class FacilityScene : public SceneBase
{
public:
	FacilityScene(GameManager* gm, CityData* city, TurnManager* turnMgr)
		: m_gameManager(gm)
		, m_cityData(city)
		, m_turnManager(turnMgr)
		, m_selectedSlot(-1)
		, m_selectedType(CityFacility::Type::None)
		, m_selectedLevel(1)
	{
		// ★ 施設配列を初期化（まだ初期化されていない場合）
		if (m_cityData->facilities.isEmpty())
		{
			m_cityData->facilities.resize(5);
			for (auto& facility : m_cityData->facilities)
			{
				facility.type = CityFacility::Type::None;
				facility.level = 0;
				facility.isBuilding = false;
				facility.buildTurnsLeft = 0;
			}
		}
	}

	void update() override
	{
		int screenW = Scene::Width();
		int screenH = Scene::Height();

		// 戻るボタン
		Rect btnBack(50, screenH - 100, 150, 60);
		if (btnBack.leftClicked())
		{
			m_sceneEnd = true;
			m_nextScene = U"City";
		}

		// 既存施設のクリック判定
		for (int i = 0; i < m_cityData->facilities.size(); ++i)
		{
			Rect slotRect(100, 250 + i * 100, screenW - 200, 90);
			if (slotRect.leftClicked())
			{
				m_selectedSlot = i;
			}
		}

		// 選択中のスロットがある場合
		if (m_selectedSlot >= 0 && m_selectedSlot < m_cityData->facilities.size())
		{
			auto& facility = m_cityData->facilities[m_selectedSlot];

			// 新規建設
			if (facility.type == CityFacility::Type::None && !facility.isBuilding)
			{
				// 施設タイプ選択ボタン
				Array<CityFacility::Type> types = {
					CityFacility::Type::Farm,
					CityFacility::Type::Market,
					CityFacility::Type::Barracks,
					CityFacility::Type::School,
					CityFacility::Type::Wall
				};

				for (int i = 0; i < types.size(); ++i)
				{
					Rect btnType(screenW - 300, 250 + i * 70, 250, 60);
					if (btnType.leftClicked())
					{
						m_selectedType = types[i];
					}
				}

				// レベル選択（横並び）
				for (int lv = 1; lv <= 5; ++lv)
				{
					Rect btnLevel(screenW - 470 + (lv - 1) * 88, 600, 80, 50);  // ★ 間隔88に
					if (btnLevel.leftClicked())
					{
						m_selectedLevel = lv;
					}
				}

				// 建設実行ボタン
				Rect btnBuild(screenW - 470, screenH - 200, 400, 70);
				if (btnBuild.leftClicked() && m_selectedType != CityFacility::Type::None)
				{
					CityFacility::Facility tempFacility;
					tempFacility.type = m_selectedType;
					tempFacility.level = m_selectedLevel;

					int cost = tempFacility.GetBuildCost();

					if (m_cityData->gold >= cost && m_turnManager->CanExecuteCommand())
					{
						m_cityData->gold -= cost;
						m_cityData->facilities[m_selectedSlot].StartBuild(m_selectedType, m_selectedLevel);
						m_turnManager->UseCommand();
						m_selectedSlot = -1;
						m_selectedType = CityFacility::Type::None;
					}
				}
			}
			// レベルアップ
			else if (facility.CanUpgrade())
			{
				Rect btnUpgrade(screenW - 300, screenH - 200, 250, 70);
				if (btnUpgrade.leftClicked())
				{
					CityFacility::Facility tempFacility;
					tempFacility.type = facility.type;
					tempFacility.level = facility.level + 1;

					int cost = tempFacility.GetBuildCost();

					if (m_cityData->gold >= cost && m_turnManager->CanExecuteCommand())
					{
						m_cityData->gold -= cost;
						m_cityData->facilities[m_selectedSlot].StartBuild(facility.type, facility.level + 1);
						m_turnManager->UseCommand();
						m_selectedSlot = -1;
					}
				}
			}
		}
	}

	void draw() const override
	{
		int screenW = Scene::Width();
		int screenH = Scene::Height();

		// 背景
		Scene::SetBackground(ColorF(0.15, 0.2, 0.25));

		// タイトル
		FontAsset(U"huge")(U"🏗️ 施設建設 🏗️").drawAt(screenW / 2, 80, Palette::Gold);

		// 都市名と資源
		FontAsset(U"title")(m_cityData->name).drawAt(screenW / 2, 150, Palette::White);
		FontAsset(U"menu")(U"金: {} / 残りコマンド: {}/4"_fmt(
			m_cityData->gold,
			m_turnManager->GetRemainingCommands()
		)).drawAt(screenW / 2, 190, Palette::Lightgray);

		// 施設一覧
		for (int i = 0; i < m_cityData->facilities.size(); ++i)
		{
			const auto& facility = m_cityData->facilities[i];
			Rect slotRect(100, 250 + i * 100, screenW - 550, 90);  // ★ 550に変更

			bool isSelected = (i == m_selectedSlot);
			slotRect.draw(isSelected ? ColorF(0.3, 0.4, 0.5) : ColorF(0.2, 0.25, 0.3));
			slotRect.drawFrame(3, isSelected ? Palette::Gold : Palette::Gray);

			if (facility.type != CityFacility::Type::None)
			{
				// 施設情報
				String facilityText = facility.GetIcon() + U" " + facility.GetName() + U" Lv" + Format(facility.level);
				FontAsset(U"title")(facilityText).draw(slotRect.x + 20, slotRect.y + 15, Palette::White);

				if (facility.isBuilding)
				{
					FontAsset(U"menu")(U"建設中... 残り{}ターン"_fmt(facility.buildTurnsLeft))
						.draw(slotRect.x + 20, slotRect.y + 50, Palette::Orange);
				}
				else
				{
					FontAsset(U"menu")(facility.GetDescription())
						.draw(slotRect.x + 20, slotRect.y + 50, Palette::Lightgray);
				}
			}
			else
			{
				FontAsset(U"menu")(U"空き").drawAt(slotRect.center(), Palette::Gray);
			}
		}

		// 右側パネル（選択中の場合）
		if (m_selectedSlot >= 0 && m_selectedSlot < m_cityData->facilities.size())
		{
			auto& facility = m_cityData->facilities[m_selectedSlot];

			RectF panel(screenW - 500, 250, 470, screenH - 350);  // ★ 位置とサイズ調整
			panel.draw(ColorF(0.1, 0.15, 0.2));
			panel.drawFrame(3, Palette::Gold);

			if (facility.type == CityFacility::Type::None)
			{
				// 新規建設
				FontAsset(U"title")(U"施設を選択").drawAt(panel.center().x, panel.y + 30, Palette::White);

				Array<CityFacility::Type> types = {
					CityFacility::Type::Farm,
					CityFacility::Type::Market,
					CityFacility::Type::Barracks,
					CityFacility::Type::School,
					CityFacility::Type::Wall
				};

				for (int i = 0; i < types.size(); ++i)
				{
					Rect btnType(screenW - 470, 280 + i * 70, 440, 60);  // ★ 位置とサイズ調整
					bool isTypeSelected = (types[i] == m_selectedType);
					btnType.draw(isTypeSelected ? ColorF(0.4, 0.5, 0.3) : ColorF(0.3, 0.35, 0.4));
					btnType.drawFrame(2, isTypeSelected ? Palette::Lime : Palette::Gray);

					String text = CityFacility::GetTypeIcon(types[i]) + U" " + CityFacility::GetTypeName(types[i]);
					FontAsset(U"menu")(text).drawAt(btnType.center(), Palette::White);
				}

				// レベル選択
				FontAsset(U"menu")(U"レベル:").draw(panel.x + 20, 580, Palette::White);
				for (int lv = 1; lv <= 5; ++lv)
				{
					Rect btnLevel(screenW - 470 + (lv - 1) * 88, 610, 80, 50);  // ★ 間隔88に
					bool isLevelSelected = (lv == m_selectedLevel);
					btnLevel.draw(isLevelSelected ? ColorF(0.4, 0.5, 0.3) : ColorF(0.3, 0.35, 0.4));
					btnLevel.drawFrame(2, isLevelSelected ? Palette::Lime : Palette::Gray);
					FontAsset(U"menu")(Format(lv)).drawAt(btnLevel.center(), Palette::White);
				}

				// コスト表示
				if (m_selectedType != CityFacility::Type::None)
				{
					CityFacility::Facility tempFacility;
					tempFacility.type = m_selectedType;
					tempFacility.level = m_selectedLevel;

					int cost = tempFacility.GetBuildCost();
					int turns = tempFacility.GetBuildTurns();

					FontAsset(U"menu")(U"コスト: 金{}"_fmt(cost)).draw(panel.x + 20, 680, Palette::Yellow);
					FontAsset(U"menu")(U"期間: {}ターン"_fmt(turns)).draw(panel.x + 20, 710, Palette::Lightgray);

					// 建設ボタン
					Rect btnBuild(screenW - 470, 750, 440, 70);
					bool canBuild = (m_cityData->gold >= cost && m_turnManager->CanExecuteCommand());
					btnBuild.draw(canBuild ? ColorF(0.3, 0.6, 0.3) : ColorF(0.3, 0.3, 0.3));
					btnBuild.drawFrame(3, canBuild ? Palette::Lime : Palette::Gray);
					FontAsset(U"title")(U"建設開始").drawAt(btnBuild.center(), Palette::White);
				}
			}
			else if (facility.CanUpgrade())
			{
				// レベルアップ
				FontAsset(U"title")(U"レベルアップ").drawAt(panel.center().x, panel.y + 30, Palette::White);

				CityFacility::Facility tempFacility;
				tempFacility.type = facility.type;
				tempFacility.level = facility.level + 1;

				int cost = tempFacility.GetBuildCost();
				int turns = tempFacility.GetBuildTurns();

				FontAsset(U"menu")(U"現在: Lv{}"_fmt(facility.level)).draw(panel.x + 20, 300, Palette::White);
				FontAsset(U"menu")(U"→ Lv{}"_fmt(facility.level + 1)).draw(panel.x + 20, 330, Palette::Lime);
				FontAsset(U"menu")(U"効果: {} → {}"_fmt(facility.GetEffect(), tempFacility.GetEffect()))
					.draw(panel.x + 20, 370, Palette::Lightgray);

				FontAsset(U"menu")(U"コスト: 金{}"_fmt(cost)).draw(panel.x + 20, 420, Palette::Yellow);
				FontAsset(U"menu")(U"期間: {}ターン"_fmt(turns)).draw(panel.x + 20, 450, Palette::Lightgray);

				// アップグレードボタン
				Rect btnUpgrade(screenW - 470, 500, 400, 70);
				bool canUpgrade = (m_cityData->gold >= cost && m_turnManager->CanExecuteCommand());
				btnUpgrade.draw(canUpgrade ? ColorF(0.3, 0.6, 0.3) : ColorF(0.3, 0.3, 0.3));
				btnUpgrade.drawFrame(3, canUpgrade ? Palette::Lime : Palette::Gray);
				FontAsset(U"title")(U"アップグレード").drawAt(btnUpgrade.center(), Palette::White);
			}
		}

		// 戻るボタン
		Rect btnBack(50, screenH - 100, 150, 60);
		btnBack.draw(ColorF(0.3, 0.3, 0.4));
		btnBack.drawFrame(2, Palette::Gray);
		FontAsset(U"menu")(U"← 戻る").drawAt(btnBack.center(), Palette::White);
	}

private:
	GameManager* m_gameManager;
	CityData* m_cityData;
	TurnManager* m_turnManager;

	int m_selectedSlot;
	CityFacility::Type m_selectedType;
	int m_selectedLevel;
};
