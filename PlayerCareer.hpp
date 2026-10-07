#pragma once
#include <Siv3D.hpp>

// Independent of city/officer storage so saves and scene changes keep identity.
struct PlayerCareer
{
	bool officerMode = false;
	String officerName;
	String cityName;
	int merit = 0;
	bool governor = false;
	int completedMonth = -1;
	String report = U"今月の任務を選んでください。";

	String RoleName() const { return officerMode ? (governor ? U"太守" : U"一般武将") : U"君主"; }
	bool CanManageCity(const String& name) const { return !officerMode || (governor && cityName == name); }

	JSON toJSON() const
	{
		JSON j;
		j[U"officerMode"] = officerMode;
		j[U"officerName"] = officerName;
		j[U"cityName"] = cityName;
		j[U"merit"] = merit;
		j[U"governor"] = governor;
		j[U"completedMonth"] = completedMonth;
		j[U"report"] = report;
		return j;
	}
	static PlayerCareer fromJSON(const JSON& j)
	{
		PlayerCareer p;
		p.officerMode = j[U"officerMode"].get<bool>();
		p.officerName = j[U"officerName"].getString();
		p.cityName = j[U"cityName"].getString();
		p.merit = Max(0, j[U"merit"].get<int>());
		p.governor = j[U"governor"].get<bool>();
		p.completedMonth = j[U"completedMonth"].get<int>();
		p.report = j[U"report"].getString();
		return p;
	}
};
