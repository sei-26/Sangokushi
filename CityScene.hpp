#pragma once
#include "SceneBase.hpp"

class CityScene : public SceneBase
{
private:
	String m_cityName;

public:
	explicit CityScene(const String& cityName)
		: m_cityName(cityName)
	{
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
		Font font(28);
		font(Format(U"🏯 都市：{}", m_cityName)).draw(50, 100, Palette::Yellow);
		font(U"ESCでマップに戻る").draw(50, 150, Palette::White);
	}
};
