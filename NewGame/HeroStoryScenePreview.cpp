#include "HeroStoryScene.hpp"

void HeroStoryScene::Preview(int mode)
{
	if (mode == 49 || mode == 50)
	{
		Preview(mode == 49 ? 19 : 40);
		m_presentation.Skip();
		const hero::Story before = m_story;
		if (mode == 49)
			m_story.Skill(5);
		else
		{
			m_story.turn = 3;
			m_story.FireSignal();
		}
		m_presentation.Capture(before, m_story, mode == 49 ? 5 : -1);
		if (mode == 50)
		{
			m_presentation.Moment(4, U"紅蓮の火攻め", StoryPresentation::Quote(4));
			m_presentation.Tick(true, 1.98);
		}
		else
			m_presentation.Tick(true, .38);
		return;
	}
	Open();
	if (m_musicPlayer.Enabled())
		m_musicPlayer.Toggle();
	m_presentation.ToggleSound(); // Automated screenshot previews are silent.
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
	if (mode == 38)
	{
		m_landing = false;
		m_story.Reset();
		m_chapterIntro = 2.6;
	}
	if (mode == 39)
	{
		m_landing = false;
		m_story.Reset();
		m_story.chapter = 3;
		m_dialogueBeat = 0;
	}
	if (mode == 40)
	{
		m_landing = false;
		m_story.Reset();
		m_story.chapter = 4;
		m_story.Choose(0);
		m_story.Choose(0);
		m_presentation.Moment(4, U"江上、紅蓮に染まる", U"退路を守り、仲間と勝機をつかめ！");
	}
	m_presentation.Chapter(m_story.chapter);
	m_dialogueAge = 100;
	if (mode == 10 || mode == 40)
		m_presentation.Tick(false, .6);
	if (mode != 39)
		m_dialogueBeat = 2;
}
