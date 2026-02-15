#pragma once
#include <Siv3D.hpp>
#include "SceneBase.hpp"
#include "CityData.hpp"
#include "GameManager.hpp"
#include "OfficerPortrait.hpp"
#include "LoyaltyManager.hpp"

// 武将管理画面
class OfficerManagementScene : public SceneBase
{
public:
	OfficerManagementScene(GameManager* gm, CityData* city)
		: m_gameManager(gm)
		, m_cityData(city)
	{
		m_btnBack = Rect(50, Scene::Height() - 100, 200, 60);
	}

	void update() override
	{
		// 武将選択
		int startY = 200;
		int rowHeight = 120;

		for (size_t i = 0; i < m_cityData->officers.size(); ++i)
		{
			Rect row(100, startY + static_cast<int>(i) * rowHeight, Scene::Width() - 200, 110);

			if (row.mouseOver())
			{
				m_selectedOfficer = static_cast<int>(i);
			}

			// 恩賞ボタン（忠誠度を上げる）
			if (m_selectedOfficer == static_cast<int>(i))
			{
				Rect btnReward(row.x + row.w - 200, row.y + 40, 180, 50);

				if (btnReward.leftClicked())
				{
					// ★ ターン制限チェック
					if (!m_gameManager->turnManager.CanExecuteCommand())
					{
						m_message = U"今月のコマンドを使い切りました！";
					}
					else if (m_cityData->gold >= 300)
					{
						m_cityData->gold -= 300;
						LoyaltyManager::RaiseLoyalty(m_cityData->officers[i], 300);
						m_gameManager->turnManager.ExecuteCommand();

						int remaining = m_gameManager->turnManager.GetRemainingCommands();
						m_message = m_cityData->officers[i].name + U" に恩賞！忠誠度+3\n（残り" + Format(remaining) + U"コマンド）";
					}
					else
					{
						m_message = U"金が足りません（300必要）";
					}
				}
			}
		}

		// 戻る
		if (m_btnBack.leftClicked())
		{
			m_sceneEnd = true;
			m_nextScene = U"City";
		}
	}

	void draw() const override
	{
		Scene::SetBackground(ColorF(0.1, 0.08, 0.06));

		// タイトル
		FontAsset(U"huge")(U"人材管理").drawAt(Scene::Center().x, 80, Palette::Gold);

		// 残りコマンド数
		int remaining = m_gameManager->turnManager.GetRemainingCommands();
		FontAsset(U"menu")(U"残りコマンド: {}/4"_fmt(remaining))
			.drawAt(Scene::Center().x, 130, remaining > 0 ? Palette::Lime : Palette::Red);

		// 武将リスト
		int startY = 200;
		int rowHeight = 120;

		for (size_t i = 0; i < m_cityData->officers.size(); ++i)
		{
			const Officer& officer = m_cityData->officers[i];
			Rect row(100, startY + static_cast<int>(i) * rowHeight, Scene::Width() - 200, 110);

			bool isHovered = (m_selectedOfficer == static_cast<int>(i));

			// 行の背景
			row.draw(isHovered ? ColorF(0.25, 0.2, 0.15) : ColorF(0.15, 0.12, 0.1));
			
			if (isHovered)
				row.drawFrame(3, Palette::Gold);
			else
				row.drawFrame(3, ColorF(0.5, 0.4, 0.3));

			// 武将の顔
			OfficerPortrait::Draw(officer, Vec2(row.x + 60, row.y + 55), 80);

			// 武将情報
			int infoX = static_cast<int>(row.x + 130);
			int infoY = static_cast<int>(row.y + 15);

			FontAsset(U"title")(officer.name)
				.draw(infoX, infoY, Palette::White);

			// 能力値
			FontAsset(U"menu")(U"統{}武{}知{}政{}"_fmt(
				officer.leadership, officer.power,
				officer.intelligence, officer.politics
			)).draw(infoX, infoY + 35, Palette::Lightgray);

			// 忠誠度（プレイヤー君主以外のみ表示）
			if (officer.name != m_gameManager->playerFactionName)
			{
				String loyaltyText = U"忠誠: {} ({})"_fmt(officer.loyalty, officer.GetLoyaltyText());
				FontAsset(U"menu")(loyaltyText)
					.draw(infoX, infoY + 65, officer.GetLoyaltyColor());
			}
			else
			{
				// 君主の場合は「君主」と表示
				FontAsset(U"menu")(U"【君主】")
					.draw(infoX, infoY + 65, Palette::Gold);
			}

			// 恩賞ボタン（ホバー時のみ、かつプレイヤー君主以外）
			if (isHovered && officer.name != m_gameManager->playerFactionName)
			{
				Rect btnReward(row.x + row.w - 200, row.y + 40, 180, 50);

				bool canReward = m_gameManager->turnManager.CanExecuteCommand() && m_cityData->gold >= 300;
				if (canReward)
					btnReward.draw(ColorF(0.6, 0.5, 0.2));
				else
					btnReward.draw(Palette::Darkgray);

				btnReward.drawFrame(2, Palette::Gold);

				FontAsset(U"menu")(U"恩賞(金300)").drawAt(btnReward.center(), Palette::White);
				FontAsset(U"menu")(U"忠誠+3").drawAt(btnReward.center().movedBy(0, 20), Palette::Lime);
			}
		}

		// 戻るボタン
		m_btnBack.draw(Palette::Darkgray).drawFrame(2, Palette::White);
		FontAsset(U"menu")(U"戻る").drawAt(m_btnBack.center());

		// メッセージ
		if (!m_message.isEmpty())
		{
			RectF msgBox(Scene::Center().x - 300, Scene::Height() - 180, 600, 80);
			msgBox.draw(ColorF(0, 0, 0, 0.9));
			msgBox.drawFrame(3, Palette::Gold);
			FontAsset(U"menu")(m_message).drawAt(msgBox.center(), Palette::White);
		}
	}

private:
	GameManager* m_gameManager;
	CityData* m_cityData;
	int m_selectedOfficer = -1;
	Rect m_btnBack;
	String m_message;
};
