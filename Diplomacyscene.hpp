#pragma once
#include <Siv3D.hpp>
#include "SceneBase.hpp"
#include "DiplomacyManager.hpp"
#include "CityData.hpp"
#include "GameManager.hpp"

// 外交画面
class DiplomacyScene : public SceneBase
{
public:
	DiplomacyScene(DiplomacyManager* dipMgr, const String& playerFaction, Array<CityData>* allCities, GameManager* gm)
		: m_diplomacy(dipMgr)
		, m_playerFaction(playerFaction)
		, m_allCities(allCities)
		, m_gameManager(gm)
	{
		// 勢力リストを作成
		HashSet<String> factionSet;
		for (const auto& city : *m_allCities)
		{
			factionSet.insert(city.owner);
		}

		for (const auto& faction : factionSet)
		{
			if (faction != playerFaction)
			{
				m_otherFactions.push_back(faction);
			}
		}

		// ボタン配置
		m_btnBack = Rect(50, Scene::Height() - 100, 200, 60);
	}

	void update() override
	{
		// 勢力選択
		int mouseY = Cursor::Pos().y;
		int startY = 150;
		int rowHeight = 100;

		m_selectedFaction = -1;
		for (size_t i = 0; i < m_otherFactions.size(); ++i)
		{
			Rect row(100, startY + static_cast<int>(i) * rowHeight, Scene::Width() - 200, 90);
			if (row.mouseOver())
			{
				m_selectedFaction = static_cast<int>(i);
			}

			// 外交コマンドボタン
			if (m_selectedFaction == i)
			{
				Rect btnAlliance(row.x + row.w - 600, row.y + 10, 140, 35);
				Rect btnGift(row.x + row.w - 450, row.y + 10, 140, 35);
				Rect btnTruce(row.x + row.w - 300, row.y + 10, 140, 35);
				Rect btnWar(row.x + row.w - 150, row.y + 10, 140, 35);

				String targetFaction = m_otherFactions[i];
				auto& relation = m_diplomacy->GetRelation(m_playerFaction, targetFaction);

				// 同盟を結ぶ
				if (btnAlliance.leftClicked())
				{
					// ★ ターン制限チェック
					if (!m_gameManager->turnManager.CanExecuteCommand())
					{
						m_message = U"今月のコマンドを使い切りました！";
					}
					else if (m_diplomacy->FormAlliance(m_playerFaction, targetFaction))
					{
						m_gameManager->turnManager.ExecuteCommand();
						int remaining = m_gameManager->turnManager.GetRemainingCommands();
						m_message = targetFaction + U" と同盟を結びました！\n（残り" + Format(remaining) + U"コマンド）";
					}
					else
					{
						m_message = U"関係値が足りません（+60以上必要）";
					}
				}

				// 贈り物
				if (btnGift.leftClicked())
				{
					// ★ ターン制限チェック
					if (!m_gameManager->turnManager.CanExecuteCommand())
					{
						m_message = U"今月のコマンドを使い切りました！";
					}
					else
					{
						// プレイヤーの金を確認
						int playerGold = 0;
						for (const auto& city : *m_allCities)
						{
							if (city.owner == m_playerFaction)
							{
								playerGold += city.gold;
							}
						}

						int giftAmount = 500;
						if (playerGold >= giftAmount)
						{
							// 金を支払う
							for (auto& city : *m_allCities)
							{
								if (city.owner == m_playerFaction && city.gold >= giftAmount)
								{
									city.gold -= giftAmount;
									break;
								}
							}

							m_diplomacy->GiveGift(m_playerFaction, targetFaction, giftAmount);
							m_gameManager->turnManager.ExecuteCommand();
							int remaining = m_gameManager->turnManager.GetRemainingCommands();
							m_message = targetFaction + U" に贈り物をしました（関係+5）\n（残り" + Format(remaining) + U"コマンド）";
						}
						else
						{
							m_message = U"金が足りません（500必要）";
						}
					}
				}

				// 停戦
				if (btnTruce.leftClicked())
				{
					// ★ ターン制限チェック
					if (!m_gameManager->turnManager.CanExecuteCommand())
					{
						m_message = U"今月のコマンドを使い切りました！";
					}
					else
					{
						m_diplomacy->SignTruce(m_playerFaction, targetFaction, 12);
						m_gameManager->turnManager.ExecuteCommand();
						int remaining = m_gameManager->turnManager.GetRemainingCommands();
						m_message = targetFaction + U" と停戦協定を結びました（12ヶ月）\n（残り" + Format(remaining) + U"コマンド）";
					}
				}

				// 宣戦布告（戦争なのでターン消費なし）
				if (btnWar.leftClicked() && relation.CanWar())
				{
					m_diplomacy->DeclareWar(m_playerFaction, targetFaction);
					m_message = targetFaction + U" に宣戦布告しました！\n（宣戦布告はコマンド消費なし）";
				}
			}
		}

		// 戻る
		if (m_btnBack.leftClicked())
		{
			m_sceneEnd = true;
			m_nextScene = U"WorldMap";
		}
	}

