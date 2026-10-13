#include "CampaignSpectacle.hpp"
void CampaignSpectacle::Reset()
{
	m_reports.clear();
	m_lastSpoke.clear();
	m_age = m_soundAge = 0;
	m_signaled = false;
	m_playing = false;
	if (m_ambience)
		m_ambience.stop();
	if (m_signal)
		m_signal.stopAllShots();
}
void CampaignSpectacle::Update(double dt, bool active, bool sound, const frontline::Campaign& game)
{
	if (!sound && m_signal)
		m_signal.stopAllShots();
	if (!active || !m_enabled)
	{
		if (m_signal)
			m_signal.stopAllShots();
		if (m_ambience)
			m_ambience.stop();
		m_playing = false;
		return;
	}
	m_time += Max(0., dt);
	m_soundAge = Max(0., m_soundAge - dt);
	if (!m_reports.empty())
	{
		if (!ReportRect().mouseOver())
			m_age += dt;
		if (m_age > 5.5)
		{
			Dismiss();
		}
	}
	bool fighting = false;
	for (int i = 0; i < static_cast<int>(game.armies.size()) && !fighting; ++i)
		for (int j = i + 1; j < static_cast<int>(game.armies.size()); ++j)
		{
			const auto& a = game.armies[i];
			const auto& b = game.armies[j];
			if (a.troops > 0 && b.troops > 0 && game.Hostile(a.faction, b.faction) &&
			    game.MapDistance(a.tile, b.tile) <= 2)
			{
				fighting = true;
				break;
			}
		}
	for (const auto& a : game.armies)
		if (a.troops > 0 && !a.retreat)
		{
			const int city = game.CityAt(a.target);
			if (city >= 0 && game.Hostile(a.faction, game.cities[city].owner) &&
			    game.MapDistance(a.tile, a.target) <= 1)
				fighting = true;
		}
	if (sound && (fighting || !m_reports.empty()) && !m_loaded)
	{
		m_ambience = Audio(Ambience(false), Loop::Yes);
		m_signal = Audio(Ambience(true));
		m_loaded = true;
	}
	if (sound && fighting && !m_playing)
	{
		m_ambience.setVolume(.18).play();
		m_playing = true;
	}
	if ((!sound || !fighting) && m_playing)
	{
		m_ambience.stop();
		m_playing = false;
	}
	if (!sound && m_signal)
		m_signal.stopAllShots();
	if (sound && !m_reports.empty() && !m_signaled && m_soundAge == 0 && m_loaded)
	{
		m_signal.playOneShot(.3);
		m_signaled = true;
		m_soundAge = 3;
	}
}
void CampaignSpectacle::Capture(const battlefieldmoments::Snapshot& before, const frontline::Campaign& game)
{
	if (!m_enabled)
		return;
	for (const auto& e : battlefieldmoments::Changes(before, game))
	{
		if (e.general < 0 || e.general >= static_cast<int>(game.generals.size()))
			continue;
		const bool major = e.kind == battlefieldmoments::Kind::Capture;
		if (!major && (m_reports.size() >= 4 ||
		               (m_lastSpoke.count(e.general) && game.day - m_lastSpoke[e.general] < 10)))
			continue;
		Report r;
		r.event = e;
		r.officer = game.generals[e.general];
		r.friendly = e.faction == game.player;
		using K = battlefieldmoments::Kind;
		if (e.kind == K::Capture)
		{
			r.title = String(game.cities[e.city].name.c_str()) + (r.friendly ? U" 攻略"
			                                                      : before.owners[e.city] == game.player
			                                                          ? U" 陥落"
			                                                          : U" 戦況変化");
			r.quote = r.friendly ? U"城を得た。兵を休め、次の戦線を支える備えを整えよ！"
			                     : U"戦線が動いた。次に守るべき拠点を見定めよ。";
		}
		if (e.kind == K::Capture && !r.friendly && before.owners[e.city] == game.player)
		{
			const int leader = game.Leader(game.player);
			if (leader >= 0)
				r.officer = game.generals[leader];
			r.quote = U"城を失った。残る仲間を集め、次に守る拠点を見定めよ。";
		}
		else if (e.kind == K::Defeat)
		{
			r.title = r.friendly ? U"部隊敗走" : U"敵部隊撃破";
			r.quote = r.friendly ? U"生きて帰るぞ。立て直して、もう一度旗を掲げる！"
			                     : U"敵の一隊が戦線を離れた。残る部隊と城の動きを警戒せよ。";
		}
		else if (e.kind == K::Relief)
		{
			r.title = U"補給路 復旧";
			r.quote = U"兵糧が届いた！ 街道を守る仲間の働きを、ここで勝機につなぐ。";
		}
		else if (e.kind == K::SupplyLost)
		{
			r.title = U"補給路 途絶";
			r.quote = U"糧の道が切れた。携行糧が尽きる前に、救援か帰還を！";
		}
		else if (e.kind == K::Attrition)
		{
			r.title = U"戦列に大きな損耗";
			r.quote = U"兵を失っている。このまま押すか、守りを固めて立て直すか。";
		}
		// Enemy defeats are reported by our leader, rather than spoken as an enemy boast.
		if (e.kind == K::Defeat && !r.friendly)
		{
			const int leader = game.Leader(game.player);
			if (leader >= 0)
				r.officer = game.generals[leader];
		}
		if (major)
		{
			m_reports.push_front(r);
			m_age = 0;
			m_signaled = false;
		}
		else
			m_reports.push_back(r);
		while (m_reports.size() > 6)
			m_reports.pop_back();
		m_lastSpoke[e.general] = game.day;
	}
}
Wave CampaignSpectacle::Ambience(bool signal)
{
	constexpr int rate = 22050;
	Wave w(static_cast<size_t>(rate * (signal ? 1 : 8)), Arg::sampleRate = rate);
	uint32 seed = 197;
	for (size_t i = 0; i < w.size(); ++i)
	{
		const double t = static_cast<double>(i) / rate, beat = std::fmod(t, .5);
		seed = seed * 1664525u + 1013904223u;
		const double noise = ((seed >> 8) & 65535) / 32768. - 1.;
		const double drum =
		    .32 * std::sin(Math::TwoPi * (65 * beat - 40 * beat * beat)) * std::exp(-16 * beat);
		const double metal =
		    .13 * noise * std::exp(-55 * beat) + .08 * std::sin(Math::TwoPi * 760 * t) * std::exp(-20 * beat);
		const double call =
		    signal ? (.26 * std::sin(Math::TwoPi * 146.83 * t) + .16 * std::sin(Math::TwoPi * 220 * t)) *
		                 std::exp(-3 * t)
		           : .07 * noise;
		const double edge = Min(1., Min(t * 12, (w.size() - i) / static_cast<double>(rate) * 12));
		w[i] = static_cast<float>(Clamp((drum + metal + call) * edge, -.8, .8));
	}
	return w;
}
