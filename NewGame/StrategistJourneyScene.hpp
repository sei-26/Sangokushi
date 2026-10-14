#pragma once
#include <Siv3D.hpp>
#include <memory>
#include "StrategistJourney.hpp"
#include "CampaignMapArt.hpp"
#include "StoryPresentation.hpp"
#include "MusicPlayer.hpp"

class StrategistJourneyScene
{
public:
	void Open();
	void Update();
	void Draw() const;
	void Preview(int mode);
	bool Back() const
	{
		return m_back;
	}
	bool Started() const
	{
		return m_initialized;
	}

private:
	frontline::StrategistJourney m_journey;
	std::unique_ptr<frontline::StrategistJourney> m_checkpoint;
	campaignvisual::MapArt m_art;
	Texture m_faces{U"StoryArt/heroes.png"};
	StoryPresentation m_fx;
	MusicPlayer m_music;
	bool m_back = false, m_initialized = false;
	int m_beat = 0, m_selected = 0, m_pick = 0, m_enemy = -1, m_choice = 0, m_policy = 0;
	int m_replay = -1;
	double m_replayTime = 0;
	double m_zoom = 1.65;
	Vec2 m_camera{9.5, 9.2 * hexgrid::Row + hexgrid::Radius};
	std::vector<frontline::ChainOrder> m_orders;
	RectF MapRect() const;
	double Cell() const;
	Vec2 Position(int tile) const;
	int Pick(Vec2 point) const;
	int EnemyAt(int tile) const;
	Rect Control(int id) const;
	void Sample();
	void StartReplay();
	void DrawMap() const;
	void DrawEncounter() const;
	void DrawPreparation() const;
	void DrawDebrief() const;
	static String Text(const std::u32string& value)
	{
		return String(value.c_str());
	}
};
