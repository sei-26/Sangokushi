#include "CampaignScene.hpp"
#include "CampaignSave.hpp"
#include "CampaignUI.hpp"

using namespace frontline;
using namespace campaignui;

void CampaignScene::preview(int mode)
{
	if (mode == 20)
	{
		m_started = true;
		m_game.Reset(0);
		m_camera.Fit(Width, Height);
		m_camera.zoom = 3.5;
		m_camera.x = 49;
		m_camera.y = 34;
		m_army = m_game.Deploy(2, 4, 3000, Arm::Bow);
		m_game.SetStance(m_army, battle::Stance::Guard);
		m_game.Order(m_army, m_game.cities[3].tile);
		m_city = -1;
		m_message = U"固守する弓兵と、攻勢の突破役。林・山に遮られない射線を選ぼう。";
		return;
	}
	if (mode == 16)
	{
		m_started = true;
		m_game.Reset(0);
		m_camera.Fit(Width, Height);
		m_councilMode = 3;
		m_inspect = 19;
		m_assignmentTarget = 9;
		m_game.AssignOfficer(19, 9);
		return;
	}
	if (mode == 11)
	{
		m_started = false;
		m_storyActive = false;
		return;
	}
	if (mode == 17 || mode == 18)
	{
		m_started = true;
		m_game.Reset(0);
		m_camera.Fit(Width, Height);
		m_camera.zoom = 4;
		m_camera.x = 22;
		m_camera.y = 40;
		m_city = 0;
		m_tab = 2;
		m_transportTarget = 9;
		m_generalChoice = 2;
		m_message = U"後方の蓄えを街道で運び、前線を支えよう。";
		if (mode == 17)
		{
			m_army = m_game.DispatchTransport(0, 19, 9, 10000);
			m_city = -1;
			for (int d = 0; d < 5; ++d)
				m_game.AdvanceDay();
		}
		return;
	}
	if (mode == 14 || mode == 15)
	{
		m_started = true;
		m_game.Reset(0);
		m_camera.Fit(Width, Height);
		m_city = 2;
		if (mode == 15)
		{
			m_camera.zoom = 3.5;
			m_camera.x = 49;
			m_camera.y = 34;
			m_supplyView = true;
			const int a = m_game.Deploy(2, 4, 3000, Arm::Spear);
			m_game.Order(a, m_game.cities[3].tile);
			m_army = a;
			m_city = -1;
		}
		return;
	}
	if (mode >= 4)
	{
		m_storyActive = true;
		m_story.Preview(mode);
		return;
	}
	m_started = true;
	m_game.Reset(0);
	m_city = 2;
	const int a = m_game.Deploy(2, 4, 3000, Arm::Spear);
	m_game.Order(a, m_game.cities[3].tile);
	const int b = m_game.Deploy(2, 5, 3000, Arm::Siege);
	m_game.Order(b, m_game.cities[3].tile);
	m_game.BeginTurn();
	for (int i = 0; i < 6; ++i)
		m_game.AdvanceDay();
	m_army = a;
	m_city = -1;
	m_supplyView = true;
	m_city = 0;
	m_army = -1;
	m_tab = 1;
	m_generalChoice = 1;
	m_game.Develop(0, 0, Duty::Commerce);
	for (int i = 0; i < 4; ++i)
		m_game.AdvanceDay();
	m_councilMode = 1;
	m_inspect = 19;
	m_partner = 1;
	m_rosterFaction = 0;
	if (mode == 2)
	{
		m_game.randomState = 1;
		m_game.SendMission(0, 19, 3, MissionKind::Diplomacy, 1);
		for (int i = 0; i < 20; ++i)
			m_game.AdvanceDay();
		m_councilMode = 2;
	}
	else if (mode == 3)
	{
		m_army = a;
		m_city = -1;
		m_councilMode = 0;
		m_game.ActivateTactic(a);
		m_game.AdvanceDay();
	}
	for (const auto& army : m_game.armies)
		m_positions.push_back(tileCenter(army.tile));
}
