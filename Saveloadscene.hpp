#pragma once
#include <Siv3D.hpp>
#include "SceneBase.hpp"
#include "SaveLoadManager.hpp"

// セーブ・ロード画面
class SaveLoadScene : public SceneBase
{
public:
	enum class Mode
	{
		Save,
		Load
	};

	SaveLoadScene(Mode mode, const SaveData* currentData = nullptr)
		: m_mode(mode)
	{
		if (currentData)
		{
			m_currentData = *currentData;
		}

		// ボタン配置
		for (int i = 0; i < 4; ++i)
		{
			m_slotButtons[i] = Rect(
				Scene::Width() / 2 - 300,
				200 + i * 100,
				600,
				80
			);
		}

		m_btnBack = Rect(50, Scene::Height() - 100, 200, 60);
	}

	void update() override
	{
		// スロット選択
		for (int i = 0; i < 4; ++i)
		{
			if (m_slotButtons[i].leftClicked())
			{
				if (m_mode == Mode::Save)
				{
					// セーブ実行
					if (SaveLoadManager::Save(m_currentData, i))
					{
						m_message = U"セーブしました！（スロット{}）"_fmt(i);
					}
					else
					{
						m_message = U"セーブに失敗しました";
					}
				}
				else
				{
					// ロード実行
					auto loadedData = SaveLoadManager::Load(i);
					if (loadedData)
					{
						m_loadedData = *loadedData;
						m_sceneEnd = true;
						m_nextScene = U"LoadGame";
					}
					else
					{
						m_message = U"ロードに失敗しました";
					}
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
		Scene::SetBackground(ColorF(0.1, 0.1, 0.15));

		// タイトル
		String title = (m_mode == Mode::Save) ? U"セーブ" : U"ロード";
		FontAsset(U"huge")(title).drawAt(Scene::Center().x, 80, Palette::Gold);

		// セーブスロット
		for (int i = 0; i < 4; ++i)
		{
			const Rect& btn = m_slotButtons[i];
			bool isHovered = btn.mouseOver();

			// スロット背景
			btn.draw(isHovered ? ColorF(0.3, 0.3, 0.4) : ColorF(0.2, 0.2, 0.25));
			btn.drawFrame(3, isHovered ? Palette::Gold : Palette::Gray);

			// スロット番号
			String slotLabel = (i == 0) ? U"[オート]" : U"スロット {}";
			FontAsset(U"title")(slotLabel._fmt(i))
				.draw(btn.x + 20, btn.y + 10, Palette::White);

			// セーブファイル情報
			String info = SaveLoadManager::GetSaveFileInfo(i);
			FontAsset(U"menu")(info)
				.draw(btn.x + 20, btn.y + 45,
					  SaveLoadManager::HasSaveFile(i) ? Palette::Lightgreen : Palette::Gray);
		}

		// 戻るボタン
		m_btnBack.draw(Palette::Darkgray).drawFrame(2, Palette::White);
		FontAsset(U"menu")(U"戻る").drawAt(m_btnBack.center());

		// メッセージ
		if (!m_message.isEmpty())
		{
			FontAsset(U"title")(m_message)
				.drawAt(Scene::Center().x, Scene::Height() - 150, Palette::Yellow);
		}
	}

	const SaveData& GetLoadedData() const { return m_loadedData; }

private:
	Mode m_mode;
	SaveData m_currentData;
	SaveData m_loadedData;
	Array<Rect> m_slotButtons = Array<Rect>(4);
	Rect m_btnBack;
	String m_message;
};
