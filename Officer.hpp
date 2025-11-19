#pragma once
#include <Siv3D.hpp>

struct Officer
{
	int id = -1;
	String name;

	int factionId = 0;

	int leadership = 50;
	int war = 50;
	int intelligence = 50;
	int politics = 50;
	int charm = 50;

	int level = 1;

	Officer() = default;

	Officer(int _id, const String& _name, int _factionId)
		: id(_id), name(_name), factionId(_factionId)
	{
	}

	Officer(int _id, const String& _name, int _factionId,
			int _lead, int _war, int _int, int _pol, int _cha,
			int _level = 1)
		: id(_id), name(_name), factionId(_factionId),
		leadership(_lead), war(_war), intelligence(_int),
		politics(_pol), charm(_cha), level(_level)
	{
	}
};
