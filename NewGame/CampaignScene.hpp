#pragma once
#include <Siv3D.hpp>
#include "Campaign.hpp"
#include "MapCamera.hpp"
#include "HeroStoryScene.hpp"

class CampaignScene
{
public:
	CampaignScene();
	void update();
	void draw() const;
	void preview(int mode = 1);

private:
	frontline::Campaign m_game;
	world::MapCamera m_camera;
	RectF miniRect() const;
	void updateCamera();
	HeroStoryScene m_story;
	bool m_storyActive = false;
	bool m_started = false, m_supplyView = false;
	int m_city = -1, m_army = -1, m_generalChoice = 0, m_soldiers = 3000, m_daysLeft = 0;
	double m_timer = 0;
	int m_tab = 0, m_speed = 0;
	bool m_paused = false;
	int m_councilMode = 0, m_rosterFaction = 0, m_inspect = 0, m_partner = -1, m_targetCity = 3,
	    m_historyPage = 0;
	String m_message = U"都市を選んで出陣。部隊を選んで右クリックで進路を指示。";
	Array<Vec2> m_positions;
	struct Hit
	{
		Vec2 position;
		int amount;
		double time;
	};
	Array<Hit> m_hits;
	mutable RenderTexture m_mapTexture;
	mutable unsigned m_mapRevision = 0;
	mutable int m_mapDay = -1;
	mutable bool m_mapSupply = false;
	RectF mapRect() const;
	Vec2 tileCenter(int tile) const;
	int mouseTile() const;
	Rect button(int index) const;
	void deploy(frontline::Arm arm);
	void drawMap() const;
	void drawPanel() const;
	void drawMenu() const;
	void updateCouncil();
	void drawCouncil() const;
	static String text(const std::u32string& s)
	{
		return String(s.c_str());
	}
};
