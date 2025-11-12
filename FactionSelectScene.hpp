#pragma once
#include "SceneBase.hpp"
#include "Faction.hpp"

class FactionSelectScene : public SceneBase
{
private:
	Array<Faction> m_factions;
	int m_cursor = 0;
	Faction m_selected;

	Font m_titleFont;
	Font m_font;

public:
	FactionSelectScene();
	void update() override;
	void draw() const override;

	const Faction& getSelectedFaction() const { return m_selected; }
};
