#pragma once
#include <Siv3D.hpp>
#include "Officer.hpp"

struct OfficerDatabase
{
	static Array<Officer> LoadAll()
	{
		return {
			Officer(1, U"劉備", 1, 72, 84, 75, 70, 90),
			Officer(2, U"関羽", 1, 88, 98, 72, 65, 70),
			Officer(3, U"張飛", 1, 82, 97, 54, 30, 55),

			Officer(10, U"曹操", 2, 85, 76, 94, 90, 85),
			Officer(11, U"夏侯惇", 2, 78, 86, 55, 50, 60),
			Officer(12, U"夏侯淵", 2, 81, 90, 59, 47, 55),

			Officer(20, U"孫堅",   3, 82, 86, 72, 60, 75),

			Officer(30, U"董卓",   4, 70, 85, 60, 45, 40),

			Officer(40, U"公孫瓚", 5, 75, 80, 58, 50, 55),

			Officer(50, U"袁紹", 6, 78, 72, 65, 70, 68),
		};
	}
};
