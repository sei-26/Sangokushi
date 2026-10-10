#include "CampaignScene.hpp"
#include "CampaignSave.hpp"
#include "CampaignUI.hpp"

using namespace frontline;
using namespace campaignui;

void CampaignScene::preview(int mode)
{
	if (mode == 36 || mode == 37)
	{
		m_started = true;
		m_game.Reset(0);
		m_governCity = 0;
		m_game.AppointGovernor(0, 19);
		m_governChoice = 0;
		m_councilMode = 4;
		return;
	}
	if (mode >= 30 && mode <= 35)
	{
		preview(23);
		if (mode == 30)
			openInformation(0, 0);
		if (mode == 31)
		{
			openInformation(1, 24);
			m_infoSort = 1;
			const auto rows = informationRows();
			m_infoPage = static_cast<int>(std::find(rows.begin(), rows.end(), 24) - rows.begin()) /
			             Max(1, (Scene::Height() - 360) / 38);
		}
		if (mode == 32)
		{
			openInformation(1, 0);
			m_infoRelations = true;
		}
		if (mode == 33)
			openInformation(2, 0);
		if (mode == 34)
			openInformation(3, m_army);
		if (mode == 35)
			openInformation(4);
		return;
	}
	if (mode == 28 || mode == 29)
	{
		preview(23);
		m_presentation.ToggleSound(); // Automated previews are silent.
		m_game.armies[1].troops = m_game.armies[2].troops = 0;
		const int general = mode == 28 ? 24 : 5;
		m_game.armies[m_army].general = general;
		m_game.armies[m_army].arm = mode == 28 ? Arm::Cavalry : Arm::Spear;
		m_game.armies[m_army].morale = 85;
		m_game.armies[m_army].tacticReadyDay = 0;
		if (mode == 29)
		{
			m_game.armies[1].troops = 3000;
			m_game.armies[1].tile = m_game.armies[1].target = Campaign::At(50, 34);
			m_game.armies[1].tacticReadyDay = 1000;
		}
		m_game.ActivateTactic(m_army);
		m_game.AdvanceDay();
		m_presentation.Capture(m_game);
		m_presentation.Update(mode == 28 ? .62 : 1.12);
		m_message = U"武将の顔と戦法演出 / クリック・Spaceで次へ / Mで効果音切替";
		return;
	}
	if (mode == 27)
	{
		m_started = true;
		m_game.Reset(0);
		m_camera.Fit(Width, Height, m_game.hexMap, true);
		m_councilMode = 1;
		m_rosterFaction = 0;
		m_rosterPage = 1;
		m_inspect = 24;
		m_partner = -1;
		return;
	}
	if (mode == 26)
	{
		preview(14);
		return;
	}
	if (mode == 24)
	{
		m_started = true;
		m_game.Reset(0);
		m_camera.Fit(Width, Height, m_game.hexMap, true);
		m_region = 1;
		m_city = m_army = -1;
		const auto p =
		    hexgrid::Center(m_game.regions[m_region].tile % Width, m_game.regions[m_region].tile / Width);
		m_camera.x = p.first;
		m_camera.y = p.second;
		m_camera.zoom = 4;
		m_message = U"周辺の府を押さえ、都市圏の収入と進軍の足場を広げよう。";
		return;
	}
	if (mode == 23)
	{
		m_started = true;
		m_game.Reset(0);
		m_camera.Fit(Width, Height, m_game.hexMap, true);
		m_camera.zoom = 4;
		m_camera.x = 49;
		m_camera.y = 34 * hexgrid::Row + hexgrid::Radius;
		m_supplyView = true;
		m_army = m_game.Deploy(2, 4, 3000, Arm::Spear);
		const int left = m_game.Deploy(3, 6, 3000, Arm::Spear, true);
		const int right = m_game.Deploy(3, 7, 3000, Arm::Spear, true);
		for (int x = 47; x <= 51; ++x)
			for (int y = 32; y <= 36; ++y)
				m_game.tiles[Campaign::At(x, y)] = {Terrain::Plain, 0};
		m_game.armies[m_army].tile = m_game.armies[m_army].target = Campaign::At(49, 34);
		m_game.armies[left].tile = Campaign::At(48, 34);
		m_game.armies[right].tile = Campaign::At(50, 34);
		m_game.armies[left].target = m_game.armies[right].target = Campaign::At(49, 34);
		m_game.AdvanceDay();
		m_city = -1;
		m_message = U"二方向から包囲されると被害と士気損失が増える。固守、援軍、退路で戦列を立て直そう。";
		return;
	}
	if (mode == 22)
	{
		m_started = true;
		m_game.Reset(0);
		m_camera.Fit(Width, Height, m_game.hexMap, true);
		m_camera.zoom = 3.5;
		m_camera.x = m_game.cities[3].tile % Width;
		m_camera.y = hexgrid::Center(0, m_game.cities[3].tile / Width).second;
		m_army = m_game.Deploy(3, 6, 3000, Arm::Siege, true);
		m_game.Order(m_army, m_game.cities[2].tile);
		for (int d = 0; d < 3; ++d)
			m_game.AdvanceDay();
		m_city = -1;
		m_message = U"敵の攻城隊は支援の集結を待っている。補給を断つか、援軍を分断するか。";
		return;
	}
	if (mode == 20)
	{
		m_started = true;
		m_game.Reset(0);
		m_camera.Fit(Width, Height, m_game.hexMap, true);
		m_camera.zoom = 3.5;
		m_camera.x = 49;
		m_camera.y = 34 * hexgrid::Row + hexgrid::Radius;
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
		m_camera.Fit(Width, Height, m_game.hexMap, true);
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
		m_camera.Fit(Width, Height, m_game.hexMap, true);
		m_camera.zoom = 4;
		m_camera.x = 22;
		m_camera.y = 40 * hexgrid::Row + hexgrid::Radius;
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
		m_camera.Fit(Width, Height, m_game.hexMap, true);
		m_city = -1;
		m_camera.zoom = 1.8;
		if (mode == 15)
		{
			m_camera.zoom = 3.5;
			m_camera.x = 49;
			m_camera.y = 34 * hexgrid::Row + hexgrid::Radius;
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
