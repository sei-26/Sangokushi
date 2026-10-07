#include "HeroStoryScene.hpp"

void HeroStoryScene::update()
{
	fx.Tick(!landing && !notes && s.phase == 2 && s.Tactical() && !s.failed);
	if (Rect(Scene::Width() - 180, 26, 148, 38).leftClicked())
	{
		fx.Stop();
		back = true;
		return;
	}
	if (Rect(Scene::Width() - 310, 26, 115, 38).leftClicked())
	{
		fx.ToggleSound();
		return;
	}
	if (landing)
	{
		if (B(3).leftClicked())
		{
			s.Reset();
			landing = false;
			Save();
		}
		if (B(4).leftClicked())
		{
			hero::Story loaded;
			if (hero::LoadJSON(JSON::Load(U"hero-story-save.json"), loaded))
			{
				s = std::move(loaded);
				landing = false;
				selected = 0;
			}
			else
				message = U"読み込める英雄譚の保存がありません。";
		}
		return;
	}
	if (B(7).leftClicked())
	{
		notes = !notes;
		return;
	}
	if (B(8).leftClicked())
	{
		Save();
		return;
	}
	if (notes)
	{
		if (B(6).leftClicked())
			notePage = (notePage + 1) % 4;
		return;
	}
	if (fx.Busy())
		return;
	if (!MouseL.down())
		return;
	const hero::Story before = s;
	bool changed = false;
	if (s.phase == 1 && B(0).leftClicked())
	{
		s.AssignPlanner((s.planner + 1) % s.Unlocked());
		Save();
		return;
	}
	if (s.battleEvent && s.phase == 2 && !s.failed)
	{
		for (int i = 0; i < 2; ++i)
			if (BattleChoice(i).leftClicked())
			{
				changed = s.ResolveBattleEvent(i);
				fx.Moment(s.chapter == 3 ? (i == 0 ? 2 : 3) : 0,
				          i == 0 ? U"仲間と、踏みとどまる" : U"誰も、見捨てない",
				          s.chapter == 3 ? StoryPresentation::Quote(i == 0 ? 2 : 3)
				                         : StoryPresentation::Quote(0));
			}
		if (changed)
		{
			fx.Capture(before, s);
			Save();
		}
		return;
	}
	if (s.phase == 0 || s.phase == 1 || s.phase == 3)
	{
		for (int i = 0; i < 2; ++i)
			if (Choice(i).leftClicked())
			{
				changed = s.Choose(i);
				selected = 0;
			}
	}
	else if (s.phase == 4)
	{
		if (B(3).leftClicked())
		{
			landing = true;
			return;
		}
	}
	else if (s.failed)
	{
		if (B(3).leftClicked())
		{
			s.Retry();
			selected = 0;
			changed = true;
		}
	}
	else if (!s.Tactical())
	{
		for (int i = 0; i < 3; ++i)
			if (B(i).leftClicked())
				changed = s.CivilAction(i);
	}
	else
	{
		int row = 0;
		for (int i = 0; i < static_cast<int>(s.units.size()); ++i)
			if (s.units[i].hero >= 0)
			{
				if (Rect(38, 164 + row * 78, 205, 70).leftClicked())
					selected = i;
				++row;
			}
		auto r = Map();
		if (r.mouseOver() && MouseL.down())
		{
			auto p = Cursor::PosF() - r.pos;
			int x = static_cast<int>(p.x / (r.w / hero::W)), y = static_cast<int>(p.y / (r.h / hero::H));
			int u = s.At(x, y);
			if (u >= 0 && !s.units[u].enemy && !s.units[u].civilian)
				selected = u;
			else
				changed = s.Act(selected, x, y);
		}
		if (B(0).leftClicked())
		{
			changed = s.Skill(selected);
			if (changed)
				fx.Moment(s.units[selected].hero, T(hero::SkillName(s.units[selected].hero)),
				          StoryPresentation::Quote(s.units[selected].hero));
			else
				message = U"射程内に対象がいません。移動や負傷した味方の位置を確認してください。";
		}
		if (B(1).leftClicked())
		{
			s.EndTurn();
			changed = true;
		}
		if (s.chapter == 4 && B(2).leftClicked())
		{
			changed = s.FireSignal();
			if (changed)
				fx.Moment(4, U"江上、紅蓮に染まる", U"周瑜の火攻めが始まった。今こそ、連合軍の力を一つに！");
		}
		if (B(4).leftClicked())
		{
			changed = s.Rally();
			if (changed)
				fx.Moment(0, U"絆の号令", StoryPresentation::Quote(0));
		}
	}
	if (changed)
	{
		fx.Capture(before, s);
		if (before.phase == 1 && s.phase == 2 && s.Tactical())
			fx.Moment(s.chapter == 3 ? 2 : s.Companion(),
			          U"{} ― 開戦"_fmt(String(hero::Book()[s.chapter].place)),
			          StoryPresentation::Quote(s.chapter == 3 ? 2 : s.Companion()));
		Save();
	}
}
