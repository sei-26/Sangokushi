#pragma once
#include <Siv3D.hpp>
#include "StrategistBattle.hpp"
#include "CampaignMapArt.hpp"
#include "MusicPlayer.hpp"

class StrategistWarScene
{
public:
	void Open();
	void Update();
	void Draw() const;
	bool Back() const
	{
		return m_back;
	}
	void Preview(int mode);

private:
	frontline::StrategistBattle m_battle;
	campaignvisual::MapArt m_art;
	Texture m_faces{U"StoryArt/heroes.png"};
	MusicPlayer m_music;
	bool m_back = false, m_council = true, m_paused = false;
	int m_selected = 0, m_daysLeft = 0;
	double m_timer = 0;
	String m_message = U"軍議で方針を決めよう。";
	bool m_operationOpen = false;
	int m_pickPurpose = 0, m_taskKind = 0;
	frontline::BattleOperation m_draft;
	void OpenOperation();
	void UpdateOperation();
	void DrawOperation() const;
	Rect OperationControl(int id) const;
	RectF MapRect() const;
	double Cell() const;
	Vec2 Position(int tile) const;
	int Pick(Vec2 position) const;
	Rect Control(int id) const;
	void DrawMap() const;
	void DrawCouncil() const;
	void DrawResult() const;
	static String Text(const std::u32string& value)
	{
		return String(value.c_str());
	}
};
