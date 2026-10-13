#include "CampaignScene.hpp"
#include "CampaignSave.hpp"
#include "CampaignUI.hpp"

using namespace frontline;
using namespace campaignui;

Rect CampaignScene::button(int index) const
{
	const int x = 16;
	if (m_started && !panelVisible() && index != 0 && index != 10 && index != 11 && index != 12 &&
	    index != 15 && index != 16 && index != 17 && index != 18 && index != 49 && index < 50 &&
	    (index < 19 || index >= 39))
		return Rect(-1000, -1000, 1, 1);
	const int right = Scene::Width() - 460;
	if (index >= 31 && index < 39)
		return Rect(80, 200 + (index - 31) * 42, 218, 36);
	switch (index)
	{
	case 49:
		return Rect(Scene::Width() - 246, Scene::Height() - 264, 82, 26);
	case 50:
		return Rect(16, Scene::Height() - 47, 100, 36);
	case 51:
		return Rect(232, Scene::Height() - 47, 116, 36);
	case 52:
		return Rect(464, Scene::Height() - 47, 100, 36);
	case 53:
		return Rect(Scene::Width() - 160, Scene::Height() - 264, 70, 26);
	case 54:
		return Rect(Scene::Width() - 86, Scene::Height() - 264, 28, 26);
	case 55:
		return Rect(Scene::Width() - 54, Scene::Height() - 264, 28, 26);
	case 59:
		return Rect(896, Scene::Height() - 47, 110, 36);
	case 58:
		return Rect(788, Scene::Height() - 47, 100, 36);
	case 57:
		return Rect(680, Scene::Height() - 47, 100, 36);
	case 56:
		return Rect(572, Scene::Height() - 47, 100, 36);
	case 0:
		return Rect(Scene::Width() - 246, 12, 230, 50);
	case 1:
		return Rect(x + 12, 282, 282, 34);
	case 2:
		return Rect(x + 12, 322, 282, 34);
	case 3:
		return Rect(x + 12, 370, 134, 34);
	case 4:
		return Rect(x + 158, 370, 136, 34);
	case 5:
		return Rect(x + 12, 412, 134, 34);
	case 6:
		return Rect(x + 158, 412, 136, 34);
	case 7:
		return Rect(x + 12, 458, 282, 34);
	case 8:
		return Rect(x + 12, 260, 282, 34);
	case 9:
		return Rect(x + 12, 302, 134, 34);
	case 10:
		return m_settingsOpen ? Rect(Scene::Width() - 296, 180, 126, 32) : Rect(-1000, -1000, 1, 1);
	case 11:
		return m_settingsOpen ? Rect(Scene::Width() - 162, 180, 126, 32) : Rect(-1000, -1000, 1, 1);
	case 12:
		return m_settingsOpen ? Rect(Scene::Width() - 296, 220, 260, 32) : Rect(-1000, -1000, 1, 1);
	case 13:
		return Rect(x + 12, 242, 90, 30);
	case 14:
		return Rect(x + 108, 242, 90, 30);
	case 15:
		return Rect(Scene::Width() - 362, 40, 104, 24);
	case 16:
		return m_game.hexMap ? Rect(-1000, -1000, 1, 1) : Rect(788, Scene::Height() - 47, 110, 36);
	case 17:
		return Rect(124, Scene::Height() - 47, 100, 36);
	case 18:
		return Rect(356, Scene::Height() - 47, 100, 36);
	case 19:
		return Rect(Scene::Width() - 170, 114, 90, 30);
	case 20:
		return Rect(80, 156, 218, 34);
	case 21:
		return Rect(right, 216, 370, 34);
	case 22:
		return Rect(right, 258, 370, 34);
	case 23:
		return Rect(right, 308, 370, 34);
	case 24:
		return Rect(right, 402, 370, 34);
	case 25:
		return Rect(right, 520, 176, 34);
	case 26:
		return Rect(right + 192, 520, 178, 34);
	case 27:
		return Rect(right, 562, 176, 34);
	case 28:
		return Rect(right + 192, 562, 178, 34);
	case 29:
		return Rect(80, Scene::Height() - 120, 134, 30);
	case 30:
		return Rect(228, Scene::Height() - 120, 134, 30);
	case 44:
		return Rect(x + 158, 302, 136, 34);
	case 43:
		return Rect(x + 12, 386, 282, 34);
	case 40:
		return Rect(x + 204, 242, 90, 30);
	case 41:
		return Rect(x + 12, 370, 282, 34);
	case 42:
		return Rect(x + 12, 458, 282, 34);
	case 39:
		return Rect(x + 12, 344, 282, 34);
	default:
		return Rect(x + 12, Scene::Height() - 138, 282, 34);
	}
}
