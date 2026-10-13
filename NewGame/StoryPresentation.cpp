#include "StoryPresentation.hpp"

bool StoryPresentation::Busy() const
{
	return m_moment > .15 || !m_motion.empty() || m_numbers.any([](const Number& n) { return n.age < .65; });
}

void StoryPresentation::Moment(int who, const String& title, const String& quote)
{
	m_hero = Clamp(who, 0, 5);
	m_title = title;
	m_quote = quote;
	m_moment = MomentSeconds;
	m_celebration = false;
	if (m_sound && m_loaded)
		m_call.playOneShot(.4);
}

String StoryPresentation::Quote(int who)
{
	const String lines[]{
	    U"皆の志、ここで途切れさせはしない！",     U"我が刃で、兄者の道を開く！",
	    U"ここを通りたくば、俺を越えてみろ！",     U"救いを求める声がある限り、私は退かない！",
	    U"一人の力を、皆の勝機につなげましょう。", U"この一射、若き者たちの未来へ！"};
	return lines[Clamp(who, 0, 5)];
}

void StoryPresentation::Capture(const hero::Story& before, const hero::Story& after, int actor)
{
	bool hit = false;
	for (size_t i = 0; i < Min(before.units.size(), after.units.size()); ++i)
	{
		const auto& a = before.units[i];
		const auto& b = after.units[i];
		if (a.hp > 0 && b.hp > 0 && (a.x != b.x || a.y != b.y))
			m_motion.push_back({static_cast<int>(i), Vec2(a.x, a.y), Vec2(b.x, b.y)});
		int delta = b.hp - a.hp;
		if (delta != 0 && !(a.civilian && b.hp == 0 && after.escaped > before.escaped))
		{
			m_numbers.push_back({Vec2(b.x, b.y), delta, -.07 * m_numbers.size()});
			auto& number = m_numbers.back();
			number.defeated = b.hp == 0;
			if (actor >= 0 && actor < static_cast<int>(before.units.size()))
			{
				const auto& source = before.units[actor];
				number.source = Vec2(source.x, source.y);
				number.hasSource = actor != static_cast<int>(i);
				number.ranged = source.range > 1 || source.hero == 5;
			}
			if (delta < 0)
				hit = true;
		}
	}
	if (hit && m_sound && m_loaded)
		m_hit.playOneShot(.45);
	if (after.phase == 4 && before.phase != 4)
	{
		Moment(0, U"誓いは、ここから続く", U"皆がつないだこの道を、次の時代へ運ぼう。");
		m_celebration = true;
		if (m_sound && m_loaded)
			m_win.playOneShot(.5);
	}
	else if (after.phase == 3 && before.phase == 2)
	{
		Moment(after.Companion(), U"勝利 — " + String(hero::Book()[after.chapter].title),
		       Quote(after.Companion()));
		m_celebration = true;
		if (m_sound && m_loaded)
			m_win.playOneShot(.5);
	}
	else if (after.failed && !before.failed)
		Moment(0, U"旗は、まだ折れていない", U"皆、生きて戻れ。もう一度、道をつなごう。");
	else if (after.deepRules && before.objectiveProgress < 2 && after.objectiveProgress >= 2)
		Moment(after.Companion(), U"拠点確保 — " + String(after.ObjectiveName().c_str()),
		       U"仲間の働きが、戦場の流れを変えた！");
	else if (after.escaped > before.escaped)
		Moment(3, U"民、渡し場へ", U"ありがとう……どうか、あなた方も生きて！");
}
