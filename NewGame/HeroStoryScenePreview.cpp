#include "HeroStoryScene.hpp"

void HeroStoryScene::Preview(int mode)
{
	Open();
	if (mode != 4)
	{
		m_landing = false;
		m_story.Reset();
		if (mode == 21)
		{
			m_story.chapter = 3;
		}
		if (mode == 19)
		{
			m_story.chapter = 5;
			m_story.Choose(0);
			m_story.Choose(0);
			m_story.units[0].x = 4;
			m_story.units[0].y = 3;
			m_story.units[1].x = 4;
			m_story.units[1].y = 2;
			m_story.units[2].x = 6;
			m_story.units[2].y = 3;
			m_story.units[3].x = 3;
			m_story.units[3].y = 4;
			m_story.units[4].x = 4;
			m_story.units[4].y = 4;
			m_story.units[5].x = 6;
			m_story.units[5].y = 2;
			m_story.AdvanceObjective();
			m_story.AdvanceObjective();
			m_story.Guard(2);
			m_selectedUnit = 5;
		}
		if (mode == 6 || mode == 9 || mode == 10)
		{
			m_story.chapter = 3;
			m_story.phase = 2;
			m_story.StartMission();
			m_story.spirit = 72;
			if (mode == 9)
			{
				m_story.turn = 4;
				m_story.battleEvent = 1;
				m_story.eventMask = 1;
			}
			if (mode == 10)
				m_presentation.Moment(2, U"後衛の構え", StoryPresentation::Quote(2));
		}
		if (mode == 7)
		{
			m_story.chapter = 1;
			m_story.phase = 2;
			m_story.StartMission();
		}
		if (mode == 8)
		{
			m_story.chapter = 5;
			m_story.phase = 4;
		}
		if (mode == 12)
		{
			m_story.chapter = 3;
			m_story.phase = 1;
			m_story.planner = 3;
		}
		if (mode == 13)
		{
			m_story.chapter = 5;
			m_showNotes = true;
			m_notePage = 3;
		}
	}
}
