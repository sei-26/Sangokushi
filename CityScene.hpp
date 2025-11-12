#pragma once
#include "SceneBase.hpp"

class CityScene : public SceneBase
{
private:
	String m_cityName;
	Font   m_font;
	Array<String> m_commands;
	int m_cursor = 0;

	int m_gold = 500;
	int m_food = 400;
	int m_order = 70;

public:
	explicit CityScene(const String& cityName);
	void update() override;
	void draw() const override;
};
