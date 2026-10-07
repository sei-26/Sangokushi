#include "StoryPresentation.hpp"

bool StoryPresentation::Busy() const
{
	return m_moment > .75;
}

void StoryPresentation::Moment(int who, const String& title, const String& quote)
{
	m_hero = Clamp(who, 0, 5);
	m_title = title;
	m_quote = quote;
	m_moment = 1.5;
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

void StoryPresentation::Capture(const hero::Story& before, const hero::Story& after)
{
	bool hit = false;
	for (size_t i = 0; i < Min(before.units.size(), after.units.size()); ++i)
	{
		const auto& a = before.units[i];
		const auto& b = after.units[i];
		int delta = b.hp - a.hp;
		if (delta != 0 && !(a.civilian && b.hp == 0 && after.escaped > before.escaped))
		{
			m_numbers.push_back({Vec2(b.x, b.y), delta, -.07 * m_numbers.size()});
			if (delta < 0)
				hit = true;
		}
	}
	if (hit && m_sound && m_loaded)
		m_hit.playOneShot(.45);
	if (after.escaped > before.escaped)
		Moment(3, U"民、渡し場へ", U"ありがとう……どうか、あなた方も生きて！");
	else if (after.phase == 3 && before.phase == 2)
	{
		Moment(0, U"志を、つなぎとめた", U"この勝利を、明日を生きる力に。皆、よく戦ってくれた！");
		if (m_sound && m_loaded)
			m_win.playOneShot(.5);
	}
}
