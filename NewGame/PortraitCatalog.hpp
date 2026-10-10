#pragma once
#include <array>
#include <string>
namespace campaignvisual
{
	inline constexpr std::array<const char32_t*, 72> PortraitNames{
	    U"劉備", U"趙雲", U"張飛", U"黄忠", U"関羽",   U"諸葛亮", U"簡雍", U"糜竺",   U"馬超",
	    U"馬岱", U"魏延", U"龐統", U"姜維", U"法正",   U"徐庶",   U"張苞", U"関興",   U"劉封",
	    U"李厳", U"王平", U"蔣琬", U"費禕", U"董允",   U"鄧芝",   U"曹操", U"曹仁",   U"夏侯惇",
	    U"徐晃", U"張遼", U"荀彧", U"任峻", U"満寵",   U"夏侯淵", U"張郃", U"楽進",   U"于禁",
	    U"許褚", U"典韋", U"曹洪", U"曹真", U"曹休",   U"鄧艾",   U"鍾会", U"司馬懿", U"郭嘉",
	    U"荀攸", U"程昱", U"陳群", U"孫権", U"太史慈", U"周瑜",   U"甘寧", U"呂蒙",   U"陸遜",
	    U"顧雍", U"歩騭", U"黄蓋", U"程普", U"韓当",   U"周泰",   U"蔣欽", U"凌統",   U"徐盛",
	    U"丁奉", U"朱桓", U"朱然", U"孫策", U"孫尚香", U"魯粛",   U"張昭", U"諸葛瑾", U"陸抗",
	};
	// Actual boundaries measured from the generated v1 atlases, in source pixels.
	inline std::array<int, 4> PortraitCrop(int index, int width, int height)
	{
		if (index < 0 || index >= 72 || width < 16 || height < 16)
			return {};
		constexpr int rows[3][7] = {{0, 240, 476, 720, 965, 1216, 1536},
		                            {0, 231, 457, 688, 931, 1199, 1536},
		                            {0, 238, 471, 711, 952, 1203, 1536}};
		const int sheet = index / 24, cell = index % 24, col = cell % 4, row = cell / 4;
		const int left = width * col / 4 + 1, right = width * (col + 1) / 4 - 1;
		const int top = height * rows[sheet][row] / 1536 + 1,
		          bottom = height * rows[sheet][row + 1] / 1536 - 1;
		return {left, top, right - left, bottom - top};
	}
	inline int PortraitIndex(const std::u32string& name)
	{
		for (int i = 0; i < 72; ++i)
			if (name == PortraitNames[i])
				return i;
		return -1;
	}
} // namespace campaignvisual
