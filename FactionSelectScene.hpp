#pragma once
#include <Siv3D.hpp>
#include "SceneBase.hpp"
#include "GameManager.hpp"
#include "Faction.hpp"

class FactionSelectScene : public SceneBase
{
public:
	// 引数付きのコンストラクタに変更
	FactionSelectScene(GameManager* gm, Array<CityData>& cities)
		: m_gameManager(gm), m_cities(cities)
	{
		m_factions = {
			{ U"劉備", Palette::Green },
			{ U"曹操", Palette::Blue },
			{ U"袁術", Palette::Yellow },
			{ U"張楊", Palette::Purple },
			{ U"袁紹", Palette::Red}, 
			{ U"劉表", Palette::Skyblue }
		};
	}

	void update() override;
	void draw() const override;

	// 選択された勢力を返す関数
	Faction getSelectedFaction() const {
		if (m_selectedIndex >= 0 && m_selectedIndex < (int)m_factions.size())
			return m_factions[m_selectedIndex];
		return { U"無所属", Palette::Gray };
	}

private:
	GameManager* m_gameManager;
	Array<CityData>& m_cities;
	Array<Faction> m_factions;
	int m_selectedIndex = -1;
};
