#pragma once
#include <Siv3D.hpp>
namespace informationui
{
	inline int Rows()
	{
		return Max(1, (Scene::Height() - 360) / 38);
	}
	inline int DetailRows(int tab = 1)
	{
		return Max(1, (Scene::Height() - (tab == 0 ? 494 : 454) - (tab == 0 ? 160 : 122)) / 36);
	}
	inline int DetailX()
	{
		return Max(470, Scene::Width() * 42 / 100);
	}
	inline int DetailWidth()
	{
		return Scene::Width() - DetailX() - 68;
	}
	inline Rect Close()
	{
		return Rect(Scene::Width() - 158, 118, 94, 30);
	}
	inline Rect Tab(int i)
	{
		return Rect(64 + i * 132, 162, 124, 32);
	}
	inline Rect Filter()
	{
		return Rect(64, 208, 190, 30);
	}
	inline Rect Sort()
	{
		return Rect(264, 208, 220, 30);
	}
	inline Rect Row(int i)
	{
		return Rect(64, 274 + i * 38, DetailX() - 88, 34);
	}
	inline Rect Prev()
	{
		return Rect(64, Scene::Height() - 86, 110, 30);
	}
	inline Rect Next()
	{
		return Rect(184, Scene::Height() - 86, 110, 30);
	}
	inline Rect DetailRow(int i, int tab = 1)
	{
		return Rect(DetailX() + 12, (tab == 0 ? 494 : 454) + i * 36, DetailWidth() - 24, 32);
	}
	inline Rect DetailPrev()
	{
		return Rect(DetailX() + 12, Scene::Height() - 86, 100, 30);
	}
	inline Rect DetailNext()
	{
		return Rect(DetailX() + 124, Scene::Height() - 86, 100, 30);
	}
	inline Rect Map()
	{
		return Rect(DetailX() + DetailWidth() - 168, Scene::Height() - 86, 156, 30);
	}
	inline Rect Profile()
	{
		return Rect(DetailX() + 12, 398, 160, 30);
	}
	inline Rect Relations()
	{
		return Rect(DetailX() + 184, 398, 160, 30);
	}
} // namespace informationui
