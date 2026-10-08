#include "HeroStoryScene.hpp"

void HeroStoryScene::update()
{
	m_presentation.Tick(!m_landing && !m_showNotes && m_story.phase == 2 && m_story.Tactical() &&
	                    !m_story.failed);
	if (Rect(Scene::Width() - 180, 26, 148, 38).leftClicked())
	{
		m_presentation.Stop();
		m_backRequested = true;
		return;
	}
	if (Rect(Scene::Width() - 310, 26, 115, 38).leftClicked())
	{
		m_presentation.ToggleSound();
		return;
	}
	if (m_landing)
	{
		if (buttonRect(3).leftClicked())
		{
			m_story.Reset();
			m_landing = false;
			save();
		}
		if (buttonRect(4).leftClicked())
		{
			hero::Story loaded;
			if (hero::LoadJSON(JSON::Load(U"hero-story-save.json"), loaded))
			{
				m_story = std::move(loaded);
				m_landing = false;
				m_selectedUnit = 0;
			}
			else
				m_message = U"読み込める英雄譚の保存がありません。";
		}
		return;
	}
	if (buttonRect(7).leftClicked())
	{
		m_showNotes = !m_showNotes;
		return;
	}
	if (buttonRect(8).leftClicked())
	{
		save();
		return;
	}
	if (m_showNotes)
	{
		if (buttonRect(6).leftClicked())
			m_notePage = (m_notePage + 1) % 4;
		return;
	}
	if (m_presentation.Busy())
		return;
	if (!MouseL.down())
		return;
	const hero::Story before = m_story;
	bool changed = false;
	if (m_story.phase == 1 && buttonRect(0).leftClicked())
	{
		m_story.AssignPlanner((m_story.planner + 1) % m_story.Unlocked());
		save();
		return;
	}
	if (m_story.battleEvent && m_story.phase == 2 && !m_story.failed)
	{
		for (int i = 0; i < 2; ++i)
			if (battleChoiceRect(i).leftClicked())
			{
				changed = m_story.ResolveBattleEvent(i);
				m_presentation.Moment(m_story.chapter == 3 ? (i == 0 ? 2 : 3) : 0,
				                      i == 0 ? U"仲間と、踏みとどまる" : U"誰も、見捨てない",
				                      m_story.chapter == 3 ? StoryPresentation::Quote(i == 0 ? 2 : 3)
				                                           : StoryPresentation::Quote(0));
			}
		if (changed)
		{
			m_presentation.Capture(before, m_story);
			save();
		}
		return;
	}
	if (m_story.phase == 0 || m_story.phase == 1 || m_story.phase == 3)
	{
		for (int i = 0; i < 2; ++i)
			if (choiceRect(i).leftClicked())
			{
				changed = m_story.Choose(i);
				m_selectedUnit = 0;
			}
	}
	else if (m_story.phase == 4)
	{
		if (buttonRect(3).leftClicked())
		{
			m_landing = true;
			return;
		}
	}
	else if (m_story.failed)
	{
		if (buttonRect(3).leftClicked())
		{
			m_story.Retry();
			m_selectedUnit = 0;
			changed = true;
		}
	}
	else if (!m_story.Tactical())
	{
		for (int i = 0; i < 3; ++i)
			if (buttonRect(i).leftClicked())
				changed = m_story.CivilAction(i);
	}
	else
	{
		int row = 0;
		for (int i = 0; i < static_cast<int>(m_story.units.size()); ++i)
			if (m_story.units[i].hero >= 0)
			{
				if (Rect(38, 164 + row * 78, 205, 70).leftClicked())
					m_selectedUnit = i;
				++row;
			}
		auto r = mapRect();
		if (r.mouseOver() && MouseL.down())
		{
			auto p = Cursor::PosF() - r.pos;
			int x = static_cast<int>(p.x / (r.w / hero::W)), y = static_cast<int>(p.y / (r.h / hero::H));
			int u = m_story.At(x, y);
			if (u >= 0 && !m_story.units[u].enemy && !m_story.units[u].civilian)
				m_selectedUnit = u;
			else
				changed = m_story.Act(m_selectedUnit, x, y);
		}
		if (buttonRect(0).leftClicked())
		{
			changed = m_story.Skill(m_selectedUnit);
			if (changed)
				m_presentation.Moment(m_story.units[m_selectedUnit].hero,
				                      text(hero::SkillName(m_story.units[m_selectedUnit].hero)),
				                      StoryPresentation::Quote(m_story.units[m_selectedUnit].hero));
			else
				m_message = U"射程内に対象がいません。移動や負傷した味方の位置を確認してください。";
		}
		if (buttonRect(1).leftClicked())
		{
			m_story.EndTurn();
			changed = true;
		}
		if (m_story.chapter == 4 && buttonRect(2).leftClicked())
		{
			changed = m_story.FireSignal();
			if (changed)
				m_presentation.Moment(4, U"江上、紅蓮に染まる",
				                      U"周瑜の火攻めが始まった。今こそ、連合軍の力を一つに！");
		}
		if (buttonRect(9).leftClicked())
			changed = m_story.Guard(m_selectedUnit);
		if (buttonRect(4).leftClicked())
		{
			changed = m_story.Rally();
			if (changed)
				m_presentation.Moment(0, U"絆の号令", StoryPresentation::Quote(0));
		}
	}
	if (changed)
	{
		m_presentation.Capture(before, m_story);
		if (before.phase == 1 && m_story.phase == 2 && m_story.Tactical())
			m_presentation.Moment(m_story.chapter == 3 ? 2 : m_story.Companion(),
			                      U"{} ― 開戦"_fmt(String(hero::Book()[m_story.chapter].place)),
			                      StoryPresentation::Quote(m_story.chapter == 3 ? 2 : m_story.Companion()));
		save();
	}
}
