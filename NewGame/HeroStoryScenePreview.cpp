#include "HeroStoryScene.hpp"

void HeroStoryScene::Preview(int mode)
{
	Open();
	if (mode != 4)
	{
		landing = false;
		s.Reset();
		if (mode == 6 || mode == 9 || mode == 10)
		{
			s.chapter = 3;
			s.phase = 2;
			s.StartMission();
			s.spirit = 72;
			if (mode == 9)
			{
				s.turn = 4;
				s.battleEvent = 1;
				s.eventMask = 1;
			}
			if (mode == 10)
				fx.Moment(2, U"後衛の構え", StoryPresentation::Quote(2));
		}
		if (mode == 7)
		{
			s.chapter = 1;
			s.phase = 2;
			s.StartMission();
		}
		if (mode == 8)
		{
			s.chapter = 5;
			s.phase = 4;
		}
		if (mode == 12)
		{
			s.chapter = 3;
			s.phase = 1;
			s.planner = 3;
		}
		if (mode == 13)
		{
			s.chapter = 5;
			notes = true;
			notePage = 3;
		}
	}
}