	void draw() const override
	{
		Scene::SetBackground(ColorF(0.1, 0.12, 0.15));

		// タイトル
		FontAsset(U"huge")(U"外交").drawAt(Scene::Center().x, 60, Palette::Gold);

		// ★ 残りコマンド数表示
		int remaining = m_gameManager->turnManager.GetRemainingCommands();
		Color commandColor = (remaining > 0) ? Palette::Lime : Palette::Red;
		FontAsset(U"menu")(U"残りコマンド: {}/4"_fmt(remaining))
			.drawAt(Scene::Center().x, 100, commandColor);

		// 勢力リスト
		int startY = 150;
		int rowHeight = 100;

		for (size_t i = 0; i < m_otherFactions.size(); ++i)
		{
			String targetFaction = m_otherFactions[i];
			Rect row(100, startY + static_cast<int>(i) * rowHeight, Scene::Width() - 200, 90);

			bool isHovered = (m_selectedFaction == static_cast<int>(i));

			// 行の背景
			row.draw(isHovered ? ColorF(0.25, 0.25, 0.3) : ColorF(0.15, 0.15, 0.2));
			row.drawFrame(2, isHovered ? Palette::Gold : Palette::Gray);

			// 勢力名
			FontAsset(U"title")(targetFaction)
				.draw(row.x + 20, row.y + 10, Palette::White);

			// 関係情報
			auto& relation = m_diplomacy->GetRelation(m_playerFaction, targetFaction);

			String statusText = relation.GetStatusText();
			Color statusColor = relation.GetColor();

			FontAsset(U"menu")(U"関係: " + statusText + U" ({:+d})"_fmt(relation.relationValue))
				.draw(row.x + 20, row.y + 50, statusColor);

			if (relation.truceMonthsLeft > 0)
			{
				FontAsset(U"menu")(U"停戦中（残り{}ヶ月）"_fmt(relation.truceMonthsLeft))
					.draw(row.x + 300, row.y + 50, Palette::Skyblue);
			}

			// 外交コマンドボタン（ホバー時のみ）
			if (isHovered)
			{
				Rect btnAlliance(row.x + row.w - 600, row.y + 10, 140, 35);
				Rect btnGift(row.x + row.w - 450, row.y + 10, 140, 35);
				Rect btnTruce(row.x + row.w - 300, row.y + 10, 140, 35);
				Rect btnWar(row.x + row.w - 150, row.y + 10, 140, 35);

				// 同盟ボタン
				bool canAlliance = !relation.isAlliance && relation.relationValue >= 60;
				btnAlliance.draw(canAlliance ? Palette::Darkgreen : Palette::Darkgray);
				btnAlliance.drawFrame(1, Palette::White);

				FontAsset(U"menu")(U"同盟").drawAt(btnAlliance.center(), Palette::White);

				// 贈り物ボタン
				btnGift.draw(Palette::Goldenrod);
				btnGift.drawFrame(1, Palette::White);
				FontAsset(U"menu")(U"贈物(500)").drawAt(btnGift.center(), Palette::White);

				// 停戦ボタン
				btnTruce.draw(Palette::Steelblue);
				btnTruce.drawFrame(1, Palette::White);
				FontAsset(U"menu")(U"停戦").drawAt(btnTruce.center(), Palette::White);

				// 宣戦布告ボタン
				bool canWar = relation.CanWar();
				btnWar.draw(canWar ? Palette::Darkred : Palette::Darkgray);
				btnWar.drawFrame(1, Palette::White);
				FontAsset(U"menu")(U"宣戦布告").drawAt(btnWar.center(), Palette::White);
			}
		}

		// 戻るボタン
		m_btnBack.draw(Palette::Darkgray).drawFrame(2, Palette::White);
		FontAsset(U"menu")(U"戻る").drawAt(m_btnBack.center());

		// メッセージ
		if (!m_message.isEmpty())
		{
			RectF msgBox(Scene::Center().x - 300, Scene::Height() - 200, 600, 80);
			msgBox.draw(ColorF(0, 0, 0, 0.8));
			msgBox.drawFrame(2, Palette::Gold);
			FontAsset(U"menu")(m_message).drawAt(msgBox.center(), Palette::White);
		}
	}

private:
	DiplomacyManager* m_diplomacy;
	String m_playerFaction;
	Array<CityData>* m_allCities;
	GameManager* m_gameManager;
	Array<String> m_otherFactions;
	int m_selectedFaction = -1;
	Rect m_btnBack;
	String m_message;
};
