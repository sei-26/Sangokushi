#pragma once
#include <Siv3D.hpp>
#include "Campaign.hpp"
#include "CampaignMapArt.hpp"
#include "AtlasCamera.hpp"
#include "CampaignPresentation.hpp"
#include "CampaignSpectacle.hpp"
#include "MusicPlayer.hpp"
#include "MapCamera.hpp"
#include "HeroStoryScene.hpp"
#include "StrategistWarScene.hpp"
#include "StrategistJourneyScene.hpp"

class CampaignScene
{
public:
	CampaignScene();
	void update();
	void draw() const;
	void preview(int mode = 1);

private:
	frontline::Campaign m_game;
	StrategistJourneyScene m_journey;
	bool m_journeyActive = false;
	world::MapCamera m_camera;
	RectF miniRect() const;
	void updateCamera();
	HeroStoryScene m_story;
	bool m_storyActive = false;
	StrategistWarScene m_strategist;
	bool m_strategistActive = false;
	bool m_started = false, m_supplyView = false;
	bool m_territoryView = true;
	mutable bool m_mapTerritory = false;
	bool m_settingsOpen = false;
	bool m_atlasView = false;
	bool m_gridVisible = true;
	void drawHexGrid(double cell) const;
	Texture m_atlasTexture;
	Image m_atlasSource;
	mutable Texture m_atlasTerritory;
	mutable unsigned m_atlasOwnerHash=0;
	chinaatlas::Camera m_atlasCamera;
	RectF atlasViewport() const;
	RectF atlasRect() const;
	void updateAtlasCamera();
	void drawAtlasContents() const;
	Vec2 atlasPosition(int tile) const;
	void drawAtlas() const;
	void ensureAtlasGrid() const;
	int atlasGridCellAt(Vec2 point) const;
	mutable RenderTexture m_atlasGrid;
	mutable Array<Polygon> m_atlasCells;
	void drawHUD() const;
	void updateSettings();
	int m_region = -1;
	int m_city = -1, m_army = -1, m_generalChoice = 0, m_soldiers = 3000, m_daysLeft = 0;
	double m_timer = 0;
	int m_tab = 0, m_speed = 0;
	bool m_paused = false;
	int m_councilMode = 0, m_rosterFaction = 0, m_inspect = 0, m_partner = -1, m_targetCity = 3,
	    m_historyPage = 0, m_rosterPage = 0;
	String m_message = U"都市を選んで出陣。部隊を選んで右クリックで進路を指示。";
	Array<Vec2> m_positions;
	struct Hit
	{
		Vec2 position;
		int amount;
		double time;
	};
	Array<Hit> m_hits;
	Texture m_campaignFaces{U"StoryArt/heroes.png"};
	campaignvisual::MapArt m_mapArt;
	CampaignPresentation m_presentation;
	CampaignSpectacle m_spectacle;
	MusicPlayer m_musicPlayer;
	mutable RenderTexture m_mapTexture;
	mutable unsigned m_mapRevision = 0;
	mutable int m_mapDay = -1;
	mutable bool m_mapSupply = false;
	bool m_panelHidden = false;
	bool panelVisible() const;
	bool overMapUI() const;
	void fitMap();
	RectF mapRect() const;
	Vec2 tileCenter(int tile) const;
	int mouseTile() const;
	Rect button(int index) const;
	void deploy(frontline::Arm arm);
	int returnTarget() const;
	void updateReturnOrders();
	int m_returnCity = -1, m_returnArmy = -1;
	void drawMap() const;
	void drawTerrainRelief(double cell) const;
	void drawTerrainTransitions(double pixels) const;
	void drawCityLandscape(double pixels) const;
	void drawWaterSurface(double cell) const;
	RectF territoryLegendRect() const;
	void drawTerritoryLegend() const;
	void drawSieges(double cell) const;
	void drawFormationLinks(double cell) const;
	void updateFormation();
	void drawFormation() const;
	int m_trainingFirst=-1,m_trainingSecond=-1,m_trainingPage=0,m_trainingSlot=0;
	void updateTraining();
	void drawTraining()const;
	void drawPanel() const;
	void drawRegions(double cell) const;
	void drawRegionPanel() const;
	void updateTransport();
	void drawTransport() const;
	int transportTarget() const;
	mutable int m_transportEstimateFrom = -1, m_transportEstimateTo = -1, m_transportEstimateDay = -1,
	            m_transportEstimate = -1;
	mutable unsigned m_transportEstimateRevision = 0;
	int m_transportTarget = -1, m_cargo = 5000;
	void drawMenu() const;
	bool m_infoOpen = false, m_infoRelations = false;
	int m_infoTab = 0, m_infoFaction = -1, m_infoSort = 0, m_infoPage = 0, m_infoSelection = -1,
	    m_infoDetailPage = 0;
	void openInformation(int tab = 0, int selected = -1);
	void updateInformation();
	void drawInformation() const;
	void drawInformationDetail() const;
	void focusInformation();
	std::vector<int> informationRows() const;
	String informationPosting(int officer) const;

	void updateCouncil();
	void updateRosterPage(int faction);
	void drawRosterPages(int faction) const;
	void drawCouncil() const;
	int m_governCity = -1, m_governChoice = -1, m_governPage = 0;
	void updateGovernance();
	void drawGovernance() const;
	int m_domesticCity = -1, m_domesticDuty = 0, m_domesticOfficer = -1;
	void openDomestic(int city = -1);
	void updateDomestic();
	void drawDomestic() const;
	std::vector<int> domesticCandidates() const;
	String domesticBlockReason() const;
	int m_policySlot = 0, m_policyOfficer = -1, m_policyChoice = 0, m_policyPage = 0;
	void openPolicies();
	void updatePolicies();
	void drawPolicies() const;
	std::vector<int> policyCandidates() const;
	void updateAssignments();
	void drawAssignments() const;
	int m_assignmentTarget = -1;
	int m_assignmentPage = 0;
	static String text(const std::u32string& s)
	{
		return String(s.c_str());
	}
};
