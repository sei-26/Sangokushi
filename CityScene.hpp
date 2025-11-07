#pragma once
#include "SceneBase.hpp"

class CityScene : public SceneBase
{
private:
	String m_cityName;
	Font m_font;  // ← フォントをメンバーに！

public:
	explicit CityScene(const String& cityName)
		: m_cityName(cityName)
		, m_font(28) // ← コンストラクタで1回だけ生成！
	{
		Print << U"CityScene 開始：" << m_cityName;
	}

	void update() override
	{
		if (KeyEscape.down())
		{
			m_isEnd = true;
			m_nextScene = U"WorldMapScene";
		}
	}

	void draw() const override
	{
		Scene::SetBackground(ColorF{ 0.15, 0.15, 0.2 });

		m_font(Format(U"🏯 都市：{}", m_cityName))
			.draw(50, 100, Palette::Yellow);

		m_font(U"ESCでマップに戻る")
			.draw(50, 150, Palette::White);
	}
};
