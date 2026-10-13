#pragma once
#include <Siv3D.hpp>
#include "BattlefieldMoments.hpp"
#include <deque>
#include <map>
class CampaignSpectacle
{
	struct Report
	{
		battlefieldmoments::Event event;
		frontline::General officer;
		String title, quote;
		bool friendly = true;
	};
	std::deque<Report> m_reports;
	std::map<int, int> m_lastSpoke;
	double m_time = 0, m_age = 0, m_soundAge = 0;
	bool m_signaled = false;
	bool m_enabled = true, m_loaded = false, m_playing = false;
	Audio m_ambience, m_signal;
	static Wave Ambience(bool signal);

public:
	void Reset();
	RectF ReportRect() const;
	RectF CloseRect() const;
	int ReportTile() const
	{
		return m_enabled && !m_reports.empty() ? m_reports.front().event.tile : -1;
	}
	void Dismiss()
	{
		if (!m_reports.empty())
			m_reports.pop_front();
		m_age = 0;
		m_signaled = false;
	}
	void Toggle()
	{
		m_enabled = !m_enabled;
		if (!m_enabled)
			Reset();
	}
	bool Enabled() const
	{
		return m_enabled;
	}
	void Update(double dt, bool active, bool sound, const frontline::Campaign& game);
	void Capture(const battlefieldmoments::Snapshot& before, const frontline::Campaign& game);
	void DrawMap(const frontline::Campaign& game, const std::function<Vec2(int)>& center, double cell) const;
	void DrawReports(const Texture& faces) const;
};
