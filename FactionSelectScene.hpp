#pragma once
#include <Siv3D.hpp>
#include "SceneBase.hpp"
#include "Faction.hpp"

class FactionSelectScene : public SceneBase
{
private:
	Array<Faction> m_factions;
	int m_selectedIndex = -1;

public:
	FactionSelectScene();

	void update() override;
	void draw() const override;

	const Faction& getSelectedFaction() const
	{
		return m_factions[m_selectedIndex];
	}
};
