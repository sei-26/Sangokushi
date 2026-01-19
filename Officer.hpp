#pragma once
#include <Siv3D.hpp>

struct Officer
{
	String name;
	int leadership; // 統率
	int war;        // 武力
	int intel;      // 知力
	int politics;   // 政治

	// ★ エラー修正：引数付きコンストラクタを追加
	Officer(String n, int l, int w, int i, int p = 50)
		: name(n)
		, leadership(l)
		, war(w)
		, intel(i)
		, politics(p)
	{
	}

	// デフォルトコンストラクタ
	Officer() : Officer(U"無名", 50, 50, 50, 50) {}
};
